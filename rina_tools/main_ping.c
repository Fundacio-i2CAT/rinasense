/*
 * Copyright (C) 2015-2017 Nextworks
 * Author: Vincenzo Maffione <v.maffione@gmail.com>
 *
 * This file is part of rlite.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <sys/time.h>
#include <assert.h>
#include <endian.h>
#include <signal.h>
#include <sys/poll.h>
#include <time.h>

#include <fcntl.h>
#include <math.h>

#include "configRINA.h"
#include "IPCP.h"
#include "RINA_API.h"

/*
 * rinaperf: a tool to measure bandwidth and latency of RINA networks.
 *
 * The rinaperf program uses two separate flows between the client and the
 * server for each test. The first one is a control flow, and it is used to
 * negotiate the test configuration with the server, to terminate the test
 * and to receive results. The second one is the data flow, where the test
 * data is transported.
 *
 * The application protocol on the client side works as follows:
 *   - The client allocates the control flow and sends a 20 bytes
 *     configuration message containing the number of SDUs or transactions,
 *     the SDU size and the test type.
 *   - The clients waits for a 4 bytes ticket message from the server (on
 *     the control flow), containing an integer number that identifies the
 *     test.
 *   - The client allocates the data flow and sends on this flow a 20 bytes
 *     configuration message containing the ticket received from the control
 *     flow, so that the server knows to which control flow this data flow
 *     needs to be associated.
 *   - The client runs the client-side test function (e.g., perf, ping or rr)
 *     sending and/or receiving data to/from the data flow only.
 *   - When the client-side test function ends, the client sends a 20 bytes
 *     stop message on the control flow to ask the server-side test function
 *     to stop (this may be useful to avoid that server times out, so that the
 *     test session can end immediately).
 *   - For tests different from "ping", the client waits for a 32 bytes result
 *     message, containing various statistics as measured by the server-side
 *     test function (e.g. SDU count, pps, bps, latency, ...).
 *   - The client prints the results and closes both control and data flow.
 *
 * The application protocol on the server side works as follows:
 *   - The server accepts the next flow and allocates a worker thread to handle
 *     the request.
 *   - The worker waits to receive a 20 bytes configuration message from the
 *     flow. Looking at the message opcode, the worker decides if this is a
 *     control flow or a data flow.
 *   - In case of control flow, the worker allocates a ticket for the client
 *     and sends it with a 4 bytes message. The worker then waits (on a
 *     semaphore) to be notified by a future worker that is expected to
 *     receive the same ticket on a data flow.
 *   - In case the opcode indicates a data flow, the worker looks up in its
 *     table the ticket specified in the message. If the ticket is valid, the
 *     waiting worker (see above) is notified and informed about the data flow
 *     file descriptor. The current worker can now terminate, as the rest of
 *     the test will be carried out by the notified worker.
 *   - Once woken up, the first worker deallocates the ticket and runs the
 *     server-side test function, using the test configuration contained in the
 *     20 bytes message previously read from the control flow.
 *     The server-side function uses the data flow to send/receive PDUs.
 *     However, it also monitors the control flow to check if a 20 bytes stop
 *     message comes; if one is received, the test function can return early.
 *   - When the server-side test function returns, the worker sends a 32 bytes
 *     message on the control flow, to inform the client about the test
 *     results. Finally, both control and data flows are closed.
 */

#define NUMBERS_OF_PINGS 5

#define SDU_SIZE_MAX 65535
#define RP_MAX_WORKERS 1023

#define RP_OPCODE_PING 0
#define RP_OPCODE_RR 1
#define RP_OPCODE_PERF 2
#define RP_OPCODE_DATAFLOW 3
#define RP_OPCODE_STOP 4 /* must be the last */

#define CLI_FA_TIMEOUT_MSECS 5000
#define CLI_RESULT_TIMEOUT_MSECS 5000
#define RP_DATA_WAIT_MSECS 10000

struct rinaperf;
struct worker;

struct rp_config_msg
{
    uint64_t cnt;    /* packet/transaction count for the test
                      * (0 means infinite) */
    uint32_t opcode; /* opcode: ping, perf, rr ... */
    uint32_t ticket; /* valid with RP_OPCODE_DATAFLOW */
    uint32_t size;   /* packet size in bytes */
} __attribute__((packed));

