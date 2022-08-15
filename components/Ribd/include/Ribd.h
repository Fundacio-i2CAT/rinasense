#ifndef _RIBD_H_INCLUDED
#define _RIBD_H_INCLUDED

/*Move to configurations RINA*/

#include "common/rina_name.h"

#include "Rib.h"
#include "IPCP_normal_defs.h"
#include "rina_common_port.h"

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
#define APP_CONNECTION_TABLE_SIZE (5)
#define RESPONSE_HANDLER_TABLE_SIZE (10)

typedef enum
{

    eCONNECTION_IN_PROGRESS = 0,
    eCONNECTED,
    eRELEASED,
} connectionStatus_t;

typedef struct xAPP_CONNECTION
{
    name_t *pxSourceInfo;
    name_t *pxDestinationInfo;
    uint8_t uCdapVersion;
    uint8_t uRibVersion;
    connectionStatus_t xStatus;

} appConnection_t;

typedef struct xAPP_CONNECTION_ROW
{
    appConnection_t *pxAppConnection;
    portId_t xN1portId;
    bool_t xValid;

} appConnectionTableRow_t;

typedef struct xRESPONSE_HANDLER_ROW
{
    int32_t invokeID;
    struct ribCallbackOps_t *pxCallbackHandler;
    bool_t xValid;

} responseHandlersRow_t;

typedef struct xMESSAGE_CDAP
{
    /*Operation Code*/
    opCode_t eOpCode;

    /*absSyntax*/

    /*Destination Info*/
    name_t *pxDestinationInfo;

    /*Source Info*/
    name_t *pxSourceInfo;

    /*Version*/
    intmax_t version;

    /*Authentication Policy*/
    authPolicy_t *pxAuthPolicy;

    /*Invoke ID*/
    int32_t invokeID;

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

} messageCdap_t;

typedef struct xA_DATA_MESSAGE
{
    int32_t xSourceAddress;
    int32_t xDestinationAddress;
    serObjectValue_t *pxMsgCdap;
} aDataMsg_t;

#endif // _RIBD_H_INCLUDED
