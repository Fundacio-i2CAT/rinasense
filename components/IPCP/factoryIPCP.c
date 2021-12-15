#include <stdio.h>
#include <string.h>

/* FreeRTOS includes. */
#include "Freertos/FreeRTOS.h"
#include "Freertos/task.h"
#include "Freertos/queue.h"
#include "Freertos/semphr.h"

#include "common.h"
#include "factoryIPCP.h"
#include "IPCP.h"

#include "esp_log.h"



BaseType_t xFactoryIPCPInit(ipcpFactory_t  * pxFactory)
{
    ESP_LOGI(TAG_IPCP, "TEST");
    //configASSERT(pxFactory);
    //List_t xIPCP;


    vListInitialise( &(pxFactory->xIPCPInstancesList));

    //pxFactory->xIPCPInstancesList = xIPCP;

    //memcpy(&pxFactory->xIPCPInstancesList, &xIPCP, sizeof(xIPCP)); 

        
        ESP_LOGI(TAG_IPCP, "IPCP Factory Init");
        return pdTRUE;
}


ipcpInstance_t * xFactoryIPCPFindInstance(ipcpFactory_t  * pxFactory, ipcpInstanceType_t xType)
{
	ESP_LOGI(TAG_IPCP,"Finding Instance Type:%d", xType);
	ipcpInstance_t * pxInstanceIPCP;
	//ipcpInstance_t * pxInstanceIPCP;

	ListItem_t *pxListItem;
	ListItem_t const *pxListEnd;

	pxInstanceIPCP = pvPortMalloc(sizeof(*pxInstanceIPCP));

	/* Find a way to iterate in the list and compare the addesss*/
	pxListEnd = listGET_END_MARKER(& pxFactory->xIPCPInstancesList);
	pxListItem = listGET_HEAD_ENTRY(& pxFactory->xIPCPInstancesList);

	//ESP_LOGI(TAG_IPCP, "ListItemHeadInstance:%p",pxListItem);
	//ESP_LOGI(TAG_IPCP, "ListItemEndInstance:%p",pxListEnd);

	while (pxListItem != pxListEnd)
	{
		
		//ESP_LOGI(TAG_IPCP, "ListItemInstance:%p",pxListItem);
		pxInstanceIPCP = (ipcpInstance_t *)listGET_LIST_ITEM_OWNER(pxListItem);
		//pxFlowNext = (shimFlow_t *)listGET_LIST_ITEM_VALUE(pxListItem);
        if(pxInstanceIPCP->xType == xType)
        {
            ESP_LOGI(TAG_IPCP, "Instance founded %p, Type: %d", pxInstanceIPCP,pxInstanceIPCP->xType);
            return pxInstanceIPCP;
        }

		pxListItem = listGET_NEXT(pxListItem);
	}

    return NULL;
}