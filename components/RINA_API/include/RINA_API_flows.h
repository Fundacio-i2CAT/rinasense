#ifndef _COMPONENTS_RINA_API_FLOWS_H
#define _COMPONENTS_RINA_API_FLOWS_H

#include "rina_name.h"
#include "rina_ids.h"

typedef struct xFLOW_ALLOCATE_HANDLE{
    /*uint32_t xSrcPort;
    uint32_t xDestPort;
    ipcProcessId_t xSrcIpcpId;
    ipcProcessId_t xDestIpcpId;*/

#ifdef ESP_PLATFORM
    EventBits_t xEventBits;  /*Keep Tract of events*/
    EventGroupHandle_t xEventGroup; /*Event Group for this flow allocate request*/
    TickType_t xReceiveBlockTime;          /**< if recv[to] is called while no data is available, wait this amount of time. Unit in clock-ticks */
    TickType_t xSendBlockTime;
#endif

    portId_t    xPortId;    /*Should be change by the TASK*/
    name_t      *pxLocal;
    name_t      *pxRemote;
    name_t      *pxDifName;
    struct flowSpec_t *pxFspec;

} flowAllocateHandle_t;

#endif // _COMPONENTS_RINA_API_FLOWS_H
