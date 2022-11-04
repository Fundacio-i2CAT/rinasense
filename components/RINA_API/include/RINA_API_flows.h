#ifndef _COMPONENTS_RINA_API_FLOWS_H
#define _COMPONENTS_RINA_API_FLOWS_H

#include "common/list.h"
#include "common/rina_name.h"
#include "common/rina_ids.h"
#include "common/list.h"

#include "rina_common_port.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xFLOW_ALLOCATE_HANDLE
{
    /*uint32_t xSrcPort;
    uint32_t xDestPort;
    ipcProcessId_t xSrcIpcpId;
    ipcProcessId_t xDestIpcpId;*/

#ifdef ESP_PLATFORM
    EventBits_t xEventBits;         /*Keep Tract of events*/
    EventGroupHandle_t xEventGroup; /*Event Group for this flow allocate request*/
#endif

    useconds_t xSendBlockTime;

    useconds_t xReceiveBlockTime;   /**< if recv[to] is called while no data is available, wait this amount of time. Unit in microseconds */
    useconds_t usTimeout; /**< Time (in microseconds) after which this socket needs attention */

    portId_t xPortId; /*Should be change by the TASK*/

    rname_t xLocal;
    rname_t xRemote;
    rname_t xDifName;

    flowSpec_t xFspec;

    RsList_t xListWaitingPackets;

    /* Condition variable. */
    pthread_cond_t xEventCond;

    /* */
    long nEventBits;

    /* Mutex for nEventBits. */
    pthread_mutex_t xEventMutex;

} flowAllocateHandle_t;

#ifdef __cplusplus
}
#endif

#endif // _COMPONENTS_RINA_API_FLOWS_H
