#ifndef _RIBD_OBJ_H_INCLUDED
#define _RIBD_OBJ_H_INCLUDED

#include "common/rsrc.h"

#include "SerDesMessage.h"
#include "SerDesAData.h"
#include "RibObject.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CDAP_OPCODE M_STOP_R
#define APP_CONNECTION_TABLE_SIZE (5)
#define RESPONSE_HANDLER_TABLE_SIZE (10)

#define RIB_TABLE_SIZE (10)

typedef enum
{
    ENROLLMENT,
    FLOW_ALLOCATOR,
    OPERATIONAL,
    FLOW,

} eObjectType_t;

typedef struct
{
    bool_t (*start_response)(struct ipcpInstanceData_t *pxData,
                             string_t pcRemoteAPName, serObjectValue_t *pxSerObjectValue);

    bool_t (*stop_response)(struct ipcpInstanceData_t *pxData,
                            string_t pcRemoteAPName);

    bool_t (*create_response)(struct ipcpInstanceData_t *pxData,
                              serObjectValue_t *pxSerObjectValue, int result);

    bool_t (*delete_response)(struct ipcpInstanceData_t *pxData,
                              ribObject_t *pxRibObject, int invoke_id);
} ribCallbackOps_t;

typedef struct
{
    bool_t xValid;

    ribObject_t *pxRibObject;
} ribObjectRow_t;

typedef enum
{
    eCONNECTION_IN_PROGRESS = 0,
    eCONNECTED,
    eRELEASED,
} connectionStatus_t;

typedef struct xAPP_CONNECTION
{
    rname_t xSourceInfo;
    rname_t xDestinationInfo;
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
    ribCallbackOps_t *pxCallbackHandler;
    bool_t xValid;

} responseHandlersRow_t;

typedef struct
{
    MessageSerDes_t xMsgSD;

    ADataSerDes_t xADataSD;

    /* Pool for messageCdap_t objects */
    rsrcPoolP_t xMsgPool;

    /* Pool for ribCallbackOps_t objects */
    rsrcPoolP_t xCbPool;

    /* Pool for DU objects */
    rsrcPoolP_t xDuPool;

    /* Table to manage the app connections */
    appConnectionTableRow_t xAppConnectionTable[APP_CONNECTION_TABLE_SIZE];

    /* Table to manage the pending request to response*/
    responseHandlersRow_t xPendingResponseHandlersTable[RESPONSE_HANDLER_TABLE_SIZE];

    ribObjectRow_t xRibObjectTable[RIB_TABLE_SIZE];

} Ribd_t;

#ifdef __cplusplus
}
#endif

#endif /* _RIBD_OBJ_H_INCLUDED */
