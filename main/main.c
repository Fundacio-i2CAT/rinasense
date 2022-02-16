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



void app_main(void)
{
	nvs_flash_init();
	/*
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);*/

	//ESP_LOGI(TAG_WIFI, "ESP_WIFI_MODE_STA");
	//ESP_LOGI(TAG_WIFI, "Enrolling to DIF");

	

	RINA_IPCPInit( );

	portId_t test = 0;
	struct rinaFlowSpec_t *xFlowSpec = pvPortMalloc(sizeof(*xFlowSpec));
	uint8_t Flags=1;

	vTaskDelay(1000);



	test = RINA_flow_alloc("Irati", "Test|1|Testing|1", "TestRemote", xFlowSpec, Flags);




}

