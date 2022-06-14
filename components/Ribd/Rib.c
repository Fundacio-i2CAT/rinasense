#include <stdio.h>
#include <string.h>

/* FreeRTOS includes. */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "IPCP.h"
#include "common.h"
#include "configRINA.h"
#include "BufferManagement.h"
#include "CDAP.pb.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "Ribd.h"
#include "configSensor.h"
#include "Rib.h"

#include "esp_log.h"

/*Table of Objects*/
struct ribObjectRow_t xRibObjectTable[RIB_TABLE_SIZE];

void vRibAddObjectEntry(struct ribObject_t *pxRibObject)
{

    BaseType_t x = 0;

    for (x = 0; x < RIB_TABLE_SIZE; x++)
    {
        if (xRibObjectTable[x].xValid == pdFALSE)
        {
            xRibObjectTable[x].pxRibObject = pxRibObject;
            xRibObjectTable[x].xValid = pdTRUE;
            ESP_LOGI(TAG_RIB, "Rib Object Entry successful: %p", pxRibObject);

            break;
        }
    }
}

struct ribObject_t *pxRibFindObject(string_t ucRibObjectName)
{

    ESP_LOGI(TAG_RIB, "Looking for the object %s in the RIB Object Table", ucRibObjectName);
    BaseType_t x = 0;
    struct ribObject_t *pxRibObject;
    pxRibObject = pvPortMalloc(sizeof(*pxRibObject));

    for (x = 0; x < RIB_TABLE_SIZE; x++)

    {
        if (xRibObjectTable[x].xValid == pdTRUE)
        {
            pxRibObject = xRibObjectTable[x].pxRibObject;
            // ESP_LOGI(TAG_RIB, "RibObj->ucObjName'%s', ucRibObjectName:'%s'", pxRibObject->ucObjName, ucRibObjectName);
            if (!strcmp(pxRibObject->ucObjName, ucRibObjectName))
            {
                ESP_LOGI(TAG_RIB, "RibObj founded '%p', '%s'", pxRibObject, pxRibObject->ucObjName);

                return pxRibObject;
                break;
            }
        }
    }
    ESP_LOGI(TAG_IPCPMANAGER, "RibObj '%s' not founded", ucRibObjectName);

    return NULL;
}

struct ribObject_t *pxRibCreateObject(string_t ucObjName, long ulObjInst,
                                      string_t ucDisplayableValue, string_t ucObjClass, eObjectType_t eObjType)
{
    ESP_LOGI(TAG_RIB, "Creating object %s into the RIB", ucObjName);
    struct ribObject_t *pxObj = pvPortMalloc(sizeof(*pxObj));
    struct ribObjOps_t *pxObjOps = pvPortMalloc(sizeof(*pxObjOps));

    pxObj->ucObjName = ucObjName;
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

    default:
    {
        pxObj->pxObjOps->create = NULL;
    }
    break;
    }

    /*Add object into the table*/
    vRibAddObjectEntry(pxObj);
    ESP_LOGI(TAG_RIB, "RIB object created: %s, %p", pxObj->ucObjName, pxObj);

    return pxObj;
}
