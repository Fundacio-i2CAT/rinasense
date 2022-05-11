

/*Move to configurations RINA*/

#include "Enrollment.h"
#include "Rib.h"

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
    BaseType_t xValid;

} appConnectionTableRow_t;

typedef struct xRESPONSE_HANDLER_ROW
{
    int32_t invokeID;
    struct ribCallbackOps_t *pxCallbackHandler;
    BaseType_t xValid;

} responseHandlersRow_t;

typedef struct xMESSAGE_CDAP
{
    /*Operation Code*/
    opCode_t eOpCode;

    /*absSyntax*/

    /*Destination Info*/
    //name_t *pxDestinationInfo;
    string_t pcDestApName;
    string_t pcDestApInst; 
    string_t pcDestAeName; 
    string_t pcDestAeInst;  

    /*Source Info*/
    //name_t *pxSourceInfo;
    string_t pcSrcApName;
    string_t pcSrcApInst; 
    string_t pcSrcAeName; 
    string_t pcSrcAeInst;  

    /*Version*/
    int64_t version;

    /*Authentication Policy*/
    //authPolicy_t *pxAuthPolicy;
    string_t  pcAuthPoliName;
    string_t  pcAuthPoliVersion;

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

BaseType_t xRibdConnectToIpcp(name_t *pxSource, name_t *pxDestInfo, portId_t xN1flowPortId, authPolicy_t *pxAuth);
BaseType_t xRibdDisconnectToIpcp(portId_t xN1flowPortId);
BaseType_t xRibdProcessLayerManagementPDU(struct ipcpInstanceData_t *pxData, portId_t xN1flowPortId, struct du_t *pxDu);
BaseType_t xRibdSendRequest(string_t pcObjClass, string_t pcObjName, long objInst,
                            opCode_t eMsgType, portId_t n1_port, serObjectValue_t *pxObjVal);

BaseType_t xRibdSendResponse(string_t pcObjClass, string_t pcObjName, long objInst,
                             int result, string_t pcResultReason,
                             opCode_t eOpCode, int invokeId, portId_t xN1Port,
                             serObjectValue_t *pxObjVal);

    BaseType_t xTest(void);
