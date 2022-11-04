/*
 * RINA_API.h
 *
 *  Created on: 12 jan. 2022
 *      Author: i2CAT
 */

#ifndef RINA_API_H_INCLUDED
#define RINA_API_H_INCLUDED

#include "ARP826.h"
#include "RINA_API_flows.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appRegistration_t
{
    string_t pcNameDIF;
    string_t pcApplicationName;
    RsList_t xFlowsList;
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

typedef struct xREGISTER_APPLICATION_HANDLE
{
    uint32_t xSrcPort;
    uint32_t xDestPort;
    ipcProcessId_t xSrcIpcpId;
    ipcProcessId_t xDestIpcpId;
    rname_t xAppName;
    rname_t xDafName;
    rname_t xDifName;

} registerApplicationHandle_t;

struct appRegistration_t *RINA_application_register(string_t pcNameDif,
                                                    string_t pcLocalApp,
                                                    uint8_t Flags);

bool_t RINA_application_unregister(struct appRegistration_t *xAppRegistration);

portId_t RINA_flow_accept(struct appRegistration_t *xAppRegistration,
                          string_t pcRemoteApp,
                          struct rinaFlowSpec_t *xFlowSpec,
                          uint8_t Flags);

portId_t RINA_flow_alloc(string_t pcNameDIF,
                         string_t pcLocalApp,
                         string_t pcRemoteApp,
                         struct rinaFlowSpec_t *xFlowSpec,
                         uint8_t Flags);

int32_t RINA_flow_read(portId_t xPortId,
                       void *pvBuffer,
                       size_t uxBufferLength);

size_t RINA_flow_write(portId_t xPortId,
                       void *pvBuffer,
                       size_t uxTotalDataLength);

bool_t RINA_flow_close(portId_t xPortId);

//void vRINA_WeakUpUser(flowAllocateHandle_t *pxFlowAllocateResponse);

void vRINA_WakeUpFlowRequest(flowAllocateHandle_t *pxFlowAllocateResponse, int nNewBits);

void RINA_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* RINA_API_H_ */
