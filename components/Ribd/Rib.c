#include <stdio.h>
#include <string.h>

#include "rina_common_port.h"
#include "configRINA.h"
#include "BufferManagement.h"
#include "CDAP.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "Ribd.h"
#include "configSensor.h"
#include "Rib.h"
#include "Enrollment_api.h"

/*Table of Objects*/
struct ribObjectRow_t xRibObjectTable[RIB_TABLE_SIZE];

static void vRibAddObjectEntry(struct ribObject_t *pxRibObject)
{
    num_t x = 0;

    for (x = 0; x < RIB_TABLE_SIZE; x++)
    {
        if (xRibObjectTable[x].xValid == false)
        {
            xRibObjectTable[x].pxRibObject = pxRibObject;
            xRibObjectTable[x].xValid = true;
            LOGD(TAG_RIB, "Rib Object Entry successful: %p", pxRibObject);

            break;
        }
    }
}

struct ribObject_t *pxRibFindObject(string_t ucRibObjectName)
{
    LOGD(TAG_RIB, "Looking for the object %s in the RIB Object Table", ucRibObjectName);
    num_t x = 0;
    struct ribObject_t *pxRibObject;
    pxRibObject = pvRsMemAlloc(sizeof(*pxRibObject));

    for (x = 0; x < RIB_TABLE_SIZE; x++)

    {
        if (xRibObjectTable[x].xValid == true)
        {
            pxRibObject = xRibObjectTable[x].pxRibObject;
            ESP_LOGD(TAG_RIB, "RibObj->ucObjName'%s', ucRibObjectName:'%s'", pxRibObject->ucObjName, ucRibObjectName);
            if (!strcmp(pxRibObject->ucObjName, ucRibObjectName))
            {
                LOGI(TAG_RIB, "RibObj founded '%p', '%s'", pxRibObject, pxRibObject->ucObjName);

                return pxRibObject;
                break;
            }
        }
    }
    LOGI(TAG_IPCPMANAGER, "RibObj '%s' not founded", ucRibObjectName);

    return NULL;
}

struct ribObject_t *pxRibCreateObject(string_t ucObjName,
                                      long ulObjInst,
                                      string_t ucDisplayableValue,
                                      string_t ucObjClass,
                                      eObjectType_t eObjType)
{
    LOGI(TAG_RIB, "Creating object %s into the RIB", ucObjName);
    struct ribObject_t *pxObj = pvRsMemAlloc(sizeof(*pxObj));
    struct ribObjOps_t *pxObjOps = pvRsMemAlloc(sizeof(*pxObjOps));

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
    vRibAddObjectEntry(pxObj);
    LOGI(TAG_RIB, "RIB object created: %s, %p", pxObj->ucObjName, pxObj);

    return pxObj;
}
