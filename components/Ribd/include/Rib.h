/*
 * Rib.h
 *
 *  Created on: 24 Mar. 2022
 *      Author: i2CAT
 */

#ifndef RIB_H_INCLUDED
#define RIB_H_INCLUDED

#include "configSensor.h"
#include "portability/port.h"
#include "common/rina_ids.h"

#define RIB_TABLE_SIZE (10)

struct ribObject_t;

typedef enum
{
    ENROLLMENT,
    FLOW_ALLOCATOR,
    OPERATIONAL,
    FLOW,

} eObjectType_t;

typedef struct xSER_OBJECT_VALUE
{
    /*Buffer with the message*/
    void *pvSerBuffer;

    /*Size of the buffer*/
    size_t xSerLength;

} serObjectValue_t;

struct ribCallbackOps_t
{
    bool_t (*start_response)(string_t pcRemoteAPName, serObjectValue_t *pxSerObjectValue);

    bool_t (*stop_response)(string_t pcRemoteAPName);

    bool_t (*create_response)(serObjectValue_t *pxSerObjectValue, int result);
};

struct ribObjOps_t
{
    bool_t (*start)(struct ribObject_t *pxRibObject, serObjectValue_t *pxObjValue, string_t pcRemoteApName,
                    string_t pxLocalApName, int invokeId, portId_t xN1Port);

    bool_t (*stop)(struct ribObject_t *pxRibObject, serObjectValue_t *pxObjValue, string_t pcRemoteApName,
                       string_t pxLocalApName, int invokeId, portId_t xN1Port);

    bool_t (*create)(struct ribObject_t *, serObjectValue_t *pxObjValue, string_t remote_process_name,
                         string_t local_process_name, int invokeId, portId_t xN1Port);

    bool_t (*delete)(struct ribObject_t *pxRibObject, int invoke_id);
};

struct ribObject_t
{
    struct ribObjOps_t *pxObjOps;
    string_t ucObjName;
    string_t ucObjClass;
    long ulObjInst;
    string_t ucDisplayableValue;
};

struct ribObjectRow_t
{
    bool_t xValid;
    struct ribObject_t *pxRibObject;
};

struct ribObject_t *pxRibFindObject(string_t ucRibObjectName);

struct ribObject_t *pxRibCreateObject(string_t ucObjName, long ulObjInst,
                                      string_t ucDisplayableValue, string_t ucObjClass, eObjectType_t eObjType);

#endif /* RIB_H_ */
