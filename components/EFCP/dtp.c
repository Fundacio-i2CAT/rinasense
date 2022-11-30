/*
 * dtp.c
 *
 *  Created on: 11 oct. 2021
 *      Author: i2CAT
 */

#include <limits.h>

#include "common/netbuf.h"
#include "common/rsrc.h"
#include "portability/port.h"
#include "du.h"
#include "rina_common_port.h"
#include "rmt.h"
#include "EFCP.h"

#define TAG_DTP "[DTP]"

/* REVIEWED CODE STARTS HERE */

bool_t xDtpPduSend(dtp_t *pxDtp, struct rmt_t *pxRmt, du_t *pxDu)
{
    struct efcpContainer_t *pxEfcpContainer;
    cepId_t destCepId;

    RsAssert(pxDtp);
    RsAssert(pxRmt);
    RsAssert(pxDu);

    /* Remote flow case */
    if (PCI_GET(pxDu, xSource) != PCI_GET(pxDu, xDestination)) {
        if (!xRmtSend(pxRmt, pxDu)) {
            LOGE(TAG_DTP, "Problems sending PDU to RMT");
            return false;
        }

        return true;
    }

    /* Local flow case */
    destCepId = PCI_GET(pxDu, PCI_CONN_DST_ID);
    pxEfcpContainer = pxDtp->pxEfcp->pxEfcpContainer;

    // pxEfcpContainer = pxDtp->pxEfcp->pxEfcpContainer;
    if (!pxEfcpContainer || !xDuDecap(sizeof(pci_t), pxDu) || !xDuIsOk(pxDu)) {
        LOGE(TAG_DTP, "Could not retrieve the EFCP container in loopback operation");
        return false;
    }

    if (xEfcpContainerReceive(pxEfcpContainer, destCepId, pxDu)) {
        LOGE(TAG_DTP, "Problems sending PDU to loopback EFCP");
        return false;
    }

    return 0;
}

bool_t xDtpWrite(dtp_t *pxDtp, du_t *pxDu)
{
    struct dtp_ps *ps;
    seqNum_t xCsn;
    buffer_t xPci;
    size_t sz;

    RsAssert(pxDtp);
    RsAssert(pxDu);
    RsAssert(eNetBufType(pxDu) == NB_RINA_DATA);

    /* Allocate some memory for the PCI header */
    if (!(xPci = pxRsrcAlloc(pxDtp->pxEfcp->pxEfcpContainer->xPciPool, NULL))) {
        LOGE(TAG_DTP, "Failed to allocate memory for PCI");
        return false;
    }

    /* Save the total size so we do not have to do trickery later
     * down */
    sz = unNetBufTotalSize(pxDu);

    if (!xDuEncap(xPci, sizeof(pci_t), pxDu)) {
        LOGE(TAG_DTP, "Could not encap PDU");
        return false;
    }

    xCsn = ++pxDtp->xDtpStateVector.xNextSeqNumberToSend;

    /* Setup the PCI for DTP */

    PCI_SET(pxDu, PCI_VERSION, 0x01);
    PCI_SET(pxDu, PCI_CONN_SRC_ID, pxDtp->pxEfcp->pxConnection->xSourceCepId);
    PCI_SET(pxDu, PCI_CONN_DST_ID, pxDtp->pxEfcp->pxConnection->xDestinationCepId);
    PCI_SET(pxDu, PCI_CONN_QOS_ID, pxDtp->pxEfcp->pxConnection->xQosId);
    PCI_SET(pxDu, PCI_ADDR_DST, pxDtp->pxEfcp->pxConnection->xDestinationAddress);
    PCI_SET(pxDu, PCI_ADDR_SRC, pxDtp->pxEfcp->pxConnection->xSourceAddress);
    PCI_SET(pxDu, PCI_FLAGS, 0);
    PCI_SET(pxDu, PCI_TYPE, PDU_TYPE_DT);
    PCI_SET(pxDu, PCI_PDU_LEN, sz);
    PCI_SET(pxDu, PCI_SEQ_NO, xCsn);

    if (pxDtp->xDtpStateVector.xDrfFlag) {
        pduFlags_t xPciFlags;

        xPciFlags = PCI_GET(pxDu, PCI_FLAGS);
        xPciFlags |= PDU_FLAGS_DATA_RUN;

        PCI_SET(pxDu, PCI_FLAGS, xPciFlags);

        LOGI(TAG_DTP, "PCI FLAG: 0x%04x", PCI_GET(pxDu, PCI_FLAGS));
    }

    if (!xDtpPduSend(pxDtp, pxDtp->pxRmt, pxDu))
        return false;

    return true;
}

