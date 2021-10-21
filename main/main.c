#include "freertos/FreeRTOS.h"
#include <string.h>

#include "ARP826.h"
#include "BufferManagement.h"
#include "NetworkInterface.h"
#include "ShimIPCP.h"
#include "configSensor.h"
#include "IPCP.h"

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


	//Shim_WiFiInit(  );
	RINA_IPCPInit( );


}

