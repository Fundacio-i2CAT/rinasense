

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "configRINA.h"
#include "IPCP_api.h"
#include "RINA_API.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"

#include "nvs_flash.h"

#define TAG_APP "[PING-APP]"
#define PING_SIZE 1024U
#define NUMBER_OF_PINGS (200)
#define DIF "mobile.DIF"
#define SERVER "pingserver"
#define CLIENT "ping"

void app_main(void)
{

    nvs_flash_init();
    /*
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);*/

    RINA_IPCPInit();

    portId_t xAppPortId;
    struct rinaFlowSpec_t *xFlowSpec = pvPortMalloc(sizeof(*xFlowSpec));
    uint8_t Flags = 1;
    int32_t uxRxBytes = 0;
    size_t uxTxBytes;
    int i = 0;

    int RTT[NUMBER_OF_PINGS];
    int received = 0;

    int64_t time_start,
        time_end,
        time1,
        time2;
    int time_delta;
    int time_delta2;
    float result;
    float average = 0;
    int min, max, sum;

    float ns;

    void *bufferRx;

    size_t size_ping = (size_t)PING_SIZE + 1;
    void *bufferTx;

    bufferRx = pvPortMalloc(size_ping);
    bufferTx = pvPortMalloc(size_ping);

    memset(bufferRx, 0, size_ping);

    memset(bufferTx, 'x', (size_t)PING_SIZE);
    memset(bufferTx + PING_SIZE, '\0', 1);

    LOGI(TAG_APP, "Pinging %s with %d bytes of data: ", SERVER, strlen(bufferTx));

    vTaskDelay(2000);

    LOGD(TAG_APP, "----------- Requesting a Flow ----- ");

    xAppPortId = RINA_flow_alloc(DIF, CLIENT, SERVER, xFlowSpec, Flags);

    LOGI(TAG_APP, "Flow Allocated at Port id: %u ", xAppPortId);

    // vTaskDelay(100);

    time_start = esp_timer_get_time();

    if (xAppPortId != -1)
    {
        LOGI(TAG_APP, "Pinging %s with %d bytes of data: ", SERVER, strlen(bufferTx));
        while (i < NUMBER_OF_PINGS)
        {

            time1 = esp_timer_get_time();

            uxTxBytes = RINA_flow_write(xAppPortId, (void *)bufferTx, strlen(bufferTx));

            if (uxTxBytes == 0)
            {
                LOGE(TAG_APP, "Error to send Data");
                break;
            }

            LOGD(TAG_APP, "Sended: %d", uxTxBytes);

            uxRxBytes = 0;

            while (uxRxBytes < uxTxBytes)
            {
                // vTaskDelay(10 / portTICK_RATE_MS);
                uxRxBytes = RINA_flow_read(xAppPortId, (void *)bufferRx, size_ping);
                time2 = esp_timer_get_time();

                if (uxRxBytes == 0)
                {
                    LOGE(TAG_APP, "It was an error receiving the buffer");
                    break;
                }
                if (uxRxBytes > 0)
                {
                    received++;
                    time_delta = time2 - time1;

                    // ESP_LOGD(TAG_APP, "%d bytes from server: rtt = %.3f ms\n", uxRxBytes, ns);
                    // ESP_LOGI(TAG_APP, "%d bytes from server: rtt = %d ms", uxRxBytes, time_delta / 1000);
                    LOGI(TAG_APP, "%i bytes from server: rtt = %.3f ms", uxRxBytes, (float)time_delta / 1000);
                    RTT[i] = time_delta;
                }
                else
                {
                    LOGI(TAG_APP, "Request time out");
                    RTT[i] = 0;
                }
            }

            vTaskDelay(1000 / portTICK_RATE_MS);
            i = i + 1;
        }
    }
    time_end = esp_timer_get_time();
    time_delta2 = time_end - time_start;

    result = 1000000 * received;
    result = result / time_delta2;
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

    LOGI(TAG_APP, "Ping Statistics to for %s:", SERVER);
    LOGI(TAG_APP, "     Packets: send = %d, received = %d , timeout = %d", NUMBER_OF_PINGS, received, NUMBER_OF_PINGS - received);
    LOGI(TAG_APP, "Approximate round trip times in milliseconds: ");
    LOGI(TAG_APP, "     Minimum = %.3f ms , Maximum = %.3f ms, Average = %.3f ms ", (float)min / 1000, (float)max / 1000, average / 1000);
    // ESP_LOGI(TAG_APP, "     Througput = %f bps\n", result);

    /* if (RINA_flow_close(xAppPortId))
     {
         ESP_LOGI(TAG_APP, "Flow Deallocated");
     }
     else
     {
         ESP_LOGI(TAG_APP, "It was not possible to deallocated the flow");
     } */
}