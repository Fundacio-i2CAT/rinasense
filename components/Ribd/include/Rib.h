/*
 * Rib.h
 *
 *  Created on: 24 Mar. 2022
 *      Author: i2CAT
 */


#ifndef RIB_H_INCLUDED
#define RIB_H_INCLUDED

#include "configSensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define RIB_TABLE_SIZE       (10)


struct  ribObject_t;

typedef enum{
    ENROLLMENT,
    FLOW_ALLOCATOR,
    NEIGHBOR,

}eObjectType_t;

typedef struct xSER_OBJECT_VALUE{
    /*Buffer with the message*/
    void * pvSerBuffer;

    /*Size of the buffer*/
    size_t xSerLength;

}serObjectValue_t;

struct ribCallbackOps_t {
    BaseType_t (*start_response)(void *, struct ser_obj_value *);

    BaseType_t (*stop_response)(void *);

    BaseType_t (*create_response)( struct ser_obj_value *);
};

struct ribObjOps_t{
    /*BaseType_t (*start)(struct ribObject_t *, struct ser_obj_value *obj_value, void *remote_process_name,
                 void *local_process_name, int invoke_id, int n1_port);

    BaseType_t (*stop)(struct rib_obj *, struct ipcp *,
                struct ser_obj_value *obj_value, void *remote_process_name,
                void *local_process_name, int invoke_id, int n1_port);

    BaseType_t (*read)(struct rib_obj *, struct ipcp *,
                struct ser_obj_value *obj_value, void *remote_process_name,
                void *local_process_name, int invoke_id, int n1_port);

    BaseType_t (*write)(struct rib_obj *, struct ipcp *,
                 struct ser_obj_value *obj_value, void *remote_process_name,
                 void *local_process_name, int invoke_id, int n1_port);*/

    BaseType_t (*create)(struct ribObject_t *, serObjectValue_t *pxObjValue, string_t remote_process_name,
                  string_t local_process_name, int invokeId, portId_t xN1Port);
/*
    BaseType_t (*delete)(struct rib_obj *, struct ipcp *,
                  struct ser_obj_value *obj_value, void *remote_process_name,
                  void *local_process_name, int invoke_id, int n1_port);*/

};

struct ribObject_t{
    struct ribObjOps_t *pxObjOps;
    string_t ucObjName;
    string_t ucObjClass;
    long  ulObjInst;
    string_t ucDisplayableValue;
};

struct ribObjectRow_t{
    BaseType_t xValid;
    struct ribObject_t * pxRibObject;
};

struct ribObject_t *pxRibFindObject(string_t ucRibObjectName);

void vRibAddObjectEntry(struct ribObject_t *pxRibObject);

#endif /* RIB_H_ */