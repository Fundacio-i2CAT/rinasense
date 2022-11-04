#ifndef RIB_OBJECT_H_INCLUDED
#define RIB_OBJECT_H_INCLUDED

#include "portability/port.h"
#include "common/rina_ids.h"

#include "SerDes.h"
#include "Enrollment_obj.h"
#include "IPCP_instance.h"

#ifdef __cplusplus
extern "C" {
#endif

struct xRIBOBJ;

typedef struct
{
    bool_t (*start)(struct ipcpInstanceData_t *pxData,
                    struct xRIBOBJ *pxRibObject, serObjectValue_t *pxObjValue, string_t pcRemoteApName,
                    string_t pxLocalApName, int invokeId, portId_t xN1Port);

    bool_t (*stop)(struct ipcpInstanceData_t *pxData,
                   struct xRIBOBJ *pxRibObject, serObjectValue_t *pxObjValue, string_t pcRemoteApName,
                   string_t pxLocalApName, int invokeId, portId_t xN1Port);

    bool_t (*create)(struct ipcpInstanceData_t *pxData,
                     struct xRIBOBJ *pxRibObject, serObjectValue_t *pxObjValue, string_t remote_process_name,
                     string_t local_process_name, int invokeId, portId_t xN1Port);

    bool_t (*delete)(struct ipcpInstanceData_t *pxData, struct xRIBOBJ *pxRibObject, int invoke_id);

} ribObjOps_t;


struct xRIBOBJ
{
    ribObjOps_t *pxObjOps;
    string_t ucObjName;
    string_t ucObjClass;
    long ulObjInst;
    string_t ucDisplayableValue;
};

typedef struct xRIBOBJ ribObject_t;

#ifdef __cplusplus
}
#endif

#endif /* RIB_OBJECT_H_INCLUDED */
