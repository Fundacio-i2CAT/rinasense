
#include "freertos/FreeRTOS.h"

#include "common.h"
#include "configRINA.h"
#include "configSensor.h"
#include "ipcpIdm.h"

#include "esp_log.h"
#define BITS_PER_BYTE (8)
#define MAX_PORT_ID (((2 << BITS_PER_BYTE) * sizeof(ipcProcessId_t)) - 1)

ipcpIdm_t *pxIpcpIdmCreate(void)
{
        ipcpIdm_t *pxIpcpIdmInstance;

        pxIpcpIdmInstance = pvPortMalloc(sizeof(*pxIpcpIdmInstance));
        if (!pxIpcpIdmInstance)
                return NULL;

        vListInitialise(&pxIpcpIdmInstance->xAllocatedIpcpIds);
        pxIpcpIdmInstance->xLastAllocated = 0;

        return pxIpcpIdmInstance;
}

BaseType_t xIpcpIdmDestroy(ipcpIdm_t *pxInstance)
{
        allocIpcpId_t *pxPos, *pxNext;

        if (!pxInstance)
        {
                ESP_LOGE(TAG_IPCPMANAGER, "Bogus instance passed, bailing out");
                return pdFALSE;
        }

        //  list_for_each_entry_safe(pos, next, &pxInstance->xAllocatedPorts, list) {
        //vPortFree(pxPos);
        //}

        vPortFree(pxInstance);

        return pdTRUE;
}

BaseType_t xIpcpIdmAllocated(ipcpIdm_t *pxInstance, ipcProcessId_t xIpcpId)
{
        allocIpcpId_t *pos;
 

        ListItem_t *pxListItem;
        ListItem_t const *pxListEnd;

        pos = pvPortMalloc(sizeof(*pos));

        /* Find a way to iterate in the list and compare the addesss*/

        pxListEnd = listGET_END_MARKER(&pxInstance->xAllocatedIpcpIds);
        pxListItem = listGET_HEAD_ENTRY(&pxInstance->xAllocatedIpcpIds);

        while (pxListItem != pxListEnd)
        {

                pos = (allocIpcpId_t *)listGET_LIST_ITEM_VALUE(pxListItem);

                if (pos)
                {
                        ESP_LOGI(TAG_IPCPMANAGER, "Port ID %p, #: %d", pos, pos->xIpcpId);

                        /*if (pxInstance->xType == xType)
                        {
                                //ESP_LOGI(TAG_IPCPMANAGER, "Instance founded %p, Type: %d", pxInstance, pxInstance->xType);
                                return pxInstance;
                        }*/
                }
                else
                {
                        return 0;
                }

                pxListItem = listGET_NEXT(pxListItem);
        }

        return 0;

}

ipcProcessId_t xIpcpIdmAllocate(ipcpIdm_t *pxInstance)
{
        allocIpcpId_t *pxNewPortId;
        ipcProcessId_t pid;

        /* Check if the PxInstances parameter is not null*/
        if (!pxInstance)
        {
                ESP_LOGE(TAG_IPCPMANAGER, "Bogus instance passed, bailing out");
                return -1;
        }

        /* If the last ipId allocated is same as the max port id then start it again*/
        if (pxInstance->xLastAllocated == MAX_PORT_ID)
        {
                pid = 1;
        }
        else
        {
                pid = pxInstance->xLastAllocated + 1;
        }

        while (xIpcpIdmAllocated(pxInstance, pid))
        {
                if (pid == MAX_PORT_ID)
                {
                        pid = 1;
                }
                else
                {
                        pid++;
                }
        }

        pxNewPortId = pvPortMalloc(sizeof(*pxNewPortId));
        if (!pxNewPortId)
                return -1;

        vListInitialiseItem(&pxNewPortId->xIpcpIdItem);
        pxNewPortId->xIpcpId = pid;
        vListInsert( &pxInstance->xAllocatedIpcpIds,&pxNewPortId->xIpcpIdItem);

        pxInstance->xLastAllocated = pid;

        ESP_LOGI(TAG_IPCPMANAGER, "Port-id allocation completed successfully (id = %d)", pid);

        return pid;
}

BaseType_t xIpcpIdmRelease(ipcpIdm_t *pxInstance,
                        ipcProcessId_t id)
{
        allocIpcpId_t *pos, *next;
        int found = 0;

        if (!is_ipcp_id_ok(id))
        {
                ESP_LOGE(TAG_IPCPMANAGER, "Bad flow-id passed, bailing out");
                return pdFALSE;
        }
        if (!pxInstance)
        {
                ESP_LOGE(TAG_IPCPMANAGER, "Bogus instance passed, bailing out");
                return pdFALSE;
        }

        /*list_for_each_entry_safe(pos, next, &instance->allocated_ports, list) {
                if (pos->pid == id) {
                	list_del(&pos->list);
                	rkfree(pos);
                        found = 1;
                }
        }*/

        if (!found)
        {
                ESP_LOGE(TAG_IPCPMANAGER, "Didn't find port-id %d, returning error", id);
        }
        else
        {
                ESP_LOGI(TAG_IPCPMANAGER, "Port-id release completed successfully (port_id: %d)", id);
        }

        return pdTRUE;
}
