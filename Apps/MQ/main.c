#include "freertos/FreeRTOS.h"
#include <string.h>

#include "configRINA.h"
#include "IPCP.h"
#include "RINA_API.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "mq.h"

#define TAG_APP "[Sensor-APP]"

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
	int i = 0;
	char json[200];
	float CO;

	char buffer[500];

	vTaskDelay(1000);

	ESP_LOGI(TAG_APP, "----------- Requesting a Flow ----- ");

	xAppPortId = RINA_flow_alloc("mobile.DIF", "QA1", "sensor3", xFlowSpec, Flags);

	ESP_LOGI(TAG_APP, "Flow Port id: %d ", xAppPortId);
	if (xAppPortId != -1)
	{

		while (pdTRUE)
		{

			CO = readData();

			ESP_LOGI(TAG_APP, "CO2: %.1f ppm", CO);
			sprintf(json, "%.1f,0 \n", CO);

			if (RINA_flow_write(xAppPortId, (void *)json, strlen(json)))
			{
				ESP_LOGI(TAG_APP, "Sent Data successfully");
			}

			vTaskDelay(30000);

			i = i + 1;
		}
	}
}