struct rp_ticket_msg
{
    uint32_t ticket; /* ticket allocated by the server for the
                      * client to identify the data flow */
} __attribute__((packed));

struct rp_result_msg
{
    uint64_t cnt;     /* number of packets or completed transactions
                       * as seen by the sender or the receiver */
    uint64_t pps;     /* average packet rate measured by the sender or receiver */
    uint64_t bps;     /* average bandwidth measured by the sender or receiver */
    uint64_t latency; /* in nanoseconds */
} __attribute__((packed));

typedef int (*perf_fn_t)(struct worker *);
typedef void (*report_fn_t)(struct worker *w, struct rp_result_msg *snd,
                            struct rp_result_msg *rcv);

struct worker
{
    pthread_t th;
    struct rinaperf *rp; /* backpointer */
    struct worker *next; /* next worker */
    struct rp_config_msg test_config;
    struct rp_result_msg result;
    uint32_t ticket;       /* ticket to be sent to the client */
    sem_t data_flow_ready; /* to wait for dfd */
    unsigned int interval;
    unsigned int burst;
    int ping; /* is this a ping test? */
    struct rp_test_desc *desc;
    int cfd;                       /* control file descriptor */
    int dfd;                       /* data file descriptor */
    int retcode;                   /* for the client to report success/failure */
    unsigned int real_duration_ms; /* measured by the client */

    /* A window of RTT samples to compute ping statistics. */
#define RTT_WINSIZE 4096
    unsigned int rtt_win_idx;
    uint32_t rtt_win[RTT_WINSIZE];
};

struct rinaperf
{
    struct rinaFlowSpec_t *flowspec;
    const char *cli_appl_name;
    const char *srv_appl_name;
    const char *dif_name;
    int cfd;                /* control file descriptor */
    int parallel;           /* num of parallel clients */
    int duration;           /* duration of client test (secs) */
    int use_mss_size;       /* use flow MSS as packet size */
    int verbose;            /* be verbose */
    int timestamp;          /* print timestamp during ping test */
    int stop_pipe[2];       /* to stop client threads */
    int cli_stop;           /* another way to stop client threads */
    int cli_flow_allocated; /* client flows allocated ? */
    int background;         /* server runs as a daemon process */
    int cdf;                /* report CDF percentiles */

    /* Synchronization between client threads and main thread. */
    sem_t cli_barrier;

    /* Ticket table. */
    pthread_mutex_t ticket_lock;
    struct worker *ticket_table[RP_MAX_WORKERS];

    /* List of workers. */
    struct worker *workers_head;
    struct worker *workers_tail;
    sem_t workers_free;
};

