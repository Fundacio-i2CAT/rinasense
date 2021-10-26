
/*TODO:
 * - Write EFCP
 * - Write EFCP container
 * - Receive EFCP
 * - Receive EFCP Container*/

#include <stdio.h>

/* FreeRTOS includes. */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "EFCP.h"
#include "RMT.h"
#include "pci.h"
#include "du.h"
#include "efcpStructures.h"
#include "dtp.h"

#include "esp_log.h"

#define TAG_EFCP "EFCP"


struct efcp_t * pxEfcpImapFind(efcpImap_t * pxMap, cepId_t xCepIdKey)
{
        efcpImapRow_t * pxEntry;
        BaseType_t x = 0;

        configASSERT(pxMap);

        for( x = 0; x < EFCP_IMAP_ENTRIES; x++ ) //lookup in the MAP for the EFCP Instance based on cepIdKey
	{
		//Comparison between Cache MACAddress and the MacAddress looking up for.
		if( pxMap->xEfcpImapTable[x].xCepIdKey == xCepIdKey)
		{
                        pxEntry->xCepIdKey = pxMap->xEfcpImapTable[x].xCepIdKey;
                        pxEntry->xEfcpValue = pxMap->xEfcpImapTable[x].xEfcpValue;
			ESP_LOGD(TAG_EFCP, "EFCP Instance founded");
        
			break;
		}
        }
        return pxEntry->xEfcpValue;
}


BaseType_t xEfcpContainerReceive(efcpContainer_t * pxEfcpContainer, cepId_t  xCepId, struct du_t * pxDu)
{

        efcp_t *           pxEfcp;
        BaseType_t         ret = pdTRUE;
        pduType_t          xPduType;
        
        if (!is_cep_id_ok(xCepId)) {
                ESP_LOGE(TAG_EFCP, "Bad cep-id, cannot write into container");
                xDuDestroy(pxDu);
                return pdFALSE;
        }


        pxEfcp = pxEfcpImapFind(pxEfcpContainer->xEfcpInstances, xCepId);
        if (!pxEfcp) {
                //spin_unlock_bh(&container->lock);
                ESP_LOGE(TAG_EFCP,"Cannot find the requested instance cep-id: %d",
                        xCepId);
                xDuDestroy(pxDu);
                return pdFALSE;
        }
        if (pxEfcp->xState == eEfcpDeallocated) {
                //spin_unlock_bh(&container->lock);
                xDuDestroy(pxDu);
                ESP_LOGI(TAG_EFCP,"EFCP already deallocated");
                return pdTRUE;
        }
      
        //atomic_inc(&efcp->pending_ops);
       

        xPduType = pxDu->pxPci->xType; // Check this
        if (xPduType == PDU_TYPE_DT &&
        		pxEfcp->pxConnection->xDestinationCepId < 0) 
        {
        	/* Check that the destination cep-id is set to avoid races,
        	 * otherwise set it*/
        	pxEfcp->pxConnection->xDestinationCepId = pxDu->pxPci->connectionId_t.xDestination;
        }
        
        //spin_unlock_bh(&container->lock);

        ret = xEfcpReceive(pxEfcp, pxDu);
#if 0
        //spin_lock_bh(&container->lock);
        if (atomic_dec_and_test(&efcp->pending_ops) &&
        		efcp->state == EFCP_DEALLOCATED) {
                spin_unlock_bh(&container->lock);
		wake_up_interruptible(&container->del_wq);
                return ret;
        }
        spin_unlock_bh(&container->lock);

        return ret;
#endif
        return ret;
}



BaseType_t xEfcpReceive(efcp_t * pxEfcp,  struct du_t *  pxDu)
{
        pduType_t        xPduType;
            
    	

        xPduType = pxDu->pxPci->xType;

        /* Verify the type of PDU*/
        if (pdu_type_is_control(xPduType))
        {
                if (!pxEfcp->pxDtp || !pxEfcp->pxDtp->pxDtcp)
                {
                        ESP_LOGE( TAG_EFCP, "No DTCP instance available");
                        xDuDestroy(pxDu);
                        return pdFALSE;
                }

               /* if (dtcp_common_rcv_control(pxEfcp->pxDtp.>pxDtcp, pxDu))
                        return pdFALSE;*/

                return pdTRUE;
        }

        if (xDtpReceive(pxEfcp->pxDtp, pxDu)) {
                ESP_LOGE(TAG_EFCP, "DTP cannot receive this PDU");
                return pdFALSE;
        }

        return pdTRUE;
}

