#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_ieee802154.h>

#include "ieee802154_NetworkInterface.h"
#include "ieee802154_frame.h"

#define TAG_APP "IEEE802154_TEST"
#define TEST_DATA "Hello 802.15.4"
#define TEST_DELAY_MS 5000
#define TEST_ITERATIONS 10

void app_main(void)
{
   
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    LOGI(TAG_APP, "Initializing IEEE 802.15.4 interface...");

    MACAddress_t xPhyDev;
    if (xIeee802154NetworkInterfaceInitialise(&xPhyDev))
    {
        LOGI(TAG_APP, "Interface initialized successfully");
    }
    else
    {
        LOGE(TAG_APP, "Failed to initialize IEEE 802.15.4 interface");
        return;
    }


    if (xIeee802154NetworkInterfaceConnect())
    {
        LOGI(TAG_APP, "Connected to IEEE 802.15.4 network");
    }
    else
    {
        LOGE(TAG_APP, "Failed to connect to IEEE 802.15.4 network");
        return;
    }

    LOGI(TAG_APP, "Starting transmission test...");
    for (int i = 0; i < TEST_ITERATIONS; i++)
    {
        NetworkBufferDescriptor_t *pxNetworkBuffer = pxGetNetworkBufferWithDescriptor(strlen(TEST_DATA), 0);
        if (pxNetworkBuffer != NULL)
        {
            memcpy(pxNetworkBuffer->pucEthernetBuffer, TEST_DATA, strlen(TEST_DATA));
            pxNetworkBuffer->xDataLength = strlen(TEST_DATA);

            if (xIeee802154NetworkInterfaceOutput(pxNetworkBuffer, pdTRUE))
            {
                LOGI(TAG_APP, "Packet %d sent successfully", i + 1);
            }
            else
            {
                LOGE(TAG_APP, "Failed to send packet %d", i + 1);
            }
        }
        else
        {
            LOGE(TAG_APP, "Failed to allocate network buffer");
        }

        vTaskDelay(pdMS_TO_TICKS(TEST_DELAY_MS));
    }

    LOGI(TAG_APP, "Waiting for incoming packets...");
    vTaskDelay(pdMS_TO_TICKS(60000));

    if (xIeee802154NetworkInterfaceDisconnect())
    {
        LOGI(TAG_APP, "Disconnected from IEEE 802.15.4 network");
    }
    else
    {
        LOGE(TAG_APP, "Failed to disconnect from IEEE 802.15.4 network");
    }

    LOGI(TAG_APP, "Test completed.");
}
