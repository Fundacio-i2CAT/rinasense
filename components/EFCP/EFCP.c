
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



BaseType_t xEfcpContainerReceive(efcpContainer_t * pxContainer,
                           cepId_t                xCepId,
                            du_t *             pxDu)
{

        efcp_t *           pxEfcp;
        int                ret = 0;
        pduType_t          pxPduType;
#if 0
        if (!is_cep_id_ok(cep_id)) {
                LOG_ERR("Bad cep-id, cannot write into container");
                du_destroy(du);
                return -1;
        }

        spin_lock_bh(&container->lock);
        efcp = efcp_imap_find(container->instances, cep_id);
        if (!efcp) {
                spin_unlock_bh(&container->lock);
                LOG_ERR("Cannot find the requested instance cep-id: %d",
                        cep_id);
                /* FIXME: It should call unknown_flow policy of EFCP */
                du_destroy(du);
                return -1;
        }
        if (efcp->state == EFCP_DEALLOCATED) {
                spin_unlock_bh(&container->lock);
                du_destroy(du);
                LOG_DBG("EFCP already deallocated");
                return 0;
        }
        atomic_inc(&efcp->pending_ops);

        pdu_type = pci_type(&du->pci);
        if (pdu_type == PDU_TYPE_DT &&
        		efcp->connection->destination_cep_id < 0) {
        	/* Check that the destination cep-id is set to avoid races,
        	 * otherwise set it*/
        	efcp->connection->destination_cep_id = pci_cep_source(&du->pci);
        }
        spin_unlock_bh(&container->lock);

        ret = efcp_receive(efcp, du);

        spin_lock_bh(&container->lock);
        if (atomic_dec_and_test(&efcp->pending_ops) &&
        		efcp->state == EFCP_DEALLOCATED) {
                spin_unlock_bh(&container->lock);
		wake_up_interruptible(&container->del_wq);
                return ret;
        }
        spin_unlock_bh(&container->lock);

        return ret;
#endif
        return pdTRUE;
}

BaseType_t xEfcpReceive(efcp_t * pxEfcp, du_t *  pxDu);

BaseType_t xEfcpReceive(efcp_t * pxEfcp, du_t *  pxDu)
{
        pduType_t    xPduType;
    	pci_t * 	 pxPciTmp;

    	/* Cast to PCI type structure*/
    	pxPciTmp = vCastPointerTo_pci_t(pxDu->pxNetworkBuffer->pucEthernetBuffer);

        xPduType = pxPciTmp->xType;

        /* Verify the type of PDU*/
        if (pdu_type_is_control(xPduType))
        {
                if (!pxEfcp->pxDtp || !pxEfcp->pxDtp)//efcp->dtp->dtcp
                {
                        ESP_LOGE( TAG_EFCP, "No DTCP instance available");
                        xDuDestroy(pxDu);
                        return pdFALSE;
                }

               /* if (dtcp_common_rcv_control(pxEfcp->pxDtp, pxDu)) //efcp->dtp->dtcp
                        return pdFALSE;*/

                return pdTRUE;
        }

        if (xDtpReceive(pxEfcp->pxDtp, pxDu)) {
                ESP_LOGE(TAG_EFCP, "DTP cannot receive this PDU");
                return pdFALSE;
        }

        return pdTRUE;
}

