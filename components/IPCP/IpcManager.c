/**
 * @file IPCManager.c
 * @author David Sarabia - i2CAT(you@domain.com)
 * @brief Handler IPCP events.
 * @version 0.1
 * @date 2022-02-23
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <stdio.h>
#include <string.h>

/* FreeRTOS includes. */
#include "Freertos/FreeRTOS.h"
#include "Freertos/task.h"
#include "Freertos/queue.h"
#include "Freertos/semphr.h"

#include "IPCP.h"
#include "common.h"
#include "factoryIPCP.h"
#include "configRINA.h"
#include "IpcManager.h"
#include "normalIPCP.h"
#include "RINA_API.h"
#include "efcpStructures.h"
#include "ipcpIdm.h"
#include "Ribd.h"
#include "du.h"
#include "FlowAllocator.h"
#include "ShimIPCP.h"
#include "EFCP.h"

#include "esp_log.h"

/* Table to store instances created */
static InstanceTableRow_t xInstanceTable[INSTANCES_IPCP_ENTRIES];

/**
 * @brief Initialize a IPC Manager object. Create a Port Id Manager
 * instance. Create an IPCP Id Manager. Finally Initialize the Factories
 * List
 *
 * @param pxIpcManager object created in the IPCP TASK.
 * @return BaseType_t
 */
BaseType_t xIpcManagerInit(ipcManager_t *pxIpcManager)
{

    // factories_t *pxIpcpFactories;
    pidm_t *pxPidm;
    ipcpIdm_t *pxIpcpIdm;

    // pxIpcpFactories = pvPortMalloc(sizeof(*pxIpcpFactories));
    pxPidm = pvPortMalloc(sizeof(*pxPidm));
    pxIpcpIdm = pvPortMalloc(sizeof(*pxIpcpIdm));

    pxPidm = pxPidmCreate();
    pxIpcpIdm = pxIpcpIdmCreate();

    // pxIpcManager->pxFactories = pxIpcpFactories;
    pxIpcManager->pxPidm = pxPidm;
    pxIpcManager->pxIpcpIdm = pxIpcpIdm;

    vListInitialise(&pxIpcManager->xShimInstancesList);

    if (!listLIST_IS_INITIALISED(&pxIpcManager->xShimInstancesList))
    {
        ESP_LOGI(TAG_IPCPMANAGER, "IpcpFactoriesList was not Initialized properly");
        return pdFALSE;
    }

    if (xTest())
    {
        ESP_LOGI(TAG_IPCPMANAGER, "INIT Ribd");
    }

    return pdTRUE;
}

/**
 * @brief Add an IPCP instance into the Ipcp Instance Table.
 *
 * @param pxIpcpInstaceToAdd to added into the table
 */
void vIpcpManagerAddInstanceEntry(ipcpInstance_t *pxIpcpInstaceToAdd)
{

    BaseType_t x = 0;

    for (x = 0; x < INSTANCES_IPCP_ENTRIES; x++)
    {
        if (xInstanceTable[x].xActive == pdFALSE)
        {
            xInstanceTable[x].pxIpcpInstance = pxIpcpInstaceToAdd;
            xInstanceTable[x].pxIpcpType = pxIpcpInstaceToAdd->xType;
            xInstanceTable[x].xIpcpId = pxIpcpInstaceToAdd->xId;
            xInstanceTable[x].xActive = pdTRUE;
            // ESP_LOGE(TAG_IPCPMANAGER, "IPCP INSTANCE Entry successful: %p,id:%d",pxIpcpInstaceToAdd,pxIpcpInstaceToAdd->xId );
            // ESP_LOGE(TAG_IPCPMANAGER, "Instance: %p, id:%d", xInstanceTable[x].pxIpcpInstance,xInstanceTable[x].xIpcpId);

            break;
        }
    }
}

ipcpInstance_t *pxIpcManagerFindInstanceById(ipcpInstanceId_t xIpcpId)
{

    BaseType_t x = 0;

    // ESP_LOGE(TAG_IPCPMANAGER,"Searching for: %d", xIpcpId);

    for (x = 0; x < INSTANCES_IPCP_ENTRIES; x++)

    {
        if (xInstanceTable[x].xActive == pdTRUE)
        {
            // ESP_LOGE(TAG_IPCPMANAGER, "Instance: %p, Id: %d", xInstanceTable[x].pxIpcpInstance, xInstanceTable[x].xIpcpId);
            if (xInstanceTable[x].xIpcpId == xIpcpId)
            {
                ESP_LOGI(TAG_IPCPMANAGER, "Instance founded '%p'", xInstanceTable[x].pxIpcpInstance);
                return xInstanceTable[x].pxIpcpInstance;
                break;
            }
        }
    }
    return NULL;
}