static void vDtpStateVectorInit(dtpSv_t *pxDtpSv)
{
    pxDtpSv->xNextSeqNumberToSend = 0;
    pxDtpSv->xMaxSeqNumberToSend = 0;
    pxDtpSv->xSeqNumberRolloverThreshold = 0;
    pxDtpSv->xMaxSeqNumberRcvd = 0;
    pxDtpSv->stats.drop_pdus = 0;
    pxDtpSv->stats.err_pdus = 0;
    pxDtpSv->stats.tx_pdus = 0;
    pxDtpSv->stats.tx_bytes = 0;
    pxDtpSv->stats.rx_pdus = 0;
    pxDtpSv->stats.rx_bytes = 0;
    pxDtpSv->xRexmsnCtrl = false;
    pxDtpSv->xRateBased = false;
    pxDtpSv->xWindowBased = false;
    pxDtpSv->xDrfRequired = true;
    pxDtpSv->xRateFulfiled = false;
    pxDtpSv->xMaxFlowPduSize = UINT_MAX;
    pxDtpSv->xMaxFlowSduSize = UINT_MAX;
    pxDtpSv->xRcvLeftWindowEdge = 0;
    pxDtpSv->xWindowClosed = false;
    pxDtpSv->xDrfFlag = true;
}

dtp_t *pxDtpCreate(struct efcp_t *pxEfcp,
                   struct rmt_t *pxRmt,
                   dtpConfig_t *pxDtpCfg)
{
    dtp_t *pxDtp;
    string_t *psName;
    dtpSv_t *pxDtpSv;

    RsAssert(pxEfcp);
    RsAssert(pxDtpCfg);
    RsAssert(pxRmt);

    if (!(pxDtp = pvRsMemAlloc(sizeof(dtp_t)))) {
        LOGE(TAG_DTP, "Cannot create DTP instance");
        return NULL;
    }

    pxDtp->pxEfcp = pxEfcp;

    vDtpStateVectorInit(&pxDtp->xDtpStateVector);

    pxDtp->pxRmt = pxRmt;

    return pxDtp;
}

static inline bool_t xDtpPduPost(dtp_t *pxDtp, du_t *pxDu)
{
    RsAssert(pxDtp);
    RsAssert(pxDu);

    if (!xEfcpEnqueue(pxDtp->pxEfcp, pxDtp->pxEfcp->pxConnection->xPortId, pxDu)) {
        LOGE(TAG_DTP, "Could not enqueue SDU to EFCP");
        return false;
    }

    LOGI(TAG_DTP, "DTP enqueued to upper IPCP");
    return true;
}

bool_t xDtpReceive(dtp_t *pxDtp, du_t *pxDu)
{
    seqNum_t xSeqNum;
    seqNum_t xLWE;
    bool_t xRtxCtrl = false;
    seqNum_t xMaxSduGap;
    struct efcp_t *pxEfcp = 0;

    RsAssert(pxDtp);
    RsAssert(pxDu);

    LOGI(TAG_DTP, "DTP receive started...");

    pxEfcp = pxDtp->pxEfcp;

    xLWE = pxDtp->xDtpStateVector.xRcvLeftWindowEdge;

    xMaxSduGap = pxDtp->xDtpCfg.xMaxSduGap;

    xSeqNum = PCI_GET(pxDu, PCI_SEQ_NO);

    if (pxDtp->xDtpStateVector.xDrfRequired) {
        LOGD(TAG_DTP, "PCI FLAG: 0x%04x", PCI_GET(pxDu, PCI_FLAGS));

        if (PCI_GET(pxDu, PCI_FLAGS) & PDU_FLAGS_DATA_RUN) {
            LOGI(TAG_DTP, "Data Run Flag");

            pxDtp->xDtpStateVector.xDrfRequired = false;
            pxDtp->xDtpStateVector.xRcvLeftWindowEdge = xSeqNum;

            // dtp_send_pending_ctrl_pdus(instance); // check this
            xDtpPduPost(pxDtp, pxDu); // check this
            // stats_inc_bytes(rx, instance->sv, sbytes);

            return true;
        }

        /* LOGE(TAG_DTP, "Expecting DRF but not present, dropping PDU %d...", */
        /*      xSeqNum); */
        xDtpPduPost(pxDtp, pxDu);
        // stats_inc(drop, instance->sv);
        // spin_unlock_bh(&instance->sv_lock);

        return true;
    }

    /*
     * NOTE:
     *   no need to check presence of in_order or dtcp because in case
     *   they are not, LWE is not updated and always 0
     */
    if (xSeqNum <= xLWE) {
        /* Duplicate PDU or flow control overrun */
        LOGE(TAG_DTP, "Duplicate PDU or flow control overrun.SN: %u, LWE:%u", xSeqNum, xLWE);
        // stats_inc(drop, instance->sv);

        // spin_unlock_bh(&instance->sv_lock);

        return 0;
    }

    xLWE = pxDtp->xDtpStateVector.xRcvLeftWindowEdge;

    LOGI(TAG_DTP, "DTP receive LWE: %u", xLWE);
    if (xSeqNum == xLWE + 1) {
        pxDtp->xDtpStateVector.xRcvLeftWindowEdge = xSeqNum;

        xLWE = xSeqNum;
    }

    xDtpPduPost(pxDtp, pxDu);

    LOGI(TAG_DTP, "DTP receive ended...");

    return true;
}

bool_t xDtpDestroy(dtp_t *pxDtp)
{
    dtcp_t *pxDtcp = NULL;

    RsAssert(pxDtp);

    LOGI(TAG_DTP, "DTP %pK destroyed successfully", pxDtp);

    // robject_del(&instance->robj);
    vRsMemFree(pxDtp);

    return true;
}




