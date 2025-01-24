/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "portability/port.h"

/* FreeRTOS includes. */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

/* RINA Components includes. */
#include "BufferManagement.h"
#include "ieee802154_NetworkInterface.h"
#include "configRINA.h"
#include "configSensor.h"
#include "common/mac.h"

#include "ieee802154_frame.h"

#include "common/rina_common.h"

/* ESP includes.*/
#include "esp_ieee802154.h"
#if ESP_IDF_VERSION_MAJOR > 4
#include "esp_mac.h"
#endif
#include "esp_event.h"
#include "esp_system.h"
#include "esp_event_base.h"
#include <esp_log.h>

/* Constants */
#define TAG_802154 "ieee802154"

enum if_state_t
{
    DOWN = 0,
    UP,
};

/* Variable State of Interface */
volatile static uint32_t xInterfaceState = DOWN;
static QueueHandle_t rx_queue = NULL;

/* Forward declarations */
static void handler_task(void *pvParameters);

static void handler_task(void *pvParameters)
{
    uint8_t packet[257];
    while (xQueueReceive(rx_queue, &packet, portMAX_DELAY) != false)
    {
        LOGI(TAG_802154, "Processing received packet of length: %d", packet[0]);
        xIeee802154NetworkInterfaceInput(&packet[1], packet[0], NULL);
    }

    ESP_LOGE(TAG_802154, "Handler task terminated");
    vTaskDelete(NULL);
}

bool_t xIeee802154NetworkInterfaceInitialise(MACAddress_t *pxPhyDev)
{
    LOGI(TAG_802154, "Initializing the network interface 802.15.4");

    /* Init ieee802154 interface */
    esp_ieee802154_enable();
    esp_ieee802154_set_rx_when_idle(true);
    esp_ieee802154_set_promiscuous(true);

    uint8_t ucMACAddress[MAC_ADDRESS_LENGTH_BYTES];
    esp_read_mac(ucMACAddress, ESP_MAC_IEEE802154);

    /* Reverse the MAC address */
    for (int i = 0; i < MAC_ADDRESS_LENGTH_BYTES; i++)
    {
        pxPhyDev->ucBytes[i] = ucMACAddress[MAC_ADDRESS_LENGTH_BYTES-1 - i];
    }

    esp_ieee802154_set_extended_address(pxPhyDev->ucBytes);
    esp_ieee802154_set_short_address(ieee802154_SHORT_ADDRESS);

    xInterfaceState = UP;

    rx_queue = xQueueCreate(8, 257);
    xTaskCreate(handler_task, "handler_task", 8192, NULL, 20, NULL);

    return true;
}

bool_t xIeee802154NetworkInterfaceConnect(void){
    LOGI(TAG_802154, "Connecting to IEEE 802.15.4 network");

#ifdef ieee802154_COORDINATOR
    esp_ieee802154_set_coordinator(true);
#else
    esp_ieee802154_set_coordinator(false);
#endif
    esp_ieee802154_set_channel(ieee802154_CHANNEL);
    esp_ieee802154_set_panid(ieee802154_PANID_SOURCE);

    esp_ieee802154_receive();

    uint8_t extended_address[8];
    esp_ieee802154_get_extended_address(extended_address);
    LOGI(TAG_802154, "Connected: PAN ID: 0x%04x, Channel: %d, MAC: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
         esp_ieee802154_get_panid(), esp_ieee802154_get_channel(),
         extended_address[0], extended_address[1], extended_address[2], extended_address[3],
         extended_address[4], extended_address[5], extended_address[6], extended_address[7]);

    return true;
}

bool_t xIeee802154NetworkInterfaceDisconnect(void)
{
    esp_ieee802154_disable();
    LOGI(TAG_802154, "Disconnected from the IEEE 802.15.4 network");
    return true;
}

void esp_ieee802154_receive_done(uint8_t *buffer, esp_ieee802154_frame_info_t *frame_info)
{
    BaseType_t task;
    if (xQueueSendToBackFromISR(rx_queue, buffer, &task) != pdTRUE)
    {
        ESP_LOGE(TAG_802154, "Failed to enqueue received packet");
    }
}

esp_err_t xIeee802154NetworkInterfaceInput(void *buffer, uint16_t len, void *eb)
{
    NetworkBufferDescriptor_t *pxNetworkBuffer = pxGetNetworkBufferWithDescriptor(len, 0);

    if (pxNetworkBuffer != NULL)
    {
        memcpy(pxNetworkBuffer->pucEthernetBuffer, buffer, len);
        pxNetworkBuffer->xDataLength = len;

        vHandleIEEE802154Frame(pxNetworkBuffer);
		vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
        return ESP_OK;
    }
    else
    {
        LOGE(TAG_802154, "Failed to allocate network buffer");
        return ESP_FAIL;
    }
}

bool_t xIeee802154NetworkInterfaceOutput(NetworkBufferDescriptor_t *const pxNetworkBuffer, bool_t xReleaseAfterSend)
{
    if (pxNetworkBuffer == NULL || pxNetworkBuffer->pucEthernetBuffer == NULL || pxNetworkBuffer->xDataLength == 0)
    {
        LOGE(TAG_802154, "Invalid parameters");
        return false;
    }

    if (xInterfaceState == DOWN)
    {
        LOGI(TAG_802154, "Interface down");
        return false;
    }

    vIeee802154FrameSend(pxNetworkBuffer->pucEthernetBuffer, pxNetworkBuffer->xDataLength);

    LOGI(TAG_802154, "Packet sent successfully");

    if (xReleaseAfterSend == true)
    {
        vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
    }

    return true;
}
