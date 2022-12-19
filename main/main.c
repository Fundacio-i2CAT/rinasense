

/**
 * @file main.c
 * @author David Sarabia (david.sarabia@i2cat.net)
 * @brief Dummy application that writes a json file into the RINA flow. This json file is constant and aims
 * to emulate the json file made from the DHT sensor. This dummy application aims to test the RINA_flow_write
 * and the RINA_flow_read APIs.
 *
 * @version 0.1
 * @date 2022-07-25
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "freertos/FreeRTOS.h"
#include <string.h>

#include "configRINA.h"
#include "IPCP.h"
#include "IPCP_api.h"
#include "RINA_API.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#define TAG_APP "[Sensor-APP]"

void app_main(void)
{
	nvs_flash_init();

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
	size_t uxTxBytes = 0;

	buffer = pvPortMalloc(xLenBuffer);

	memset(buffer, 0, xLenBuffer);

	vTaskDelay(4000);

	ESP_LOGD(TAG_APP, "----------- Requesting a Flow ----- ");

	xAppPortId = RINA_flow_alloc("slice1.DIF", "st1", "sensorHub", xFlowSpec, Flags);

	ESP_LOGD(TAG_APP, "Flow Port id: %d ", xAppPortId);
	if (xAppPortId != -1)
	{

		while (i < 100)
		{

			// ESP_LOGD(TAG_APP, "Temperature: 30 C");

			sprintf(json, "Temperature: 30 C, Humidity: 24%%\n");

			uxTxBytes = RINA_flow_write(xAppPortId, (void *)json, strlen(json));

			if (uxTxBytes == 0)
			{
				ESP_LOGE(TAG_APP, "Error to send Data");
				break;
			}
			if (uxTxBytes > 0)
			{
				ESP_LOGI(TAG_APP, "Sent Data successfully");
			}

			vTaskDelay(8000 / portTICK_RATE_MS);

			i = i + 1;
		}
	}
}