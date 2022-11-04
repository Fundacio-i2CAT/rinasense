#include <stdio.h>
#include <string.h>

#include "configRINA.h"

#include "rina_common_port.h"
#include "BufferManagement.h"
#include "CDAP.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "Ribd.h"
#include "configSensor.h"
#include "Rib.h"
#include "Enrollment_api.h"
#include "FlowAllocator_api.h"

bool_t xRibAddObjectEntry(Ribd_t *pxRibd, ribObject_t *pxRibObject)
{
    num_t x = 0;

    for (x = 0; x < RIB_TABLE_SIZE; x++) {
        if (pxRibd->xRibObjectTable[x].xValid == false) {
            pxRibd->xRibObjectTable[x].pxRibObject = pxRibObject;
            pxRibd->xRibObjectTable[x].xValid = true;

            LOGD(TAG_RIB, "RIB Object Entry successful: %p", pxRibObject);

            return true;
        }
    }

    return false;
}

ribObject_t *pxRibFindObject(Ribd_t *pxRibd, string_t ucRibObjectName)
{
    num_t x = 0;
    ribObject_t *pxRibObject;

    LOGD(TAG_RIB, "Looking for the object '%s' in the RIB Object Table", ucRibObjectName);

    for (x = 0; x < RIB_TABLE_SIZE; x++) {

        if (pxRibd->xRibObjectTable[x].xValid == true) {

            pxRibObject = pxRibd->xRibObjectTable[x].pxRibObject;

            LOGD(TAG_RIB, "RibObj->ucObjName: '%s', ucRibObjectName: '%s'",
                 pxRibObject->ucObjName, ucRibObjectName);

            if (!strcmp(pxRibObject->ucObjName, ucRibObjectName)) {
                LOGI(TAG_RIB, "RIB Object found '%p', '%s'", pxRibObject, pxRibObject->ucObjName);

                return pxRibObject;
                break;
            }
        }
    }

    LOGI(TAG_RIB, "RIB Object '%s' not found", ucRibObjectName);

    return NULL;
}

ribObject_t *pxRibCreateObject(Ribd_t *pxRibd,
                               string_t ucObjName,
                               long ulObjInst,
                               string_t ucDisplayableValue,
                               string_t ucObjClass,
                               eObjectType_t eObjType)
{
    LOGI(TAG_RIB, "Creating object %s into the RIB", ucObjName);
    ribObject_t *pxObj = pvRsMemAlloc(sizeof(ribObject_t));
    ribObjOps_t *pxObjOps = pvRsMemAlloc(sizeof(ribObjOps_t));

    pxObj->ucObjName = strdup(ucObjName);
    pxObj->ulObjInst = ulObjInst;
    pxObj->ucDisplayableValue = ucDisplayableValue;
    pxObj->pxObjOps = pxObjOps;

    switch (eObjType)
    {
    case ENROLLMENT:
        pxObj->pxObjOps->stop = xEnrollmentHandleStop;
        pxObj->pxObjOps->start = xEnrollmentEnroller;
        break;

    case FLOW_ALLOCATOR:
        // M_create
        // M_Delete
        // M_write

        break;

    case OPERATIONAL:
        pxObj->pxObjOps->start = xEnrollmentHandleOperationalStart;
        break;

    case FLOW:
        pxObj->pxObjOps->delete = xFlowAllocatorHandleDelete;
        // pxObj->pxObjOps->create = xFlowAllocatorHandleCreate;

        break;

    default:
        pxObj->pxObjOps->create = NULL;
        break;
    }

    /*Add object into the table*/
    if (!xRibAddObjectEntry(pxRibd, pxObj)) {
    }
    else
        LOGI(TAG_RIB, "RIB object created: %s, %p", pxObj->ucObjName, pxObj);

    return pxObj;
}