/**
 * @brief Find an ipcp instances into the table by ipcp Type.
 *
 * @param xType Type of instance to be founded.
 * @return ipcpInstance_t* pointer to the ipcp instance.
 */

static ipcpInstance_t *pxIpcManagerFindInstanceByType(ipcpInstanceType_t xType)
{

    BaseType_t x = 0;

    for (x = 0; x < INSTANCES_IPCP_ENTRIES; x++)

    {
        if (xInstanceTable[x].xActive == pdTRUE)
        {
            if (xInstanceTable[x].pxIpcpType == xType)
            {
                // ESP_LOGI(TAG_IPCPMANAGER, "Instance founded '%p'", xInstanceTable [ x ].pxIpcpInstance);
                return xInstanceTable[x].pxIpcpInstance;
                break;
            }
        }
    }
    return NULL;
}

/**
 * @brief create an IPCP instance depending on the config File. Search the correct factory
 * depending on the factory type. Then, request allocate a new IPCP id. Request to the factory
 * create the instance. The IPCP instance created is registered into the IPCP table.
 * @param pxFactories list of factories registered to create the IPCP requiered
 * @param xFactoryType Type of factory requiered to create the IPCP
 * @param pxIpcpIdManager Ipcp Ids Manager to request allocated new IPCP id
 *
 */
BaseType_t xIpcManagerCreateInstance(factories_t *pxFactories, ipcpFactoryType_t xFactoryType, ipcpIdm_t *pxIpcpIdManager)
{
    ipcpFactory_t *pxFactory;
    ipcpInstance_t *pxInstance;
    ipcProcessId_t xIpcpId;

    pxFactory = pvPortMalloc(sizeof(*pxFactory));
    pxInstance = pvPortMalloc(sizeof(*pxInstance));

    /*Find the Factory required depending on the IPCP Type*/
    pxFactory = pxFactoryIPCPFind(pxFactories, xFactoryType);

    if (!pxFactory)
    {
        ESP_LOGE(TAG_IPCPMANAGER, "Cannot find factory '%d'", xFactoryType);
        return 0;
    }

    xIpcpId = xIpcpIdmAllocate(pxIpcpIdManager);
    ESP_LOGD(TAG_IPCPMANAGER, "IPCP id to be ADD: %d", xIpcpId);

    pxInstance = pxFactory->pxFactoryOps->create(pxFactory->pxFactoryData, xIpcpId);

    if (!pxInstance)
    {
        return pdFALSE;
    }
    // ESP_LOGI(TAG_IPCPMANAGER,"Adding IPCP instance into the IMAP Table");
    /* Should send add the instance into the IMAP List*/
    vIpcpManagerAddInstanceEntry(pxInstance);

    return pdTRUE;
}

BaseType_t xIcpManagerNormalRegister(factories_t *pxFactories, ipcpFactoryType_t xFactoryTypeFrom, ipcpFactoryType_t xFactoryTypeTo)
{
    ipcpInstance_t *pxInstanceFrom;
    ipcpInstance_t *pxInstanceTo;

    pxInstanceFrom = pvPortMalloc(sizeof(*pxInstanceFrom));
    pxInstanceTo = pvPortMalloc(sizeof(*pxInstanceTo));

    pxInstanceFrom = pxIpcManagerFindInstanceByType(xFactoryTypeFrom);
    pxInstanceTo = pxIpcManagerFindInstanceByType(xFactoryTypeTo);

    // ESP_LOGI(TAG_IPCPMANAGER, "pxInstanceFrom: %p",pxInstanceFrom);
    // ESP_LOGI(TAG_IPCPMANAGER, "pxInstanceTo: %p",pxInstanceTo);

    // if (xNormalRegistering(pxInstanceFrom, pxInstanceTo))
    //{
    //   return pdTRUE;
    // }

    return pdFALSE;
}

