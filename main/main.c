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
	int i;
	char *temperature;

	vTaskDelay(1000);

	xAppPortId = RINA_flow_alloc("mobile.DIF", "Temperature", "sensor1", xFlowSpec, Flags);

	if (xAppPortId != -1)
	{
		for (i = 0; i > 10; i++)
		{
			temperature = "32 Celsius";
			if (RINA_flow_write(xAppPortId, (void *)temperature, sizeof(temperature)))
			{
				ESP_LOGI(TAG_APP, "Sent Data: %s successfully", temperature);
			}
		}
	}
}
