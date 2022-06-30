/*
 * RINA_API.h
 *
 *  Created on: 12 jan. 2022
 *      Author: i2CAT
 */

#ifndef RINA_API_H_INCLUDED
#define RINA_API_H_INCLUDED

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

struct appRegistration_t
{
    string_t pcNameDIF;
    string_t pcApplicationName;
    List_t xFlowsList;
};

struct rinaFlowSpec_t
{
    uint32_t version; /* version number to allow for extensions */
#define RINA_FLOW_SPEC_VERSION 1
    uint64_t max_sdu_gap;   /* in SDUs */
    uint64_t avg_bandwidth; /* in bits per second */
    uint32_t max_delay;     /* in microseconds */
#define RINA_FLOW_SPEC_LOSS_MAX 10000
    uint16_t max_loss;         /* from 0 (0%) to 10000 (100%) */
    uint32_t max_jitter;       /* in microseconds */
    uint8_t in_order_delivery; /* boolean */
    uint8_t msg_boundaries;    /* boolean */
};

typedef struct xFLOW_ALLOCATE_HANDLE
{

    EventBits_t xEventBits;         /*Keep Tract of events*/
    EventGroupHandle_t xEventGroup; /*Event Group for this flow allocate request*/
    TickType_t xReceiveBlockTime;   /**< if recv[to] is called while no data is available, wait this amount of time. Unit in clock-ticks */
    TickType_t xSendBlockTime;
    uint16_t usTimeout; /**< Time (in ticks) after which this socket needs attention */

    portId_t xPortId; /*Should be change by the TASK*/
    name_t *pxLocal;
    name_t *pxRemote;
    name_t *pxDifName;
    struct flowSpec_t *pxFspec;

} flowAllocateHandle_t;

typedef struct xREGISTER_APPLICATION_HANDLE
{
    uint32_t xSrcPort;
    uint32_t xDestPort;
    ipcProcessId_t xSrcIpcpId;
    ipcProcessId_t xDestIpcpId;
    name_t *pxAppName;
    name_t *pxDafName;
    name_t *pxDifName;

} registerApplicationHandle_t;

struct appRegistration_t *RINA_application_register(string_t pcNameDif,
                                                    string_t pcLocalApp,
                                                    uint8_t Flags);

BaseType_t RINA_application_unregister(struct appRegistration_t *xAppRegistration);

portId_t RINA_flow_accept(struct appRegistration_t *xAppRegistration,
                          string_t pcRemoteApp,
                          struct rinaFlowSpec_t *xFlowSpec,
                          uint8_t Flags);

portId_t RINA_flow_alloc(string_t pcNameDIF,
                         string_t pcLocalApp,
                         string_t pcRemoteApp,
                         struct rinaFlowSpec_t *xFlowSpec,
                         uint8_t Flags);

BaseType_t RINA_flow_read(portId_t xPortId,
                          void *pvBuffer,
                          size_t uxBufferLength);
BaseType_t RINA_flow_write(portId_t xPortId,
                           void *pvBuffer,
                           size_t uxTotalDataLength);
BaseType_t RINA_flow_close(portId_t xPortId);

void vRINA_WeakUpUser(flowAllocateHandle_t *pxFlowAllocateResponse);

#endif /* RINA_API_H_ */
