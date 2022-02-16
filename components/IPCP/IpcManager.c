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

#include "esp_log.h"

static InstanceTableRow_t xInstanceTable[INSTANCES_IPCP_ENTRIES];

BaseType_t xIpcManagerInit(ipcManager_t *pxIpcManager)
{

    factories_t *pxIpcpFactories;
    pidm_t *pxPidm;
    ipcpIdm_t * pxIpcpIdm;

    pxIpcpFactories = pvPortMalloc(sizeof(*pxIpcpFactories));
    pxPidm = pvPortMalloc(sizeof(*pxPidm));
    pxIpcpIdm = pvPortMalloc(sizeof(*pxIpcpIdm));
    

    pxPidm = pxPidmCreate();
    pxIpcpIdm = pxIpcpIdmCreate();

    pxIpcManager->pxFactories = pxIpcpFactories;
    pxIpcManager->pxPidm = pxPidm;
    pxIpcManager->pxIpcpIdm = pxIpcpIdm;

    vListInitialise(&pxIpcManager->pxFactories->xFactoriesList);

    if (!listLIST_IS_INITIALISED(&pxIpcManager->pxFactories->xFactoriesList))
    {
        ESP_LOGI(TAG_IPCPMANAGER, "IpcpFactoriesList was not Initialized properly");
        return pdFALSE;
    }

    return pdTRUE;
}

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
            //ESP_LOGI(TAG_IPCPMANAGER, "IPCP INSTANCE Entry successful: %p",pxIpcpInstaceToAdd);
            break;
        }
    }
}

/* Create a IPCP Instance*/
BaseType_t xIpcManagerCreateInstance(factories_t *pxFactories, ipcpFactoryType_t xFactoryType, ipcpIdm_t * pxIpcpIdManager)
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
    pxInstance = pxFactory->pxFactoryOps->create(pxFactory->pxFactoryData, xIpcpId);

    if (!pxInstance)
    {
        return pdFALSE;
    }

    /* Should send add the instance into the IMAP List*/
    vIpcpManagerAddInstanceEntry(pxInstance);

    return pdTRUE;
}

ipcpInstance_t *pxIpcManagerFindInstanceByType(ipcpInstanceType_t xType)
{

    BaseType_t x = 0;

    for (x = 0; x < INSTANCES_IPCP_ENTRIES; x++)

    {
        if (xInstanceTable[x].xActive == pdTRUE)
        {
            if (xInstanceTable[x].pxIpcpType == xType)
            {
                //ESP_LOGI(TAG_IPCPMANAGER, "Instance founded '%p'", xInstanceTable [ x ].pxIpcpInstance);
                return xInstanceTable[x].pxIpcpInstance;
                break;
            }
        }
    }
    return NULL;
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

    if (xNormalRegistering(pxInstanceFrom, pxInstanceTo))
    {
        return pdTRUE;
    }

    return pdFALSE;
}

BaseType_t xIcpManagerEnrollmentFlowRequest(factories_t *pxFactories, ipcpFactoryType_t xFactoryTypeFrom, ipcpFactoryType_t xFactoryTypeTo, pidm_t *pxPidm)
{
    ipcpInstance_t *pxInstanceFrom;
    ipcpInstance_t *pxInstanceTo;
    portId_t xPortID;

    pxInstanceFrom = pvPortMalloc(sizeof(*pxInstanceFrom));
    pxInstanceTo = pvPortMalloc(sizeof(*pxInstanceTo));

    pxInstanceFrom = pxIpcManagerFindInstanceByType(xFactoryTypeFrom);
    pxInstanceTo = pxIpcManagerFindInstanceByType(xFactoryTypeTo);

    xPortID = xPidmAllocate(pxPidm);

    if (xNormalFlowAllocationRequest(pxInstanceFrom, pxInstanceTo, xPortID))
    {
        return pdTRUE;
    }

    return pdFALSE;
}

BaseType_t xIpcpManagerShimAllocateResponseHandle(factories_t *pxFactories, ipcpFactoryType_t xShimType)
{
    ipcpInstance_t *pxShimInstance;

    pxShimInstance = pvPortMalloc(sizeof(*pxShimInstance));

    pxShimInstance = pxIpcManagerFindInstanceByType(xShimType);

    if (pxShimInstance->pxOps->flowAllocateResponse(pxShimInstance))
    {
        return pdTRUE;
    }

    return pdFALSE;
}

/* Handle a Flow allocation request sended by the User throught the RINA API.
* Return a the Flow xPortID that the RINA API is going to use to send data. */
portId_t xIpcpManagerAppFlowAllocateRequestHandle(pidm_t *pxPidm, void *data)
{

    ipcpInstance_t *pxNormalInstance;
    portId_t xPortId;

    dtpConfig_t *pxDtpCfg;
    struct dtcpConfig_t *pxDtcpCfg;

    /* This shouldbe read from configRINA.h */
    address_t xSource = 10;
    address_t xDest = 3;
    qosId_t xQosId = 1;

    pxNormalInstance = pvPortMalloc(sizeof(*pxNormalInstance));
    pxDtcpCfg = pvPortMalloc(sizeof(*pxDtcpCfg));
    pxDtpCfg = pvPortMalloc(sizeof(*pxDtpCfg));

    pxNormalInstance = pxIpcManagerFindInstanceByType(eNormal);

    xPortId = xPidmAllocate(pxPidm);

    if (pxNormalInstance->pxOps->flowPrebind(pxNormalInstance->pxData, xPortId))
    {

        if (pxNormalInstance->pxOps->connectionCreate(pxNormalInstance->pxData, xPortId,
                                                      xSource, xDest, xQosId, pxDtpCfg,
                                                      pxDtcpCfg))
        {
            return pdTRUE;
        }
    }

    return pdFALSE;
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

BaseType_t xIpcManagerSendMgmtHandler(void *pxData);
BaseType_t xIpcManagerSendMgmtHandler(void *pxData)
{
    struct du_t * pxDu;
    pxDu = (struct du_t *) (pxData);

    return pdTRUE;
    
}