#ifndef __RINA_ECHO_ASYNC_H__
#define __RINA_ECHO_ASYNC_H__

/*##########################*/
/* CONFIGURE DEFINES  */

/*##########################*/
#define SDU_SIZE_MAX 64
#define MAX_CLIENTS 128
#define TIMEOUT_SECS 3
#define REGISTER_TIMEOUT_SECS 10
#if TIMEOUT_SECS < 2
#error "TIMEOUT_SECS must be >= 2"
#endif

#define MAX(a, b) ((a) > (b) ? (a) : (b))

struct fdfsm
{
    char buf[SDU_SIZE_MAX];
    int buflen;
#define SELFD_S_ALLOC 1
#define SELFD_S_WRITE 2
#define SELFD_S_READ 3
#define SELFD_S_NONE 4
#define SELFD_S_REGISTER 5
#define SELFD_S_ACCEPT 6
    int state;
    int fd;
    time_t last_activity;
};

#define PRINTF(FMT, ...)            \
    do                              \
    {                               \
        printf(FMT, ##__VA_ARGS__); \
        fflush(stdout);             \
    } while (0)

struct echo_async
{
    int cfd;
    const char *cli_appl_name;
    const char *srv_appl_name;
    char *dif_name;
    struct rinaFlowSpec_t *flowspec;
    int p; /* parallel clients */
};
BaseType_t rina_echo_client(char *dif_name, struct rinaFlowSpec_t *flowspec);

#endif