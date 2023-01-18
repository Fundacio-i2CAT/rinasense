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

#include  "common/mac.h"

/* RINA Components includes. */
//#include "ARP826.h"
#include "ShimIPCP.h"
#include "ShimIPCP_instance.h"
#include "NetworkInterface.h"
#include "configRINA.h"

#include "IPCP.h" //temporal
#include "IPCP_api.h"
#include "IPCP_events.h"

/* ESP includes.*/
#include "esp_wifi.h"
#if ESP_IDF_VERSION_MAJOR > 4
#include "esp_mac.h"
#endif
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

/* TEMPORARY: blind pointer to the IPCP instance data so we can pass
 * it to the shim when we receive packets. */
struct ipcpInstance_t *pxSelf;

// NetworkBufferDescriptor_t * pxNetworkBuffer;

static void event_handler(void *arg, esp_event_base_t event_base,
						  int32_t event_id, void *event_data)
{
	esp_err_t ret;

    LOGD(TAG_WIFI, "ENTERING: wifi event handler");

	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
		ret = esp_wifi_connect();
		LOGI(TAG_WIFI, "ret:%d", ret);
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
			LOGI(TAG_WIFI, "ret:%d", ret);
			s_retry_num++;
			LOGI(TAG_WIFI, "retry to connect to the AP");
		}
		else
		{
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
		LOGI(TAG_WIFI, "connect to the AP fail");
	}
	else
	{
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}

    LOGD(TAG_WIFI, "EXITING: wifi event handler");
}

/* Initialize the Network Interface
 *  MAC_ADDRESS_LENGTH_BYTES (6) defined in the Shim_IPCP.h
 *  Get the Mac Address from the ESP_WIFI. Then copy it into the
 *  Mac address variable in the ShimIPCP.h
 *  Return a Boolean pdTrue or pdFalse
 * */
BaseType_t xNetworkInterfaceInitialise(struct ipcpInstance_t *pxS, MACAddress_t *pxPhyDev)
{
    pxSelf = pxS;
    stringbuf_t pcMac[MAC2STR_MIN_BUFSZ];

    /* This is supposed to only returns an error if the argument is an
     * invalid address. */
	RsAssert(esp_efuse_mac_get_default(pxPhyDev->ucBytes) == ESP_OK);
    mac2str((const MACAddress_t *)&pxPhyDev, pcMac, sizeof(pcMac));

	LOGI(TAG_WIFI, "Initialized the network interface, MAC address: %s", pcMac);

    /* FIXME: The previous calls reads the MAC address to use, but
     * as of the commit carrying this comment, I've not looked into
     * what else I need to be doing here. */

	return pdTRUE;
}

BaseType_t xNetworkInterfaceConnect(void)
{
	LOGI(TAG_WIFI, "%s", __func__);

	if (event_loop_inited == pdTRUE)
	{
		esp_wifi_stop();
		esp_wifi_deinit();
		s_retry_num = 0;
	}

	LOGI(TAG_WIFI, "Creating group");
	s_wifi_event_group = xEventGroupCreate();
    RsAssert(s_wifi_event_group);

	/*MAC address initialized to pdFALSE*/
	static BaseType_t xMACAdrInitialized = pdFALSE;
	uint8_t ucMACAddress[MAC_ADDRESS_LENGTH_BYTES];

	if (event_loop_inited == pdFALSE)
	{
		LOGI(TAG_SHIM, "Creating event Loop");

		ESP_ERROR_CHECK(esp_event_loop_create_default()); // EventTask init
		// esp_netif_create_default_wifi_sta();// Binding STA with TCP/IP
		event_loop_inited = pdTRUE;
	}

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // @suppress("Type cannot be resolved")
	LOGI(TAG_SHIM, "Init Network Interface with Config file");
	ESP_ERROR_CHECK(esp_wifi_init(&cfg)); // WiFi driver Task with default config

	esp_event_handler_instance_t instance_any_id;

	LOGI(TAG_SHIM, "Registering event handler");
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
														ESP_EVENT_ANY_ID,
														&event_handler,
														NULL,
														&instance_any_id));

	wifi_config_t wifi_config = {
		.sta = {
			.ssid = CFG_ESP_WIFI_SSID,
			.password = CFG_ESP_WIFI_PASS,
			.threshold.authmode = WIFI_AUTH_WPA2_PSK,
			.pmf_cfg = {
				.capable = true,
				.required = false},
		},
	};

	LOGI(TAG_SHIM, "Setting mode and starting wifi");
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
	LOGI(TAG_WIFI, "esp_wifi_starting as station");
	ESP_ERROR_CHECK(esp_wifi_start());

	LOGI(TAG_WIFI, "wifi_init_sta finished, waiting for events...");

	/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
	 * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
										   WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
										   pdFALSE,
										   pdFALSE,
										   portMAX_DELAY);

    LOGD(TAG_WIFI, "xEventGroupWaitBits returned!");

	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
	 * happened. */

	// vTaskDelay(1000 / portTICK_PERIOD_MS);
	if (bits & WIFI_CONNECTED_BIT)
	{
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		LOGI(TAG_WIFI, "OK");
		LOGI(TAG_WIFI, "connected to ap SSID:%s password:%s",
             CFG_ESP_WIFI_SSID, CFG_ESP_WIFI_PASS);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		xInterfaceState = INTERFACE_UP;
	}
	else if (bits & WIFI_FAIL_BIT)
	{
		LOGI(TAG_WIFI, "Failed to connect to SSID:%s, password:%s",
             CFG_ESP_WIFI_SSID, CFG_ESP_WIFI_PASS);
		xInterfaceState = INTERFACE_DOWN;
	}
	else
	{
		LOGE(TAG_WIFI, "UNEXPECTED EVENT");
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
		LOGI(TAG_WIFI, "Interface Down");
		return pdFALSE;
	}
}

