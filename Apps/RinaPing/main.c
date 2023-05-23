
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <rom/ets_sys.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "configRINA.h"
#include "RINA_API.h"
#include "IPCP_api.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define TAG_APP "[PING-APP]"
#define PING_SIZE 1024U
#define NUMBER_OF_PINGS (200)

#define RP_OPCODE_PING 0
#define RP_OPCODE_PERF 2
#define RP_OPCODE_DATAFLOW 3
#define RP_OPCODE_STOP 4 /* must be the last */

#define DIF "mobile.DIF"
#define SERVER "sensor1"
#define CLIENT "ping"

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

struct rp_ticket_msg const tmsg;
struct rp_config_msg cfg;

struct rp_result_msg result;

portId_t xControlPortId;
struct rinaFlowSpec_t *xFlowSpec;
uint8_t Flags = 1;
portId_t xDataPortId;

size_t ret = 0;

unsigned int real_duration_ms; /* measured by the client */

unsigned long IRAM_ATTR micros()
{
    return (unsigned long)(esp_timer_get_time());
}

void IRAM_ATTR delayMicros(uint32_t us)
{
    uint32_t m = micros();
    if (us)
    {
        uint32_t e = (m + us);
        if (m > e)
        { // overflow
            while (micros() > e)
            {
                // NOP();
            }
        }
        while (micros() < e)
        {
            // NOP();
        }
    }
}

static int ping_client(portId_t xPingPortId)
{
    unsigned int limit = NUMBER_OF_PINGS;
    int64_t time_start, time_end;
    int64_t time1, time2;
    size_t size_sdu = (size_t)PING_SIZE + 1;
    void *bufferTx;
    bufferTx = pvPortMalloc((size_t)size_sdu);

    volatile uint16_t *seqnum = (uint16_t *)bufferTx;
    uint16_t expected = 0;
    unsigned int i = 0;
    unsigned long us;
    int uxTxBytes = 0, uxRxBytes;
    int RTT[NUMBER_OF_PINGS];

    float result;
    float average = 0;
    int min, max, sum, received = 0;

    memset(bufferTx, 'x', (size_t)PING_SIZE);
    memset(bufferTx + PING_SIZE, '\0', 1);

    time_start = esp_timer_get_time();

    for (i = 0; i < limit; i++, expected++)
    {
        time1 = esp_timer_get_time();

        *seqnum = (uint16_t)expected;

        ret = RINA_flow_write(xPingPortId, (void *)bufferTx, strlen(bufferTx));
        if (ret != strlen(bufferTx))
        {
            if (ret < 0)
            {
                ESP_LOGE(TAG_APP, "write(buf)");
            }
            else
            {
                // ESP_LOGE(TAG_APP, "Partial write %d/%d\n", ret, strlen(bufferTx));
            }
            break;
        }
        RTT[i] = 0;
        uxRxBytes = 0;

        while (uxRxBytes < uxTxBytes)
        {
            vTaskDelay(10 / portTICK_RATE_MS);
            uxRxBytes = RINA_flow_read(xPingPortId, (void *)bufferTx, size_sdu);

            if (uxRxBytes == 0)
            {
                ESP_LOGE(TAG_APP, "It was an error receiving the buffer");
                break;
            }
            if (uxRxBytes > 0)
            {
                if (*seqnum == expected)
                {
                    time2 = esp_timer_get_time();
                    us = time2 - time1;

                    // ESP_LOGD(TAG_APP, "%d bytes from server: rtt = %.3f ms\n", uxRxBytes, ns);
                    // ESP_LOGI(TAG_APP, "%d bytes from server: rtt = %d ms", uxRxBytes, time_delta / 1000);
                    ESP_LOGI(TAG_APP, "%d bytes from server: rtt = %.3f ms", uxRxBytes, (float)us / 1000);
                    RTT[i] = us;
                    received++;
                }
            }
            else
            {
                ESP_LOGI(TAG_APP, "Request time out");
            }
        }
    }
    time_end = esp_timer_get_time();
    us = time_end - time_start;

    result = 1000000 * received;
    result = result / us;
    result = result * 8 * uxRxBytes;

    min = 100000000000;
    max = 0;
    for (i = 1; i < NUMBER_OF_PINGS; i++)
    {
        if (RTT[i] > 0)
        {
            if (min > RTT[i])
                min = RTT[i];
            if (max < RTT[i])
                max = RTT[i];
        }
    }
    sum = 0;
    for (i = 0; i < NUMBER_OF_PINGS; i++)
    {

        if (RTT[i] > 0)
        {
            sum = sum + RTT[i];
        }
    }
    if (received > 0)
        average = (sum / received);

    ESP_LOGI(TAG_APP, "Ping Statistics to for %s:", SERVER);
    ESP_LOGI(TAG_APP, "     Packets: send = %d, received = %d , timeout = %d", NUMBER_OF_PINGS, received, NUMBER_OF_PINGS - received);
    ESP_LOGI(TAG_APP, "Approximate round trip times in milliseconds: ");
    ESP_LOGI(TAG_APP, "     Minimum = %.3f ms , Maximum = %.3f ms, Average = %.3f ms ", (float)min / 1000, (float)max / 1000, average / 1000);
    // ESP_LOGI(TAG_APP, "     Througput = %f bps\n", result);
    return 0;
}

