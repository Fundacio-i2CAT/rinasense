#ifndef _RIB_OBJECT_H_INCLUDED
#define _RIB_OBJECT_H_INCLUDED

#include "portability/port.h"
#include "common/rina_ids.h"
#include "common/rina_name.h"

#include "SerDes.h"
#include "Enrollment_obj.h"
#include "IPCP_instance.h"

#ifdef __cplusplus
extern "C" {
#endif

struct xRIBOBJ;

typedef bool_t (*ribObjectMethod)(struct ipcpInstanceData_t *pxData,
                                  struct xRIBOBJ *pxThis,
                                  serObjectValue_t *pxObjValue,
                                  rname_t *pxRemoteName,
                                  rname_t *pxLocalName,
                                  invokeId_t nInvokeId,
                                  portId_t unPort);

typedef bool_t (*ribObjectShow)(struct ipcpInstanceData_t *pxData,
                                struct xRIBOBJ *pxThis);

typedef void (*ribObjectFree)(struct xRIBOBJ *pxThis);

struct xRIBOBJ
{
    string_t ucObjName;
    string_t ucObjClass;
    long ulObjInst;

    ribObjectMethod fnStart;
    ribObjectMethod fnStop;
    ribObjectMethod fnCreate;
    ribObjectMethod fnDelete;
    ribObjectMethod fnRead;
    ribObjectMethod fnWrite;

    ribObjectShow fnShow;

    ribObjectFree fnFree;
};

typedef struct xRIBOBJ ribObject_t;

#ifdef __cplusplus
}
#endif

#endif /* _RIB_OBJECT_H_INCLUDED */