void vIcpManagerEnrollmentFlowRequest(ipcpInstance_t *pxShimInstance, pidm_t *pxPidm, name_t *pxIPCPName)
{

    portId_t xPortId;

    /*This should be proposed by the Flow Allocator?*/
    name_t *destinationInfo = pvPortMalloc(sizeof(*destinationInfo));
    destinationInfo->pcProcessName = REMOTE_ADDRESS_AP_NAME;
    destinationInfo->pcEntityName = "";
    destinationInfo->pcProcessInstance = REMOTE_ADDRESS_AP_INSTANCE;
    destinationInfo->pcEntityInstance = "";

    xPortId = xPidmAllocate(pxPidm);

    if (pxShimInstance->pxOps->flowAllocateRequest == NULL)
    {
        ESP_LOGI(TAG_IPCPNORMAL, "There is not Flow Allocate Request API");
    }
    if (pxShimInstance->pxOps->flowAllocateRequest(xPortId,
                                                   pxIPCPName,
                                                   destinationInfo,
                                                   pxShimInstance->pxData))
    {
        ESP_LOGI(TAG_IPCPNORMAL, "Flow Request processed by the Shim sucessfully");
        return pdTRUE;
    }

    return pdFALSE;
}

BaseType_t xIpcpManagerShimAllocateResponseHandle(factories_t *pxFactories, ipcpFactoryType_t xShimType)
{
    ipcpInstance_t *pxShimInstance;
    ipcpInstance_t *pxNormalInstance;

    pxShimInstance = pvPortMalloc(sizeof(*pxShimInstance));
    pxNormalInstance = pvPortMalloc(sizeof(*pxNormalInstance));

    pxShimInstance = pxIpcManagerFindInstanceByType(xShimType);
    pxNormalInstance = pxIpcManagerFindInstanceByType(eNormal);

    /*Searching in table for the flow - portId, which is waiting for results*/
    /*portId Hardcode for a while*/

    return pdFALSE;
}

/* Handle a Flow allocation request sended by the User throught the RINA API.
 * Return a the Flow xPortID that the RINA API is going to use to send data. */
void vIpcpManagerAppFlowAllocateRequestHandle(pidm_t *pxPidm, struct efcpContainer_t *pxEfcpc, struct ipcpNormalData_t *pxIpcpData)
{

    portId_t xPortId;

    flowAllocateHandle_t *pxFlowAllocateRequest;
    pxFlowAllocateRequest = pvPortMalloc(sizeof(*pxFlowAllocateRequest));

    struct flowSpec_t *pxFlowSpecTmp;
    pxFlowSpecTmp = pvPortMalloc(sizeof(*pxFlowSpecTmp));
    pxFlowAllocateRequest->pxFspec = pxFlowSpecTmp;

    name_t *pxLocal, *pxRemote, *pxDIF;
    pxLocal = pvPortMalloc(sizeof(*pxLocal));
    pxRemote = pvPortMalloc(sizeof(*pxRemote));
    pxDIF = pvPortMalloc(sizeof(*pxDIF));

    pxLocal->pcProcessName = "Test";
    pxLocal->pcProcessInstance = "1";
    pxLocal->pcEntityName = "test";
    pxLocal->pcEntityInstance = "1";
    pxRemote->pcProcessName = "sensor1";
    pxRemote->pcProcessInstance = "";
    pxRemote->pcEntityName = "";
    pxRemote->pcEntityInstance = "";
    pxDIF->pcProcessName = NORMAL_DIF_NAME;
    pxDIF->pcProcessInstance = "";
    pxDIF->pcEntityName = "";
    pxDIF->pcEntityInstance = "";

    pxFlowAllocateRequest->pxDifName = pxDIF;
    pxFlowAllocateRequest->pxLocal = pxLocal;
    pxFlowAllocateRequest->pxRemote = pxRemote;
    pxFlowAllocateRequest->pxFspec->ulAverageBandwidth = 0;
    pxFlowAllocateRequest->pxFspec->ulAverageSduBandwidth = 0;
    pxFlowAllocateRequest->pxFspec->ulDelay = 0;
    pxFlowAllocateRequest->pxFspec->ulJitter = 0;
    pxFlowAllocateRequest->pxFspec->usLoss = 10000;
    pxFlowAllocateRequest->pxFspec->ulMaxAllowableGap = 10;
    pxFlowAllocateRequest->pxFspec->xOrderedDelivery = false;
    pxFlowAllocateRequest->pxFspec->ulUndetectedBitErrorRate = 0;
    pxFlowAllocateRequest->pxFspec->xPartialDelivery = true;
    pxFlowAllocateRequest->pxFspec->xMsgBoundaries = false;

    // dtpConfig_t *pxDtpCfg;
    // struct dtcpConfig_t *pxDtcpCfg;

    /* This shouldbe read from configRINA.h */
    // address_t xSource = 10;
    // address_t xDest = 3;
    // qosId_t xQosId = 1;

    // pxDtcpCfg = pvPortMalloc(sizeof(*pxDtcpCfg));
    // pxDtpCfg = pvPortMalloc(sizeof(*pxDtpCfg));

    xPortId = 33;

    // if (pxNormalInstance->pxOps->flowPrebind(pxNormalInstance->pxData, xPortId))
    //{

    // call to FlowAllocator.
    vFlowAllocatorFlowRequest(pxEfcpc, xPortId, pxFlowAllocateRequest, pxIpcpData);

    //}
    ESP_LOGE(TAG_IPCPMANAGER, "end");
}

