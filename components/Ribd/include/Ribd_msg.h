#ifndef _RIBD_MSG_H_INCLUDED
#define _RIBD_MSG_H_INCLUDED

#include "common/rina_name.h"
#include "common/rina_ids.h"

#include "rina_common_port.h"
#include "SerDes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    M_CONNECT = 0,
    M_CONNECT_R,
    M_RELEASE,
    M_RELEASE_R,
    M_CREATE,
    M_CREATE_R,
    M_DELETE,
    M_DELETE_R,
    M_READ,
    M_READ_R,
    M_CANCELREAD,
    M_CANCELREAD_R,
    M_WRITE,
    M_WRITE_R,
    M_START,
    M_START_R,
    M_STOP,
    M_STOP_R,
} opCode_t;

#define MAX_CDAP_OPCODE M_STOP_R

extern char *opcodeNamesTable[];

typedef struct
{
    /*Operation Code*/
    opCode_t eOpCode;

    /*absSyntax*/

    /*Destination Info*/
    rname_t xDestinationInfo;

    /*Source Info*/
    rname_t xSourceInfo;

    /*Version*/
    int64_t version;

    /*Authentication Policy*/
    authPolicy_t xAuthPolicy;

    /*Invoke ID*/
    invokeId_t nInvokeID;

    /*Name of the object class of objName*/
    string_t pcObjClass;

    /*Object name, unique in its class*/
    string_t pcObjName;

    /*Unique object instance*/
    int64_t objInst;

    /*value of object in read/write/etc.*/
    serObjectValue_t *pxObjValue;

    /*Result*/
    int result;

    /* Extra message for the result. */
    string_t pcResultReason;

} messageCdap_t;

#ifdef __cplusplus
}
#endif

#endif /* _RIBD_MSG_H_INCLUDED */
