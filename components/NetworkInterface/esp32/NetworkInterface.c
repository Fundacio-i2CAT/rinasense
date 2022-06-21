/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FreeRTOS includes. */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

/* RINA Components includes. */
//#include "ARP826.h"
#include "ShimIPCP.h"
#include "BufferManagement.h"
#include "NetworkInterface.h"
#include "configRINA.h"
#include "configSensor.h"

#include "IPCP.h" //temporal
#include "IPCP_api.h"
#include "IPCP_events.h"

/* ESP includes.*/
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_event_base.h"
#include "netif/wlanif.h"
#include "esp_private/wifi.h"

//#include "nvs_flash.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

#define MAX_STA_CONN (5)
#define ESP_MAXIMUM_RETRY MAX_STA_CONN

enum if_state_t
{
	INTERFACE_DOWN = 0,
	INTERFACE_UP,
};

/*Variable State of Interface */
volatile static uint32_t xInterfaceState = INTERFACE_DOWN;

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

// Retry trying to connect initialization
static int s_retry_num = 0;

BaseType_t event_loop_inited = pdFALSE;

esp_err_t xNetworkInterfaceInput(void *buffer, uint16_t len, void *eb);

// NetworkBufferDescriptor_t * pxNetworkBuffer;

static void event_handler(void *arg, esp_event_base_t event_base,
						  int32_t event_id, void *event_data)
{
	esp_err_t ret;

	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
		ret = esp_wifi_connect();
		ESP_LOGI(TAG_WIFI, "ret:%d", ret);
		/*if (ret==0)
		{
			xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
		}*/
		//
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
		if (s_retry_num < ESP_MAXIMUM_RETRY)
		{
			ret = esp_wifi_connect();
			ESP_LOGI(TAG_WIFI, "ret:%d", ret);
			s_retry_num++;
			ESP_LOGI(TAG_WIFI, "retry to connect to the AP");
		}
		else
		{
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
		ESP_LOGI(TAG_WIFI, "connect to the AP fail");
	}
	else
	{
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

/* Initialize the Network Interface
 *  MAC_ADDRESS_LENGTH_BYTES (6) defined in the Shim_IPCP.h
 *  Get the Mac Address from the ESP_WIFI. Then copy it into the
 *  Mac address variable in the ShimIPCP.h
 *  Return a Boolean pdTrue or pdFalse
 * */
BaseType_t xNetworkInterfaceInitialise(const MACAddress_t *pxPhyDev)
{
	ESP_LOGI(TAG_WIFI, "%s", __func__);
	uint8_t ucMACAddress[MAC_ADDRESS_LENGTH_BYTES];

	esp_efuse_mac_get_default(ucMACAddress);
	vARPUpdateMACAddress(ucMACAddress, pxPhyDev);
	return pdTRUE;
}

BaseType_t xNetworkInterfaceConnect(void)
{
	ESP_LOGI(TAG_WIFI, "%s", __func__);

	if (event_loop_inited == pdTRUE)
	{
		esp_wifi_stop();
		esp_wifi_deinit();
		s_retry_num = 0;
	}

	ESP_LOGI(TAG_WIFI, "Creating group");
	s_wifi_event_group = xEventGroupCreate();
	/*MAC address initialized to pdFALSE*/
	static BaseType_t xMACAdrInitialized = pdFALSE;
	uint8_t ucMACAddress[MAC_ADDRESS_LENGTH_BYTES];

	if (event_loop_inited == pdFALSE)
	{
		ESP_LOGI(TAG_SHIM, "Creating event Loop");

		ESP_ERROR_CHECK(esp_event_loop_create_default()); // EventTask init
		// esp_netif_create_default_wifi_sta();// Binding STA with TCP/IP
		event_loop_inited = pdTRUE;
	}

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // @suppress("Type cannot be resolved")
	ESP_LOGI(TAG_SHIM, "Init Network Interface with Config file");
	ESP_ERROR_CHECK(esp_wifi_init(&cfg)); // WiFi driver Task with default config

	esp_event_handler_instance_t instance_any_id;

	ESP_LOGI(TAG_SHIM, "Registering event handler");
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
														ESP_EVENT_ANY_ID,
														&event_handler,
														NULL,
														&instance_any_id));

	wifi_config_t wifi_config = {
		.sta = {
			.ssid = ESP_WIFI_SSID,
			.password = ESP_WIFI_PASS,
			.threshold.authmode = WIFI_AUTH_WPA2_PSK,
			.pmf_cfg = {
				.capable = true,
				.required = false},
		},
	};

	ESP_LOGI(TAG_SHIM, "Setting mode and starting wifi");
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
	ESP_LOGI(TAG_WIFI, "esp_wifi_starting as station");
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(TAG_WIFI, "wifi_init_sta finished.");

	/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
	 * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
										   WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
										   pdFALSE,
										   pdFALSE,
										   portMAX_DELAY);

	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
	 * happened. */

	// vTaskDelay(1000 / portTICK_PERIOD_MS);
	if (bits & WIFI_CONNECTED_BIT)
	{
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		ESP_LOGI(TAG_WIFI, "OK");
		ESP_LOGI(TAG_WIFI, "connected to ap SSID:%s password:%s",
				 ESP_WIFI_SSID, ESP_WIFI_PASS);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		xInterfaceState = INTERFACE_UP;
	}
	else if (bits & WIFI_FAIL_BIT)
	{
		ESP_LOGI(TAG_WIFI, "Failed to connect to SSID:%s, password:%s",
				 ESP_WIFI_SSID, ESP_WIFI_PASS);
		xInterfaceState = INTERFACE_DOWN;
	}
	else
	{
		ESP_LOGE(TAG_WIFI, "UNEXPECTED EVENT");
	}

	/* The event will not be processed after unregister */

	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
	vEventGroupDelete(s_wifi_event_group);

	ESP_ERROR_CHECK(esp_wifi_internal_reg_rxcb(ESP_IF_WIFI_STA, xNetworkInterfaceInput));

	if (xInterfaceState == INTERFACE_UP)
	{
		if (xMACAdrInitialized == pdFALSE)
		{
			// Wifi_Interface: WIFI_STA or WIFI_AP. This case WIFI_STA
			esp_wifi_get_mac(ESP_IF_WIFI_STA, ucMACAddress);
			// Function from Shim_IPCP to update Source MACAddress.
			// vARPUpdateMACAddress(ucMACAddress, pxPhyDev); //Maybe change the method name.
			xMACAdrInitialized = pdTRUE;
			// ESP_LOGI(TAG_WIFI,"MACAddressRINA updated");
		}

		return pdTRUE;
	}
	else
	{
		ESP_LOGI(TAG_WIFI, "Interface Down");
		return pdFALSE;
	}
}