BaseType_t xNetworkInterfaceOutput(netbuf_t *pxNbFrame)
{
	esp_err_t ret;
    void *pvBuf;
    size_t unTotalSz;

	if (xInterfaceState == INTERFACE_DOWN)
	{
		LOGI(TAG_WIFI, "Interface down");
		ret = ESP_FAIL;
	}
	else
	{
        unTotalSz = unNetBufTotalSize(pxNbFrame);

        if (!(pvBuf = pvRsMemAlloc(unTotalSz))) {
            LOGE(TAG_WIFI, "Failed to allocate memory for write buffer");
            return false;
        }

        if (unNetBufRead(pxNbFrame, pvBuf, 0, unTotalSz) != unTotalSz) {
            LOGE(TAG_WIFI, "Failed to transfer netbufs content to write buffer");
            vRsMemFree(pvBuf);
        }


		ret = esp_wifi_internal_tx(ESP_IF_WIFI_STA, pvBuf, unTotalSz);

		if (ret != ESP_OK)
		{
			LOGE(TAG_WIFI, "Failed to tx buffer %p, len %u, err %d", pvBuf, unTotalSz, ret);
		}
		else
		{
			LOGI(TAG_WIFI, "Packet Sended"); //
		}
	}

    //vNetBufFreeAll(pxNbFrame);

	return ret == ESP_OK ? pdTRUE : pdFALSE;
}

BaseType_t xNetworkInterfaceDisconnect(void)
{

	esp_err_t ret;

	ret = esp_wifi_disconnect();

	LOGI(TAG_WIFI, "Interface was disconnected");

	return ret == ESP_OK ? pdTRUE : pdFALSE;
}

void vNetBufFreeESP32(netbuf_t *pxNb)
{
    if (pxNb->pxBufStart) {
        RsAssert(pxNb->pvExtra);

        LOGD("[debug]", "FREEING RX BUFFER %p", pxNb->pvExtra);

        esp_wifi_internal_free_rx_buffer(pxNb->pvExtra);

        pxNb->pxBufStart = NULL;
    }
}

/* Called by the tap read thread to advertises that an ethernet
 * frame has arrived. */
esp_err_t xNetworkInterfaceInput(void *buffer, uint16_t len, void *eb)
{
    EthernetHeader_t *pxEthernetHeader;
    size_t unSzFrData;
    netbuf_t *pxNbFrame;

    /* Shortcut the frame processing if we do not have to deal with
     * the packet type. */
	if (eConsiderFrameForProcessing(buffer) != eProcessBuffer) {
        esp_wifi_internal_free_rx_buffer(eb);
        return ESP_OK;
    }

    if (!(pxNbFrame = pxNetBufNew(pxSelf->pxData->pxNbPool, NB_ETH_HDR, buffer, len, &vNetBufFreeESP32))) {
        LOGE(TAG_WIFI, "Failed to allocate netbuf for incoming message");
        return ESP_OK;
    }

    LOGD("[debug]", "SETTING RX BUFFER %p", eb);

    vNetBufSetExtra(pxNbFrame, eb);

    /* We need to look into the frame data. */
    pxEthernetHeader = (EthernetHeader_t *)pvNetBufPtr(pxNbFrame);;

    /* If it's a broadcasted packet, replace the target address in
     * the frame by ours. The rest of the code should not have to
     * deal with this. */
    if (xIsBroadcastMac(&pxEthernetHeader->xDestinationAddress)) {
        LOGD(TAG_WIFI, "Handling broadcasted ethernet packet.");
        memcpy(&pxEthernetHeader->xDestinationAddress, pxSelf->pxData->xPhyDev.ucBytes, sizeof(MACAddress_t));

    } else {
        /* Make sure this is actually meant from us. */
        if (memcmp(&pxEthernetHeader->xDestinationAddress,
                   pxSelf->pxData->xPhyDev.ucBytes, sizeof(MACAddress_t)) != 0) {
#ifndef NDEBUG
            {
                stringbuf_t ucMac[MAC2STR_MIN_BUFSZ];
                mac2str(&pxEthernetHeader->xDestinationAddress, ucMac, MAC2STR_MIN_BUFSZ);
                LOGW(TAG_WIFI, "Dropping packet with destination %s, not for us", ucMac);
                return ESP_OK;
            }
#else
            LOGW(TAG_WIFI, "Dropping ethernet packet not destined for us");
            return ESP_OK;
#endif
        }
    }

    /* Copy the packet data. */
    vShimHandleEthernetPacket(pxSelf, pxNbFrame);

    return ESP_OK;
}

void vNetworkNotifyIFDown()
{
	RINAStackEvent_t xRxEvent = {
        .eEventType = eNetworkDownEvent,
        .xData.PV = NULL
    };

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
