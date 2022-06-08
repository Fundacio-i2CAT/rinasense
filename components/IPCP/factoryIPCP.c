#include <stdio.h>
#include <string.h>

/* FreeRTOS includes. */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "common.h"
#include "factoryIPCP.h"
#include "IPCP.h"

#include "esp_log.h"

#if 0
BaseType_t xFactoryIPCPInit(ipcpFactory_t  * pxFactory)
{
    ESP_LOGI(TAG_IPCPFACTORY, "TEST");
    //configASSERT(pxFactory);
    //List_t xIPCP;


    vListInitialise( &(pxFactory->xIPCPInstancesList));

    //pxFactory->xIPCPInstancesList = xIPCP;

    //memcpy(&pxFactory->xIPCPInstancesList, &xIPCP, sizeof(xIPCP)); 

        
        ESP_LOGI(TAG_IPCPFACTORY, "IPCP Factory Init");
        return pdTRUE;
}
#endif

ipcpFactory_t *pxFactoryIPCPFind(factories_t *pxFactoriesList, ipcpFactoryType_t xType)
{

        ipcpFactory_t *pxFactoryIPCP;

        ListItem_t *pxListItem;
        ListItem_t const *pxListEnd;

        pxFactoryIPCP = pvPortMalloc(sizeof(*pxFactoryIPCP));

        /* Find a way to iterate in the list and compare the addesss*/

        pxListEnd = listGET_END_MARKER(&pxFactoriesList->xFactoriesList);
        pxListItem = listGET_HEAD_ENTRY(&pxFactoriesList->xFactoriesList);

        while (pxListItem != pxListEnd)
        {

                pxFactoryIPCP = (ipcpFactory_t *)listGET_LIST_ITEM_OWNER(pxListItem);

                if (pxFactoryIPCP)
                {

                        if (pxFactoryIPCP->xFactoryType == xType)
                        {
                                //ESP_LOGI(TAG_IPCPFACTORY, "Factory founded %p, Type: %d", pxFactoryIPCP,pxFactoryIPCP->xFactoryType);
                                return pxFactoryIPCP;
                        }
                }
                else
                {
                        return NULL;
                }

                pxListItem = listGET_NEXT(pxListItem);
        }

        return NULL;
}

BaseType_t xFactoryIPCPRegister(factories_t *pxFactoriesList,
                                ipcpFactoryType_t xType,
                                struct ipcpFactoryData_t *pxFactoryData,
                                ipcpFactoryOps_t *pxFactoryOps);

BaseType_t xFactoryIPCPRegister(factories_t *pxFactoriesList,
                                ipcpFactoryType_t xType,
                                struct ipcpFactoryData_t *pxFactoryData,
                                ipcpFactoryOps_t *pxFactoryOps)
{
        ipcpFactory_t *pxFactory;

        //ESP_LOGI(TAG_IPCPFACTORY, "Registering new factory");

        /*TODO: CHeck Name, Operations and List Factory*/

#if 0

        if (!string_is_ok(name)) {
                ESP_LOGI(TAG_IPCPFACTORY,"Name is bogus, cannot register factory");
                return pdFALSE;
        }

        if (!ops_are_ok(ops)) {
               ESP_LOGI(TAG_IPCPFACTORY,"Cannot register factory '%s', ops are bogus", name);
                 return pdFALSE;
        }

        if (!factories) {
                ESP_LOGI(TAG_IPCPFACTORY,"Bogus parent, cannot register factory '%s", name);
                return pdFALSE;
        }
#endif

        pxFactory = pxFactoryIPCPFind(pxFactoriesList, xType);
        if (pxFactory)
        {
                ESP_LOGE(TAG_IPCPFACTORY, "Factory '%d' already registered", xType);
                return pdFALSE;
        }

        //ESP_LOGI(TAG_IPCPFACTORY, "Registering factory '%d'", xType);

        pxFactory = pvPortMalloc(sizeof(*pxFactory));
        if (!pxFactory)
        {
                return pdFALSE;
        }

        pxFactory->xFactoryType = xType;
        pxFactory->pxFactoryData = pxFactoryData;
        pxFactory->pxFactoryOps = pxFactoryOps;

        /*TODO: Check Init operations for factory.*/

        if (!pxFactory->pxFactoryOps->init(pxFactory->pxFactoryData))
        {
                ESP_LOGE(TAG_IPCPFACTORY, "Cannot initialize factory '%d'", xType);

                vPortFree(pxFactory);
                return pdFALSE;
        }

        /*Initialialise xFactory item and add to the xFactoriesList*/
        vListInitialiseItem(&(pxFactory->xIPCPFactoryItem));
        listSET_LIST_ITEM_OWNER(&(pxFactory->xIPCPFactoryItem), (void *)pxFactory);
        vListInsert(&(pxFactoriesList->xFactoriesList), &(pxFactory->xIPCPFactoryItem));

        /* Double checking for bugs */
        //ESP_LOGI(TAG_IPCPFACTORY, "Factory registered successfully");

        return pdTRUE;
}

#if 0
BaseType_t xFactoryIPCPUnregister(struct ipcp_factories * factories,
                     struct ipcp_factory *   factory)
{
        struct ipcp_factory * tmp;
        const char *          name;

        if (!factories) {
                LOG_ERR("Bogus parent, cannot unregister factory");
                return -1;
        }

        if (!factory) {
                LOG_ERR("Bogus factory, cannot unregister");
                return -1;
        }

        name = robject_name(&factory->robj);
        ASSERT(string_is_ok(name));

        LOG_DBG("Unregistering factory '%s'", name);

        ASSERT(factories);

        tmp = ipcpf_find(factories, name);
        if (!tmp) {
                LOG_ERR("Cannot find factory '%s'", name);
                return -1;
        }

        ASSERT(tmp == factory);

        if (tmp->ops->fini(factory->data)) {
                LOG_ERR("Cannot finalize factory '%s'", name);
        }

        robject_del(&factory->robj);
	/* kobject_put is not needed since the struct factory is the one to be
	 * freed
        robject_put(&factory->robj);
	*/

        spin_lock(&factories->lock);
        if (!list_empty(&factory->list)) {
                list_del(&factory->list);
        }
        spin_unlock(&factories->lock);

        rkfree(factory->name);
        rkfree(factory);

        LOG_INFO("Factory unregistered successfully");

        return 0;
}
#endif