BaseType_t xNetworkInterfaceOutput(NetworkBufferDescriptor_t *const pxNetworkBuffer,
								   BaseType_t xReleaseAfterSend)
{
	if ((pxNetworkBuffer == NULL) || (pxNetworkBuffer->pucEthernetBuffer == NULL) || (pxNetworkBuffer->xDataLength == 0))
	{
		ESP_LOGE(TAG_WIFI, "Invalid parameters");
		return pdFALSE;
	}

	esp_err_t ret;

	if (xInterfaceState == INTERFACE_DOWN)
	{
		ESP_LOGI(TAG_WIFI, "Interface down");
		ret = ESP_FAIL;
	}
	else
	{

		ret = esp_wifi_internal_tx(ESP_IF_WIFI_STA, pxNetworkBuffer->pucEthernetBuffer, pxNetworkBuffer->xDataLength);

		if (ret != ESP_OK)
		{
			ESP_LOGE(TAG_WIFI, "Failed to tx buffer %p, len %d, err %d", pxNetworkBuffer->pucEthernetBuffer, pxNetworkBuffer->xDataLength, ret);
		}
		else
		{
			ESP_LOGI(TAG_WIFI, "Packet Sended"); //
		}
	}

	if (xReleaseAfterSend == pdTRUE)
	{
		// ESP_LOGE(TAG_WIFI, "Releasing Buffer interface WiFi after send");
		vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
	}

	return ret == ESP_OK ? pdTRUE : pdFALSE;
}

BaseType_t xNetworkInterfaceDisconnect(void)
{

	esp_err_t ret;

	ret = esp_wifi_disconnect();

	ESP_LOGI(TAG_WIFI, "Interface was disconnected");

	return ret == ESP_OK ? pdTRUE : pdFALSE;
}

esp_err_t xNetworkInterfaceInput(void *buffer, uint16_t len, void *eb)
{
	NetworkBufferDescriptor_t *pxNetworkBuffer;
	RINAStackEvent_t xRxEvent = {eNetworkRxEvent, NULL};
	const TickType_t xDescriptorWaitTime = pdMS_TO_TICKS(250);
    struct timespec ts;

	if (eConsiderFrameForProcessing(buffer) != eProcessBuffer)
	{

		esp_wifi_internal_free_rx_buffer(eb);
		return ESP_OK;
	}

    if (!rstime_waitmsec(&ts, 250))
        return ESP_FAIL;

    pxNetworkBuffer = pxGetNetworkBufferWithDescriptor(len, &ts);
	// ESP_LOGE(TAG_WIFI,"xNetworkInterfaceInput Taking buffer to copy wifidriver buffer");

	if (pxNetworkBuffer != NULL)
	{

		/* Set the packet size, in case a larger buffer was returned. */
		pxNetworkBuffer->xEthernetDataLength = len;

		/* Copy the packet data. */
		memcpy(pxNetworkBuffer->pucEthernetBuffer, buffer, len);
		xRxEvent.pvData = (void *)pxNetworkBuffer;

		// ESP_LOGE(TAG_RINA, "pucEthernetBuffer and len: %p, %d", pxNetworkBuffer->pucEthernetBuffer, len);

		if (xSendEventStructToIPCPTask(&xRxEvent, xDescriptorWaitTime) == pdFAIL)
		{
			ESP_LOGE(TAG_WIFI, "Failed to enqueue packet to network stack %p, len %d", buffer, len);
			vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
			return ESP_FAIL;
		}

		esp_wifi_internal_free_rx_buffer(eb);
		return ESP_OK;
	}

	else
	{
		ESP_LOGE(TAG_WIFI, "Failed to get buffer descriptor");
		vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
		return ESP_FAIL;
	}
}

void vNetworkNotifyIFDown()
{
	RINAStackEvent_t xRxEvent = {eNetworkDownEvent, NULL};

	if (xInterfaceState != INTERFACE_DOWN)
	{
		xInterfaceState = INTERFACE_DOWN;
		xSendEventStructToIPCPTask(&xRxEvent, 0);
	}
}

void vNetworkNotifyIFUp()
{
	xInterfaceState = INTERFACE_UP;
}
