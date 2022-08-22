

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
#include "IPCP.h"
#include "IPCP_api.h"
//#include "normalIPCP.h"
#include "RINA_API.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs_flash.h"

#define TAG_APP "[PING-APP]"
#define PING_SIZE (32)
#define NUMBER_OF_PINGS (10)
#define DIF "mobile.DIF"
#define SERVER "sensor1"
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
    float average;
    int min, max, sum;

    TickType_t t_end, t1, t2;
    TickType_t t_start;
    float ns;
    // unsigned int interval = w->interval;

    char bufferTx[200];
    void *bufferRx;
    size_t xLenBufferRx = 1024;
    char *data;

    bufferRx = pvPortMalloc(xLenBufferRx);
    memset(bufferRx, 0, xLenBufferRx);

    memset(bufferTx, 'x', (size_t)(PING_SIZE)); // 32bytes

    vTaskDelay(1000);

    ESP_LOGI(TAG_APP, "----------- Requesting a Flow ----- ");

    xAppPortId = RINA_flow_alloc(DIF, CLIENT, SERVER, xFlowSpec, Flags);

    ESP_LOGI(TAG_APP, "Flow Port id: %d ", xAppPortId);

    time_start = esp_timer_get_time();
    t_start = xTaskGetTickCount();

    if (xAppPortId != -1)
    {
        ESP_LOGI(TAG_APP, "Pinging %s with %d bytes of data: ", SERVER, PING_SIZE);
        while (i < NUMBER_OF_PINGS)
        {

            time1 = esp_timer_get_time();
            t1 = xTaskGetTickCount();

            uxTxBytes = RINA_flow_write(xAppPortId, (void *)bufferTx, strlen(bufferTx));

            if (uxTxBytes == 0)
            {
                ESP_LOGE(TAG_APP, "Error to send Data");
                break;
            }

            ESP_LOGI(TAG_APP, "Sended: %d", uxTxBytes);

            uxRxBytes = 0;

            while (uxRxBytes < uxTxBytes)
            {
                uxRxBytes = RINA_flow_read(xAppPortId, (void *)bufferRx, xLenBufferRx);
                if (uxRxBytes == 0)
                {
                    ESP_LOGE(TAG_APP, "It was an error receiving the buffer");
                    break;
                }
                if (uxRxBytes > 0)
                {
                    received++;
                    t2 = xTaskGetTickCount();
                    time2 = esp_timer_get_time();

                    time_delta = time2 - time1;

                    ns = (t2 - t1) / portTICK_PERIOD_MS;

                    // ESP_LOGI(TAG_APP, "%d bytes from server: rtt = %.3f ms\n", uxRxBytes, ns);
                    ESP_LOGI(TAG_APP, "%d bytes from server: rtt = %d us\n", uxRxBytes, time_delta);
                    RTT[i] = time_delta;
                }
            }

            vTaskDelay(1000 / portTICK_PERIOD_MS);
            i = i + 1;
        }
    }

    min = max = RTT[0];
    for (i = 1; i < NUMBER_OF_PINGS; i++)
    {
        if (min > RTT[i])
            min = RTT[i];
        if (max < RTT[i])
            max = RTT[i];
    }
    sum = 0;
    for (i = 0; i < NUMBER_OF_PINGS; i++)
    {
        sum = sum + RTT[i];
    }
    average = (sum / NUMBER_OF_PINGS) / 1000;

    ESP_LOGI(TAG_APP, "Ping Statistics to for %s:", SERVER);
    ESP_LOGI(TAG_APP, "     Packets: send = %d, received = %d , timeout = %d", NUMBER_OF_PINGS, received, NUMBER_OF_PINGS - received);
    ESP_LOGI(TAG_APP, "Approximate round trip times in milliseconds: ");
    ESP_LOGI(TAG_APP, "     Minimum = %dms , Maximum = %dms, Average = %fms ", min / 1000, max / 1000, average);
}
