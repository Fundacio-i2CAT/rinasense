#include <stdio.h>
#include <string.h>

#include "common/rinasense_errors.h"
#include "configRINA.h"

#include "rina_common_port.h"
#include "CDAP.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "Ribd_defs.h"
#include "RibObject.h"
#include "Enrollment_api.h"
#include "FlowAllocator_api.h"

rsErr_t xRibObjectAdd(Ribd_t *pxRibd, void *pvOwner, ribObject_t *pxRibObject)
{
    num_t x = 0;

    for (x = 0; x < RIB_TABLE_SIZE; x++) {
        if (!pxRibd->pxRibObjects[x]) {
            pxRibd->pxRibObjects[x] = pxRibObject;

            if (pxRibObject->fnInit) {
                if (ERR_CHK(pxRibObject->fnInit(pxRibd, pvOwner, pxRibObject))) {
                    LOGE(TAG_RIB, "RIB Object Initialization failed: %s", pxRibObject->ucObjName);
                    return FAIL;
                }
            }

            pxRibObject->pvOwner = pvOwner;
            pxRibObject->pxRibd = pxRibd;

            LOGI(TAG_RIB, "RIB Object Entry successful: %p", pxRibObject);

            return SUCCESS;
        }
    }

    return ERR_SET(ERR_RIB_TABLE_FULL);
}

ribObject_t *pxRibObjectFind(Ribd_t *pxRibd, string_t ucRibObjectName)
{
    num_t x = 0;

    LOGD(TAG_RIB, "Looking for the object '%s' in the RIB Object Table", ucRibObjectName);

    for (x = 0; x < RIB_TABLE_SIZE; x++) {

        if (pxRibd->pxRibObjects[x]) {
            LOGD(TAG_RIB, "RibObj->ucObjName: '%s', ucRibObjectName: '%s'",
                 pxRibd->pxRibObjects[x]->ucObjName, ucRibObjectName);

            if (!strcmp(pxRibd->pxRibObjects[x]->ucObjName, ucRibObjectName))
                return pxRibd->pxRibObjects[x];
        }
    }

    LOGW(TAG_RIB, "RIB Object '%s' not found", ucRibObjectName);

    return NULL;
}

