
#include "freertos/FreeRTOS.h"

#include "common.h"
#include "configRINA.h"
#include "configSensor.h"
#include "pidm.h"

#include "esp_log.h"
#define BITS_PER_BYTE (8)
#define MAX_PORT_ID (((2 << BITS_PER_BYTE) * sizeof(portId_t)) - 1)

pidm_t *pxPidmCreate(void)
{
        pidm_t *pxPdimInstance;

        pxPdimInstance = pvPortMalloc(sizeof(*pxPdimInstance));
        if (!pxPdimInstance)
                return NULL;

        vListInitialise(&pxPdimInstance->xAllocatedPorts);
        pxPdimInstance->xLastAllocated = 0;

        return pxPdimInstance;
}

BaseType_t xPidmDestroy(pidm_t *pxInstance)
{
        allocPid_t *pxPos, *pxNext;

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

BaseType_t xPidmAllocated(pidm_t *pxInstance, portId_t xPortId)
{
        allocPid_t *pos;
 

        ListItem_t *pxListItem;
        ListItem_t const *pxListEnd;

        pos = pvPortMalloc(sizeof(*pos));

        /* Find a way to iterate in the list and compare the addesss*/

        pxListEnd = listGET_END_MARKER(&pxInstance->xAllocatedPorts);
        pxListItem = listGET_HEAD_ENTRY(&pxInstance->xAllocatedPorts);

        while (pxListItem != pxListEnd)
        {

                pos = (allocPid_t *)listGET_LIST_ITEM_VALUE(pxListItem);

                if (pos)
                {
                        ESP_LOGI(TAG_IPCPMANAGER, "Port ID %p, #: %d", pos, pos->xPid);

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

portId_t xPidmAllocate(pidm_t *pxInstance)
{
        allocPid_t *pxNewPortId;
        portId_t pid;

        /* Check if the PxInstances parameter is not null*/
        if (!pxInstance)
        {
                ESP_LOGE(TAG_IPCPMANAGER, "Bogus instance passed, bailing out");
                return port_id_bad();
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

        while (xPidmAllocated(pxInstance, pid))
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
                return port_id_bad();

        vListInitialiseItem(&pxNewPortId->xporIdItem);
        pxNewPortId->xPid = pid;
        vListInsert( &pxInstance->xAllocatedPorts,&pxNewPortId->xporIdItem);

        pxInstance->xLastAllocated = pid;

        ESP_LOGI(TAG_IPCPMANAGER, "Port-id allocation completed successfully (id = %d)", pid);

        return pid;
}

BaseType_t xPidmRelease(pidm_t *pxInstance,
                        portId_t id)
{
        allocPid_t *pos, *next;
        int found = 0;

        if (!is_port_id_ok(id))
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
