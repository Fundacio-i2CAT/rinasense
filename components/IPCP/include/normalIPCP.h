#ifndef NORMALIPCP_H_
#define NORMALIPCP_H_

#include "IpcManager.h"
#include "Rmt.h"
//#include "FlowAllocator.h"

typedef enum eNormal_Flow_State
{
    ePORT_STATE_NULL = 1,
    ePORT_STATE_PENDING,
    ePORT_STATE_ALLOCATED,
    ePORT_STATE_DEALLOCATED,
    ePORT_STATE_DISABLED
} eNormalFlowState_t;

struct ipcpNormalData_t
{
    /* FIXME: add missing needed attributes */

    /* IPCP Instance's Name */
    name_t *pxName;

    /* IPCP Instance's DIF Name */
    name_t *pxDifName;

    /* IPCP Instance's List of Flows created */
    List_t xFlowsList;

    /*  Flow Allocator asociated at the IPCP Instance */
    // flowAllocator_t *pxFa;

    /* Efcp Container asociated at the IPCP Instance */
    struct efcpContainer_t *pxEfcpc;

    /* RMT asociated at the IPCP Instance */
    struct rmt_t *pxRmt;

    /* SDUP asociated at the IPCP Instance */
    // struct sdup *           sdup;

    address_t xAddress;

    // ipcManager_t *pxIpcManager;
};

BaseType_t xNormalFlowBinding(struct ipcpNormalData_t *pxUserData,
                              portId_t xPid,
                              ipcpInstance_t *pxN1Ipcp);

cepId_t xNormalConnectionCreateRequest(struct efcpContainer_t *pxEfcpc,
                                       portId_t xPortId,
                                       address_t xSource,
                                       address_t xDest,
                                       qosId_t xQosId,
                                       dtpConfig_t *pxDtpCfg,
                                       struct dtcpConfig_t *pxDtcpCfg);

BaseType_t xNormalTest(ipcpInstance_t *pxNormalInstance, ipcpInstance_t *pxN1Ipcp);

BaseType_t xNormalRegistering(ipcpInstance_t *pxShimInstance, name_t *pxDifName, name_t *pxName);

BaseType_t xNormalFlowAllocationRequest(struct ipcpInstance_t *pxInstanceFrom, struct ipcpInstance_t *pxInstanceTo, portId_t xShimPortId);

BaseType_t xNormalFlowPrebind(struct ipcpNormalData_t *pxData,
                              portId_t xPortId);

BaseType_t xNormalMgmtDuWrite(struct rmt_t *pxRmt, portId_t xPortId, struct du_t *pxDu);

BaseType_t xNormalDuEnqueue(struct ipcpNormalData_t *pxData,
                            portId_t xN1PortId,
                            struct du_t *pxDu);

BaseType_t xNormalMgmtDuPost(struct ipcpNormalData_t *pxData, portId_t xPortId, struct du_t *pxDu);

BaseType_t xNormalIsFlowAllocated(portId_t xPortId);

BaseType_t xNormalUpdateFlowStatus(portId_t xPortId, eNormalFlowState_t eNewFlowstate);

#endif