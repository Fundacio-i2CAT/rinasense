
#include "freertos/FreeRTOS.h"

#include "common.h"
#include "configRINA.h"
#include "configSensor.h"
#include "cepidm.h"

#include "esp_log.h"
#define BITS_PER_BYTE (8)
#define MAX_PORT_ID (((2 << BITS_PER_BYTE) * sizeof(cepId_t)) - 1)

cepIdm_t *pxCepIdmCreate(void)
{
        cepIdm_t *pxCepIdmInstance;

        pxCepIdmInstance = pvPortMalloc(sizeof(*pxCepIdmInstance));
        if (!pxCepIdmInstance)
                return NULL;

        vListInitialise(&pxCepIdmInstance->xAllocatedCepIds);
        pxCepIdmInstance->xLastAllocated = 0;

        return pxCepIdmInstance;
}

BaseType_t xCepIdmDestroy(cepIdm_t *pxInstance)
{
        allocCepId_t *pxPos, *pxNext;

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

BaseType_t xCepIdmAllocated(cepIdm_t *pxInstance, cepId_t xCepId)
{
        allocCepId_t *pos;
 

        ListItem_t *pxListItem;
        ListItem_t const *pxListEnd;

        pos = pvPortMalloc(sizeof(*pos));

        /* Find a way to iterate in the list and compare the addesss*/

        pxListEnd = listGET_END_MARKER(&pxInstance->xAllocatedCepIds);
        pxListItem = listGET_HEAD_ENTRY(&pxInstance->xAllocatedCepIds);

        while (pxListItem != pxListEnd)
        {

                pos = (allocCepId_t *)listGET_LIST_ITEM_VALUE(pxListItem);

                if (pos)
                {
                        ESP_LOGI(TAG_IPCPMANAGER, "Port ID %p, #: %d", pos, pos->xCepId);
                }
                else
                {
                        return 0;
                }

                pxListItem = listGET_NEXT(pxListItem);
        }

        return 0;

}

cepId_t xCepIdmAllocate(cepIdm_t *pxInstance)
{
        allocCepId_t *pxNewPortId;
        cepId_t pid;

        /* Check if the PxInstances parameter is not null*/
        if (!pxInstance)
        {
                ESP_LOGE(TAG_IPCPMANAGER, "Bogus instance passed, bailing out");
                return cep_id_bad();
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

        while (xCepIdmAllocated(pxInstance, pid))
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
                return cep_id_bad();

        vListInitialiseItem(&pxNewPortId->xCepIdItem);
        pxNewPortId->xCepId = pid;
        vListInsert( &pxInstance->xAllocatedCepIds,&pxNewPortId->xCepIdItem);

        pxInstance->xLastAllocated = pid;

        ESP_LOGI(TAG_IPCPMANAGER, "CEP-id allocation completed successfully (CEP-id = %d)", pid);

        return pid;
}

BaseType_t xCepIdmRelease(cepIdm_t *pxInstance,
                        cepId_t id)
{
        allocCepId_t *pos, *next;
        int found = 0;

        if (!is_cep_id_ok(id))
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
