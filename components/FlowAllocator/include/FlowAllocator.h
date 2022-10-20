/*
 * FlowAllocator.h
 *
 *  Created on: 24 Mar. 2022
 *      Author: i2CAT
 */

#ifndef FLOW_ALLOCATOR_H_INCLUDED
#define FLOW_ALLOCATOR_H_INCLUDED

#include "configSensor.h"
#include "common/rina_ids.h"

#include "RINA_API_flows.h"

#include "efcpStructures.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    eFA_EMPTY,
    eFA_ALLOCATION_IN_PROGRESS,
    eFA_ALLOCATED,
    eFA_WAITING_2_MPL_BEFORE_TEARING_DOWN,
    eFA_DEALLOCATED,

} eFlowAllocationState_t;

typedef enum
{
    eFAI_NONE,
    eFAI_PENDING,
    eFAI_ALLOCATED,

} eFaiState_t;

typedef struct xFLOW_ALLOCATOR_INSTANCE
{
    /* Flow allocator Instance Item */
    RsListItem_t xInstanceItem;

    /* PortId associated to this flow Allocator Instance*/
    portId_t xPortId;

    /* State */
    eFaiState_t eFaiState;

    /* Flow Allocator Request*/
    flowAllocateHandle_t *pxFlowAllocatorHandle;

} flowAllocatorInstance_t;

struct FlowRequestRow
{
    flowAllocatorInstance_t *pxFAI;

    bool_t xValid;

};

typedef struct xFLOW_ALLOCATOR
{
    /* List of FAI*/
    RsList_t xFlowAllocatorInstances;

    /* Reference to the normal IPCP owning this flow allocator. */
    struct ipcpInstance_t *pxNormalIpcp;

    struct FlowRequestRow xFlowRequestTable[FLOWS_REQUEST];

    pthread_mutex_t xMux;

} flowAllocator_t;

typedef struct xQOS_SPEC
{
    /* The name of the QoS cube, if known */
    string_t pcQosName;

    /* The id of the QoS cube, if known (-1 = not known) */
    qosId_t xQosId;

    /* Defines the characteristics of a flow */
    struct flowSpec_t *pxFlowSpec;

} qosSpec_t;

/* Contains the information to setup a new flow */
typedef struct xFLOW_MESSAGE
{
    /* The naming information of the source application process */
    name_t *pxSourceInfo;

    /* The naming information of the destination application process */
    name_t *pxDestInfo;

    /* The port id allocated to this flow by the source IPC process */

    /* While the search rules that generate the forwarding table should allow for a natural termination condition, it seems wise to have the means to enforce termination */
    uint32_t ulHopCount;

    /* Flow allocation enum State*/
    eFlowAllocationState_t eState;

    /* Maximum number of retries to create the flow before giving up */
    uint32_t ulMaxCreateFlowRetries;

    /* Source address */
    address_t xSourceAddress;

    /* Remote address to connect*/
    address_t xRemoteAddress;

    /* The identifiers of all the connections that can be used to support this flow during its lifetime */
    connectionId_t *pxConnectionId;

    uint32_t ulCurrentConnectionId;

    /* the QoS parameters specified by the application process that requested this flow */
    qosSpec_t *pxQosSpec;

    /* the configuration for the policies and parameters of this connection's DTP */
    dtpConfig_t *pxDtpConfig;

    /* the configuration for the policies and parameters of this connection's DTCP */
    struct dtcpConfig_t *pxDtcpConfig;

    /* Source Port Id */
    portId_t xSourcePortId;

    portId_t xDestinationPortId;

} flow_t;

#define IPCP_DATA_FROM_FLOW_ALLOCATOR(x) \
    container_of(x, struct ipcpInstanceData_t, xFA)

#ifdef __cplusplus
}
#endif

#endif