#define PRINTF(FMT, ...)            \
    do                              \
    {                               \
        printf(FMT, ##__VA_ARGS__); \
        fflush(stdout);             \
    } while (0)

static struct rinaperf _rp;

/* Used for both ping and rr tests. */
static int
ping_client(struct worker *w)
{
    unsigned int limit = w->test_config.cnt;
    struct timespec t_start, t_end, t1, t2;
    unsigned int interval = w->interval;
    int size = sizeof(uint16_t);
    char buf[SDU_SIZE_MAX];
    volatile uint16_t *seqnum = (uint16_t *)buf;
    uint16_t expected = 0;
    unsigned int timeouts = 0;
    int ping = w->ping;
    unsigned int i = 0;
    unsigned long long ns;
    struct pollfd pfd[2];
    int ret = 0;

    pfd[0].fd = w->dfd;
    pfd[1].fd = w->rp->stop_pipe[0];
    pfd[0].events = pfd[1].events = POLLIN;

    memset(buf, 'x', size);

    clock_gettime(CLOCK_MONOTONIC, &t_start);

    for (i = 0; !limit || i < limit; i++, expected++)
    {
        if (ping)
        {
            clock_gettime(CLOCK_MONOTONIC, &t1);
        }

        *seqnum = (uint16_t)expected;

        ret = write(w->dfd, buf, size);
        if (ret != size)
        {
            if (ret < 0)
            {
                perror("write(buf)");
            }
            else
            {
                PRINTF("Partial write %d/%d\n", ret, size);
            }
            break;
        }
    repoll:
        ret = poll(pfd, 2, RP_DATA_WAIT_MSECS);
        if (ret < 0)
        {
            perror("poll(flow)");
        }

        if (ret == 0)
        {
            PRINTF("Timeout: %d bytes lost\n", size);
            if (++timeouts > 8)
            {
                PRINTF("Stopping after %u consecutive timeouts\n", timeouts);
                break;
            }
        }
        else if (pfd[1].revents & POLLIN)
        {
            break;
        }
        else
        {
            /* Ready to read. */
            timeouts = 0;
            ret = read(w->dfd, buf, sizeof(buf));
            if (ret <= 0)
            {
                if (ret)
                {
                    perror("read(buf");
                }
                break;
            }

            if (ping)
            {
                if (*seqnum == expected)
                {
                    clock_gettime(CLOCK_MONOTONIC, &t2);
                    ns = 1000000000 * (t2.tv_sec - t1.tv_sec) +
                         (t2.tv_nsec - t1.tv_nsec);
                    if (w->rp->timestamp)
                    {
                        struct timeval recv_time;
                        gettimeofday(&recv_time, NULL);
                        PRINTF("[%lu.%06lu] ", (unsigned long)recv_time.tv_sec,
                               (unsigned long)recv_time.tv_usec);
                    }
                    w->rtt_win[w->rtt_win_idx] = ns;
                    w->rtt_win_idx = (w->rtt_win_idx + 1) % RTT_WINSIZE;
                    PRINTF("%d bytes from server: rtt = %.3f ms\n", ret,
                           ((float)ns) / 1000000.0);
                }
                else
                {
                    PRINTF("Packet lost or out of order: got %u, "
                           "expected %u\n",
                           *seqnum, expected);
                    if (*seqnum < expected)
                    {
                        goto repoll;
                    }
                }
            }
        }

        if (interval)
        {
            stoppable_usleep(w->rp, interval);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t_end);
    ns = 1000000000 * (t_end.tv_sec - t_start.tv_sec) +
         (t_end.tv_nsec - t_start.tv_nsec);
    w->real_duration_ms = ns / 1000000;

    w->result.cnt = i;
    w->result.pps = 1000000000ULL;
    w->result.pps *= i;
    w->result.pps /= ns;
    w->result.bps = w->result.pps * 8 * size;
    w->result.latency = i ? ((ns / i) - interval * 1000) : 0;

    w->test_config.cnt = i; /* write back packet count */

    return 0;
}

struct rp_test_desc
{
    const char *name;
    const char *description;
    unsigned int opcode;
    perf_fn_t client_fn;
    perf_fn_t server_fn;
    report_fn_t report_fn;
};

static struct rp_test_desc descs[] = {
    {
        .name = "ping",
        .opcode = RP_OPCODE_PING,
        .client_fn = ping_client,
        .server_fn = ping_server,
        .report_fn = ping_report,
    },
    {
        .name = "rr",
        .description = "request-response test",
        .opcode = RP_OPCODE_RR,
        .client_fn = ping_client,
        .server_fn = ping_server,
        .report_fn = rr_report,
    },
    {
        .name = "perf",
        .description = "unidirectional throughput test",
        .opcode = RP_OPCODE_PERF,
        .client_fn = perf_client,
        .server_fn = perf_server,
        .report_fn = perf_report,
    },
};

static void *
client_worker_function(void *opaque)
{
    struct worker *w = opaque;
    struct rinaperf *rp = w->rp;
    struct rp_config_msg cfg = w->test_config;
    struct rp_ticket_msg tmsg;
    struct rp_result_msg rmsg;
    struct pollfd pfd;
    int ret;

    w->retcode = -1; /* set to 0 only if everything goes well */

    /* Allocate the control flow to be used for test configuration and
     * to receive test result.
     * We should always use reliable flows. */
    pfd.fd = rina_flow_alloc(rp->dif_name, rp->cli_appl_name, rp->srv_appl_name,
                             &rp->flowspec, RINA_F_NOWAIT);
    if (pfd.fd < 0)
    {
        perror("rina_flow_alloc(cfd)");
        goto out;
    }
    pfd.events = POLLIN;
    ret = poll(&pfd, 1, CLI_FA_TIMEOUT_MSECS);
    if (ret <= 0)
    {
        if (ret < 0)
        {
            perror("poll(cfd)");
        }
        else
        {
            PRINTF("Flow allocation timed out for control flow\n");
        }
        close(pfd.fd);
        goto out;
    }
    w->cfd = rina_flow_alloc_wait(pfd.fd);
    if (w->cfd < 0)
    {
        perror("rina_flow_alloc_wait(cfd)");
        goto out;
    }

    /* Override packet size with the MSS size if necessary. */
    if (rp->use_mss_size)
    {
        unsigned int mss = rina_flow_mss_get(w->cfd);

        if (mss)
        {
            cfg.size = mss;
            w->test_config.size = mss;
        }
    }

    /* Send test configuration to the server. */
    cfg.opcode = htole32(cfg.opcode);
    cfg.cnt = htole64(cfg.cnt);
    cfg.size = htole32(cfg.size);

    ret = write(w->cfd, &cfg, sizeof(cfg));
    if (ret != sizeof(cfg))
    {
        if (ret < 0)
        {
            perror("write(cfg)");
        }
        else
        {
            PRINTF("Partial write %d/%lu\n", ret,
                   (unsigned long int)sizeof(cfg));
        }
        goto out;
    }

    /* Wait for the ticket message from the server and read it. */
    pfd.fd = w->cfd;
    pfd.events = POLLIN;
    ret = poll(&pfd, 1, RP_DATA_WAIT_MSECS);
    if (ret <= 0)
    {
        if (ret < 0)
        {
            perror("poll(ticket)");
        }
        else
        {
            PRINTF("Timeout while waiting for ticket message\n");
        }
        goto out;
    }

    ret = read(w->cfd, &tmsg, sizeof(tmsg));
    if (ret != sizeof(tmsg))
    {
        if (ret < 0)
        {
            perror("read(ticket)");
        }
        else
        {
            PRINTF("Error reading ticket message: wrong length %d "
                   "(should be %lu)\n",
                   ret, (unsigned long int)sizeof(tmsg));
        }
        goto out;
    }

    /* Allocate a data flow for the test. */
    pfd.fd = rina_flow_alloc(rp->dif_name, rp->cli_appl_name, rp->srv_appl_name,
                             &rp->flowspec, RINA_F_NOWAIT);
    if (pfd.fd < 0)
    {
        perror("rina_flow_alloc(cfd)");
        goto out;
    }
    pfd.events = POLLIN;
    ret = poll(&pfd, 1, CLI_FA_TIMEOUT_MSECS);
    if (ret <= 0)
    {
        if (ret < 0)
        {
            perror("poll(dfd)");
        }
        else
        {
            PRINTF("Flow allocation timed out for data flow\n");
        }
        close(pfd.fd);
        goto out;
    }
    w->dfd = rina_flow_alloc_wait(pfd.fd);
    rp->cli_flow_allocated = 1;
    if (w->dfd < 0)
    {
        perror("rina_flow_alloc_wait(dfd)");
        goto out;
    }

    /* Send the ticket to the server to identify the data flow. */
    memset(&cfg, 0, sizeof(cfg));
    cfg.opcode = htole32(RP_OPCODE_DATAFLOW);
    cfg.ticket = tmsg.ticket;
    ret = write(w->dfd, &cfg, sizeof(cfg));
    if (ret != sizeof(cfg))
    {
        if (ret < 0)
        {
            perror("write(identify)");
        }
        else
        {
            PRINTF("Partial write %d/%lu\n", ret,
                   (unsigned long int)sizeof(cfg));
        }
        goto out;
    }

    if (w->test_config.size > SDU_SIZE_MAX)
    {
        PRINTF("Warning: size truncated to %u\n", SDU_SIZE_MAX);
        w->test_config.size = SDU_SIZE_MAX;
    }

    if (!w->ping)
    {
        char countbuf[64];
        char durbuf[64];

        if (w->test_config.cnt)
        {
            snprintf(countbuf, sizeof(countbuf), "%llu",
                     (long long unsigned)w->test_config.cnt);
        }
        else
        {
            strncpy(countbuf, "inf", sizeof(countbuf));
        }

        if (rp->duration)
        {
            snprintf(durbuf, sizeof(durbuf), "%d secs", rp->duration);
        }
        else
        {
            strncpy(durbuf, "inf", sizeof(durbuf));
        }

        PRINTF("Starting %s; message size: %u, number of messages: %s,"
               " duration: %s\n",
               w->desc->description, w->test_config.size, countbuf, durbuf);
    }

    /* Run the test. */
    w->desc->client_fn(w);

    if (!w->ping)
    {
        /* Wait some milliseconds before asking the server to stop and get
         * results. This heuristic is useful to let the last retransmissions
         * happen before we get the server-side measurements. */
        usleep(100000);
    }

    /* Send the stop opcode on the control file descriptor. With reliable
     * flows we also send the expected packet count, so that the receiver
     * can try to receive more from the data file descriptor. */
    memset(&cfg, 0, sizeof(cfg));
    cfg.opcode = htole32(RP_OPCODE_STOP);
    if (is_reliable_spec(&rp->flowspec))
    {
        cfg.cnt = htole64(w->test_config.cnt);
    }
    ret = write(w->cfd, &cfg, sizeof(cfg));
    if (ret != sizeof(cfg))
    {
        if (ret < 0)
        {
            perror("write(stop)");
        }
        else
        {
            PRINTF("Partial write %d/%lu\n", ret,
                   (unsigned long int)sizeof(cfg));
        }
        goto out;
    }

    /* Wait for the result message from the server and read it. */
    pfd.fd = w->cfd;
    pfd.events = POLLIN;
    ret = poll(&pfd, 1, CLI_RESULT_TIMEOUT_MSECS);
    if (ret <= 0)
    {
        if (ret < 0)
        {
            perror("poll(result)");
        }
        else
        {
            PRINTF("Timeout while waiting for result message\n");
        }
        goto out;
    }

    ret = read(w->cfd, &rmsg, sizeof(rmsg));
    if (ret != sizeof(rmsg))
    {
        if (ret < 0)
        {
            perror("read(result)");
        }
        else
        {
            PRINTF("Error reading result message: wrong length %d "
                   "(should be %lu)\n",
                   ret, (unsigned long int)sizeof(rmsg));
        }
        goto out;
    }

    rmsg.cnt = le64toh(rmsg.cnt);
    rmsg.pps = le64toh(rmsg.pps);
    rmsg.bps = le64toh(rmsg.bps);
    rmsg.latency = le64toh(rmsg.latency);

    w->desc->report_fn(w, &w->result, &rmsg);

    w->retcode = 0;
out:
    worker_fini(w);

    sem_post(&rp->cli_barrier);

    return NULL;
}

static int
config_msg_read(int cfd, struct rp_config_msg *cfg)
{
    int ret = read(cfd, cfg, sizeof(*cfg));

    if (ret != sizeof(*cfg))
    {
        if (ret < 0)
        {
            perror("read(cfg)");
        }
        else
        {
            PRINTF("Error reading test configuration: wrong length %d "
                   "(should be %lu)\n",
                   ret, (unsigned long int)sizeof(*cfg));
        }
        return -1;
    }

    cfg->opcode = le32toh(cfg->opcode);
    cfg->ticket = le32toh(cfg->ticket);
    cfg->cnt = le64toh(cfg->cnt);
    cfg->size = le32toh(cfg->size);

    return 0;
}

BaseType_t rina_ping()
{

    unsigned int limit = NUMBERS_OF_PINGS;
    struct timespec t_start, t_end, t1, t2;
    unsigned int interval = 1000000;
    int size = sizeof(uint16_t);
    char buf[SDU_SIZE_MAX];
    volatile uint16_t *seqnum = (uint16_t *)buf;
    uint16_t expected = 0;
    unsigned int timeouts = 0;
    // int ping = w->ping;
    unsigned int i = 0;
    unsigned long long ns;
    struct pollfd pfd[2];
    int ret = 0;

    pfd[0].fd = w->dfd;
    pfd[1].fd = w->rp->stop_pipe[0];
    pfd[0].events = pfd[1].events = POLLIN;

    memset(buf, 'x', size);

    clock_gettime(CLOCK_MONOTONIC, &t_start);

    for (i = 0; !limit || i < limit; i++, expected++)
    {

        clock_gettime(CLOCK_MONOTONIC, &t1);

        *seqnum = (uint16_t)expected;

        ret = write(w->dfd, buf, size);
        if (ret != size)
        {
            if (ret < 0)
            {
                perror("write(buf)");
            }
            else
            {
                PRINTF("Partial write %d/%d\n", ret, size);
            }
            break;
        }
    repoll:
        ret = poll(pfd, 2, RP_DATA_WAIT_MSECS);
        if (ret < 0)
        {
            perror("poll(flow)");
        }

        if (ret == 0)
        {
            PRINTF("Timeout: %d bytes lost\n", size);
            if (++timeouts > 8)
            {
                PRINTF("Stopping after %u consecutive timeouts\n", timeouts);
                break;
            }
        }
        else if (pfd[1].revents & POLLIN)
        {
            break;
        }
        else
        {
            /* Ready to read. */
            timeouts = 0;
            ret = read(w->dfd, buf, sizeof(buf));
            if (ret <= 0)
            {
                if (ret)
                {
                    perror("read(buf");
                }
                break;
            }

            if (ping)
            {
                if (*seqnum == expected)
                {
                    clock_gettime(CLOCK_MONOTONIC, &t2);
                    ns = 1000000000 * (t2.tv_sec - t1.tv_sec) +
                         (t2.tv_nsec - t1.tv_nsec);
                    if (w->rp->timestamp)
                    {
                        struct timeval recv_time;
                        gettimeofday(&recv_time, NULL);
                        PRINTF("[%lu.%06lu] ", (unsigned long)recv_time.tv_sec,
                               (unsigned long)recv_time.tv_usec);
                    }
                    w->rtt_win[w->rtt_win_idx] = ns;
                    w->rtt_win_idx = (w->rtt_win_idx + 1) % RTT_WINSIZE;
                    PRINTF("%d bytes from server: rtt = %.3f ms\n", ret,
                           ((float)ns) / 1000000.0);
                }
                else
                {
                    PRINTF("Packet lost or out of order: got %u, "
                           "expected %u\n",
                           *seqnum, expected);
                    if (*seqnum < expected)
                    {
                        goto repoll;
                    }
                }
            }
        }

        if (interval)
        {
            stoppable_usleep(w->rp, interval);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t_end);
    ns = 1000000000 * (t_end.tv_sec - t_start.tv_sec) +
         (t_end.tv_nsec - t_start.tv_nsec);
    w->real_duration_ms = ns / 1000000;

    w->result.cnt = i;
    w->result.pps = 1000000000ULL;
    w->result.pps *= i;
    w->result.pps /= ns;
    w->result.bps = w->result.pps * 8 * size;
    w->result.latency = i ? ((ns / i) - interval * 1000) : 0;

    w->test_config.cnt = i; /* write back packet count */

    return 0;
}

void app_main(void)
{

    struct rinaperf *rp = &_rp;
    const char *type = "ping";
    int interval_specified = 0;
    int duration_specified = 0;
    int listen = 0;
    int cnt = 0;
    int size = sizeof(uint16_t);
    int interval = 0;
    int burst = 1;
    struct worker wt; /* template */
    int ret;
    int opt;
    int i;

    memset(&wt, 0, sizeof(wt));
    wt.rp = rp;
    wt.cfd = -1;

    memset(rp, 0, sizeof(*rp));
    rp->cli_appl_name = "client";
    rp->srv_appl_name = "server";
    rp->parallel = 1;
    rp->duration = 0;
    rp->use_mss_size = 1;
    rp->verbose = 0;
    rp->timestamp = 0;
    rp->cfd = -1;
    rp->stop_pipe[0] = rp->stop_pipe[1] = -1;
    rp->cli_stop = rp->cli_flow_allocated = 0;
    rp->flowspec = pvPortMalloc(sizeof(*rinaFlowSpec_t));
    sem_init(&rp->workers_free, 0, RP_MAX_WORKERS);
    sem_init(&rp->cli_barrier, 0, 0);
    pthread_mutex_init(&rp->ticket_lock, NULL);
    rp->background = 0;
    rp->cdf = 0; /* Don't report CDF percentiles. */

    /* Start with a default flow configuration (unreliable flow). */
    // rina_flow_spec_unreliable(&rp->flowspec);

    /*
     * Fixups:
     *   - Use 1 second interval for ping tests, if the user did not
     *     specify the interval explicitly.
     *   - Set rp->ping variable to distinguish between ping and rr tests,
     *     which share the same functions.
     *   - When not in ping mode, ff user did not specify the number of
     *     packets (or transactions) nor the test duration, use a 10 seconds
     *     test duration.
     *   - When in perf mode, use the flow MSS as a packet size, unless the
     *     user has specified the size explicitely.
     */
    if (strcmp(type, "ping") == 0)
    {
        if (!interval_specified)
        {
            interval = 1000000;
        }
        if (!duration_specified)
        {
            rp->duration = 0;
        }
        wt.ping = 1;
    }
    else
    {
        wt.ping = 0;
        if (!duration_specified && !cnt)
        {
            rp->duration = 10; /* seconds */
        }
    }

    if (strcmp(type, "perf") != 0)
    {
        rp->use_mss_size = 0; /* default MTU size only for perf */
    }

    /* Set defaults. */
    wt.interval = interval;
    wt.burst = burst;

    if (!listen)
    {
        ret = pipe(rp->stop_pipe);
        if (ret < 0)
        {
            perror("pipe()");
            return -1;
        }

        /* Function selection. */
        for (i = 0; i < sizeof(descs) / sizeof(descs[0]); i++)
        {
            if (descs[i].name && strcmp(descs[i].name, type) == 0)
            {
                wt.desc = descs + i;
                break;
            }
        }

        if (wt.desc == NULL)
        {
            PRINTF("    Unknown test type '%s'\n", type);
            usage();
            return -1;
        }
        wt.test_config.opcode = descs[i].opcode;
        wt.test_config.cnt = cnt;
        wt.test_config.size = size;
    }

    /* Set some signal handler */
    sa.sa_handler = listen ? sigint_handler_server : sigint_handler_client;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    ret = sigaction(SIGINT, &sa, NULL);
    if (ret)
    {
        perror("sigaction(SIGINT)");
        return ret;
    }
    ret = sigaction(SIGTERM, &sa, NULL);
    if (ret)
    {
        perror("sigaction(SIGTERM)");
        return ret;
    }

    /* Open control file descriptor. */
    rp->cfd = rina_open();
    if (rp->cfd < 0)
    {
        perror("rina_open()");
        return rp->cfd;
    }

    if (!listen)
    {
        /* Client mode. */
        struct worker *workers = calloc(rp->parallel, sizeof(*workers));
        int retcode = 0;

        if (workers == NULL)
        {
            PRINTF("Failed to allocate client workers\n");
            return -1;
        }

        for (i = 0; i < rp->parallel; i++)
        {
            memcpy(workers + i, &wt, sizeof(wt));
            worker_init(workers + i, rp);
            ret = pthread_create(&workers[i].th, NULL, client_worker_function,
                                 workers + i);
            if (ret)
            {
                PRINTF("pthread_create(#%d) failed: %s\n", i, strerror(ret));
                break;
            }
        }

        if (rp->duration > 0)
        {
            /* Wait for the clients to finish, but no more than rp->duration
             * seconds. */
            struct timespec to;

            clock_gettime(CLOCK_REALTIME, &to);
            to.tv_sec += rp->duration;

            for (i = 0; i < rp->parallel; i++)
            {
                ret = sem_timedwait(&rp->cli_barrier, &to);
                if (ret)
                {
                    if (errno == ETIMEDOUT)
                    {
                        if (rp->verbose)
                        {
                            PRINTF("Stopping clients, %d seconds elapsed\n",
                                   rp->duration);
                        }
                    }
                    else
                    {
                        perror("pthread_cond_timedwait() failed");
                    }
                    break;
                }
            }

            if (i < rp->parallel)
            {
                /* Timeout (or error) occurred, tell the clients to stop. */
                stop_clients(rp);
            }
            else
            {
                /* Client finished before rp->duration seconds. */
            }
        }

        for (i = 0; i < rp->parallel; i++)
        {
            ret = pthread_join(workers[i].th, NULL);
            if (ret)
            {
                PRINTF("pthread_join(#%d) failed: %s\n", i, strerror(ret));
            }
            retcode |= workers[i].retcode;
        }
        free(workers);
        return retcode;
    }

    /* Server mode. */
    return server(rp);
}