void app_main(void)
{

    nvs_flash_init();

    /*Init RINA Task*/
    RINA_IPCPInit();

    vTaskDelay(1000);

    uint32_t size = PING_SIZE;
    uint32_t opcode = RP_OPCODE_PING;
    struct rp_result_msg rmsg;

    /*Requesting a control flow*/
    xControlPortId = RINA_flow_alloc(DIF, CLIENT, SERVER, xFlowSpec, Flags);

    ESP_LOGI(TAG_APP, "Control Port: %d", xControlPortId);

    if (xControlPortId < 0)
    {
        ESP_LOGI(TAG_APP, "Error allocating the flow ");
    }

    /* Send a Message  with the parameters config to the server */
    cfg.cnt = NUMBER_OF_PINGS;
    cfg.size = size;
    cfg.opcode = opcode;

    ret = RINA_flow_write(xControlPortId, (void *)&cfg, sizeof(cfg));
    if (ret <= 0)
    {
        ESP_LOGE(TAG_APP, "Error to send Data");
        // break;
    }
    else
    {
        vTaskDelay(10);

        /*Read the ticket message sended by the server*/
        ret = RINA_flow_read(xControlPortId, (void *)&tmsg, sizeof(tmsg));

        if (ret != sizeof(tmsg))
        {
            ESP_LOGE(TAG_APP, "Error read ticket: %d", ret);
        }
        ESP_LOGI(TAG_APP, "Ticket Received: %d", ret);
    }

    /*Requesting Allocate a Data Flow*/
    xDataPortId = RINA_flow_alloc(DIF, CLIENT, SERVER, xFlowSpec, Flags);

    if (xDataPortId > 0)
    {

        /*Send Ticket to identify the data flow*/
        memset(&cfg, 0, sizeof(cfg));
        cfg.opcode = RP_OPCODE_DATAFLOW;
        cfg.ticket = tmsg.ticket;

        ESP_LOGE(TAG_APP, "Sending Ticket");
        ret = RINA_flow_write(xDataPortId, &cfg, sizeof(cfg));
        if (ret != sizeof(cfg))
        {
            ESP_LOGE(TAG_APP, "Error write data");
        }
        if (cfg.opcode != opcode)
        {

            ESP_LOGI(TAG_APP, "Starting PING; message size: %d, number of messages: %d",
                     PING_SIZE, NUMBER_OF_PINGS);

            /*Calling perf_client to execute the test*/
            (void)ping_client(xDataPortId);

            ESP_LOGI(TAG_APP, "----------------------------------------");

            /*Sending Stop OpCode to finish the test*/
            memset(&cfg, 0, sizeof(cfg));
            cfg.opcode = RP_OPCODE_STOP;
            cfg.cnt = (uint64_t)1000;
            ret = RINA_flow_write(xControlPortId, &cfg, sizeof(cfg));
            if (ret != sizeof(cfg))
            {
                if (ret <= 0)
                {
                    ESP_LOGE(TAG_APP, "Error write data");
                }
            }

            vTaskDelay(100 / portTICK_RATE_MS);

            /* Reading the results message from server*/
            ret = RINA_flow_read(xControlPortId, &rmsg, sizeof(rmsg));
            if (ret != sizeof(rmsg))
            {
                if (ret < 0)
                {
                    ESP_LOGE(TAG_APP, "Error write data");
                }
                else
                {
                    ESP_LOGE(TAG_APP, "Error reading result message: wrong length %d "
                                      "(should be %lu)\n",
                             ret, (unsigned long int)sizeof(rmsg));
                }
            }
            else
            {
                // vPrintBytes((void *)&rmsg, ret);
            }

            /*Calling the Report*/
            //(void)ping_report(rmsg);
        }
    }
}