BaseType_t xIpcpManagerPreBindFlow(factories_t *pxFactories, ipcpFactoryType_t xNormalType)
{

    ipcpInstance_t *pxNormalInstance = pvPortMalloc(sizeof(*pxNormalInstance));

    pxNormalInstance = pxIpcManagerFindInstanceByType(xNormalType);

    if (pxNormalInstance->pxOps->flowPrebind(pxNormalInstance->pxData, 1))
    {
        return pdTRUE;
    }

    return pdFALSE;
}

BaseType_t xIpcManagerWriteMgmtHandler(ipcpFactoryType_t xType, void *pxData);
BaseType_t xIpcManagerWriteMgmtHandler(ipcpFactoryType_t xType, void *pxData)
{
    struct du_t *pxDu;
    pxDu = (struct du_t *)(pxData);

    ipcpInstance_t *pxShimInstance;
    ipcpInstance_t *pxNormalInstance;

    pxShimInstance = pvPortMalloc(sizeof(*pxShimInstance));
    pxShimInstance = pxIpcManagerFindInstanceByType(xType);

    pxNormalInstance = pvPortMalloc(sizeof(*pxNormalInstance));
    pxNormalInstance = pxIpcManagerFindInstanceByType(eNormal);

    // ESP_LOGE(TAG_IPCPMANAGER,"Calling xNormalMgmtDuWrite");

    pxNormalInstance->pxOps->mgmtDuWrite(pxNormalInstance->pxData, pxDu->pxNetworkBuffer->ulBoundPort, pxDu);

    return pdTRUE;
}

void vIpcManagerRINAPackettHandler(struct ipcpNormalData_t *pxData, NetworkBufferDescriptor_t *pxNetworkBuffer);
void vIpcManagerRINAPackettHandler(struct ipcpNormalData_t *pxData, NetworkBufferDescriptor_t *pxNetworkBuffer)
{
    struct du_t *pxMessagePDU;

    pxMessagePDU = pvPortMalloc(sizeof(*pxMessagePDU));

    if (!pxMessagePDU)
    {
        ESP_LOGE(TAG_IPCPMANAGER, "pxMessagePDU was not allocated");
    }
    pxMessagePDU->pxNetworkBuffer = pxNetworkBuffer;

    ESP_LOGI(TAG_IPCPMANAGER, "The RINA packet is a managment packet");

    if (!xNormalDuEnqueue(pxData, 1, pxMessagePDU))
    {
        ESP_LOGI(TAG_IPCPMANAGER, "Drop frame because there is not enough memory space");
        xDuDestroy(pxMessagePDU);
    }
}

ipcpInstance_t *pxIpcManagerCreateShim(ipcManager_t *pxIpcManager);
ipcpInstance_t *pxIpcManagerCreateShim(ipcManager_t *pxIpcManager)
{

    ipcProcessId_t xIpcpId;

    xIpcpId = xIpcpIdmAllocate(pxIpcManager->pxIpcpIdm);

    // add the shimInstance into the instance list.

    return pxShimWiFiCreate(xIpcpId);
}