/* Standard includes */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "portability/port.h"

/* FreeRTOS includes */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

/* RINA Components includes */
#include "BufferManagement.h"
#include "ieee802154_NetworkInterface.h"
#include "configRINA.h"
#include "configSensor.h"
#include "common/mac.h"

#include "ieee802154_frame.h"

#include "common/rina_common.h"

/* ESP includes */
#include "esp_ieee802154.h"
#if ESP_IDF_VERSION_MAJOR > 4
#include "esp_mac.h"
#endif
#include "esp_event.h"
#include "esp_system.h"
#include "esp_event_base.h"
#include <esp_log.h>

#define TAG_802154 "ieee802154"
#define SHORT_BROADCAST 0xFFFF
#define PAN_BROADCAST 0xFFFF

static void debug_print_packet(uint8_t *packet, uint8_t packet_length);

/* Function to process received IEEE 802.15.4 frame */
void vHandleIEEE802154Frame(NetworkBufferDescriptor_t *pxNetworkBuffer)
{
    if (pxNetworkBuffer == NULL || pxNetworkBuffer->pucEthernetBuffer == NULL)
    {
        LOGE(TAG_802154, "Invalid network buffer");
        return;
    }

    uint8_t *pucBuffer = pxNetworkBuffer->pucEthernetBuffer;
    uint16_t usLength = pxNetworkBuffer->xDataLength;

    LOGI(TAG_802154, "Received packet of length %d", usLength);
    debug_print_packet(pucBuffer, usLength);

    mac_fcs_t *pxFrameHeader = (mac_fcs_t *)pucBuffer;
    if (pxFrameHeader->frameType == FRAME_TYPE_DATA)
    {
        LOGI(TAG_802154, "Processing DATA frame");
    }
    else if (pxFrameHeader->frameType == FRAME_TYPE_ACK)
    {
        LOGI(TAG_802154, "Received ACK frame");
    }
    else
    {
        LOGW(TAG_802154, "Unknown frame type received: %d", pxFrameHeader->frameType);
    }


}

/* Function to send an IEEE 802.15.4 frame */
void vIeee802154FrameSend(uint8_t *pucBuffer, uint16_t usLength)
{
    if (pucBuffer == NULL || usLength == 0)
    {
        LOGE(TAG_802154, "Invalid buffer for transmission");
        return;
    }

    uint16_t pan_id = ieee802154_PANID_SOURCE;
    esp_ieee802154_set_panid(pan_id);

    ieee802154_address_t srcAddr;
    ieee802154_address_t dstAddr;

    uint8_t eui64[8];
    esp_ieee802154_get_extended_address(eui64);
    srcAddr.mode = ADDR_MODE_LONG;
    memcpy(srcAddr.long_address, eui64, sizeof(eui64));

    dstAddr.mode = ADDR_MODE_SHORT;
    dstAddr.short_address = SHORT_BROADCAST;

    uint8_t buffer[256];
    uint8_t hdrLen = ieee802154_header(&pan_id, &srcAddr, &pan_id, &dstAddr, false, &buffer[1], sizeof(buffer) - 1);

    memcpy(&buffer[1 + hdrLen], pucBuffer, usLength);

    buffer[0] = hdrLen + usLength;

    if (esp_ieee802154_transmit(buffer, false) == ESP_OK)
    {
        LOGI(TAG_802154, "Frame sent successfully");
    }
    else
    {
        LOGE(TAG_802154, "Failed to send frame");
    }
}

/* Helper function to print received IEEE 802.15.4 frame details */
static void debug_print_packet(uint8_t *packet, uint8_t packet_length)
{
    if (packet_length < sizeof(mac_fcs_t))
        return;

    uint8_t position = 0;
    mac_fcs_t *fcs = (mac_fcs_t *)&packet[position];
    position += sizeof(uint16_t);

    ESP_LOGI(TAG_802154, "Frame type:                   %x", fcs->frameType);
    ESP_LOGI(TAG_802154, "Security Enabled:             %s", fcs->secure ? "True" : "False");
    ESP_LOGI(TAG_802154, "Frame pending:                %s", fcs->framePending ? "True" : "False");
    ESP_LOGI(TAG_802154, "Acknowledge request:          %s", fcs->ackReqd ? "True" : "False");
    ESP_LOGI(TAG_802154, "PAN ID Compression:           %s", fcs->panIdCompressed ? "True" : "False");
    ESP_LOGI(TAG_802154, "Reserved:                     %s", fcs->rfu1 ? "True" : "False");
    ESP_LOGI(TAG_802154, "Destination addressing mode:  %x", fcs->destAddrType);
    ESP_LOGI(TAG_802154, "Frame version:                %x", fcs->frameVer);
    ESP_LOGI(TAG_802154, "Source addressing mode:       %x", fcs->srcAddrType);

    switch (fcs->destAddrType)
    {
    case ADDR_MODE_SHORT:
    {
        uint16_t pan_id = *((uint16_t *)&packet[position]);
        position += sizeof(uint16_t);
        uint16_t short_dst_addr = *((uint16_t *)&packet[position]);
        position += sizeof(uint16_t);
        ESP_LOGI(TAG_802154, "On PAN %04x to short address %04x", pan_id, short_dst_addr);
        break;
    }
    case ADDR_MODE_LONG:
    {
        uint16_t pan_id = *((uint16_t *)&packet[position]);
        position += sizeof(uint16_t);
        uint8_t dst_addr[8];
        memcpy(dst_addr, &packet[position], sizeof(dst_addr));
        position += sizeof(dst_addr);
        ESP_LOGI(TAG_802154, "On PAN %04x to long address %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
                 pan_id, dst_addr[0], dst_addr[1], dst_addr[2], dst_addr[3],
                 dst_addr[4], dst_addr[5], dst_addr[6], dst_addr[7]);
        break;
    }
    default:
        ESP_LOGE(TAG_802154, "Unknown destination address type");
        return;
    }

    if (fcs->srcAddrType == ADDR_MODE_LONG)
    {
        uint8_t src_addr[8];
        memcpy(src_addr, &packet[position], sizeof(src_addr));
        position += sizeof(src_addr);
        ESP_LOGI(TAG_802154, "Originating from long address %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
                 src_addr[0], src_addr[1], src_addr[2], src_addr[3],
                 src_addr[4], src_addr[5], src_addr[6], src_addr[7]);
    }
}
