

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
    int32_t xBytes;
    int i = 0;
    char json[200];
    void *buffer;
    size_t xLenBuffer = 1024;
    char *data;

    buffer = pvPortMalloc(xLenBuffer);

    memset(buffer, 0, xLenBuffer);

    vTaskDelay(1000);

    LOGI(TAG_APP, "----------- Requesting a Flow ----- ");

    xAppPortId = RINA_flow_alloc("slice1.DIF", "ST1", "sensor1", xFlowSpec, Flags);

    LOGI(TAG_APP, "Flow Port id: %d ", xAppPortId);
    if (xAppPortId != -1)
    {

        while (i < 100)
        {

            // ESP_LOGI(TAG_APP, "Temperature: 30 C");

            sprintf(json, "Temperature: 30 C\n");

            LOGI(TAG_APP, "json:%s", json);
            if (RINA_flow_write(xAppPortId, (void *)json, strlen(json)))
            {
                LOGI(TAG_APP, "Sent Data successfully");
            }

            /*xBytes = RINA_flow_read(xAppPortId, (void *)buffer, xLenBuffer);

            if (xBytes > 0)
            {
                data = strdup(buffer);
                ESP_LOGI(TAG_APP, "Receive data");
                ESP_LOGI(TAG_APP, "Buffer: %s", data);
                ESP_LOGI(TAG_APP, "Bytes received: %d", xBytes);
            }
            if (xBytes == 0)
            {
                ESP_LOGI(TAG_APP, "It was an error receiving the buffer");
            }*/

            vTaskDelay(8000 / portTICK_RATE_MS);

            i = i + 1;
        }
    }
}
