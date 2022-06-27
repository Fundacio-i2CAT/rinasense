#include "freertos/FreeRTOS.h"
#include <string.h>

//#include "ARP826.h"
//#include "BufferManagement.h"
//#include "NetworkInterface.h"
//#include "ShimIPCP.h"
#include "configRINA.h"
#include "IPCP.h"
//#include "normalIPCP.h"
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
	char temperature[16];

	// char *ucStringTest = "Temperature:22";

	/*Getting the buffer Descriptor*/
	// xBufferSize = strlen(ucStringTest);

	vTaskDelay(1000);

	ESP_LOGI(TAG_APP, "----------- Requesting a Flow ----- ");

	xAppPortId = RINA_flow_alloc("mobile.DIF", "Temperature", "sensor1", xFlowSpec, Flags);

	ESP_LOGI(TAG_APP, "Flow Port id: %d ", xAppPortId);
	if (xAppPortId != -1)
	{

		while (i < 10)
		{
			sprintf(temperature, "Temperature:30C");
			// temperature = "Temperature:32C";

			if (RINA_flow_write(xAppPortId, (void *)temperature, strlen(temperature)))
			{
				ESP_LOGI(TAG_APP, "Sent Data: %s successfully", temperature);
			}
			i = i + 1;
		}
	}
}
