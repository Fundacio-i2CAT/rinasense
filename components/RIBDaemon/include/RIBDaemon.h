
#ifndef RIB_DAEMON_H
#define RIB_DAEMON_H

/*Move to configurations RINA*/
#define APP_CONNECTION_TABLE_SIZE ( 5 )

typedef enum{
    eCONNECTION_IN_PROGRESS = 0,
    eCONNECTED,
    eRELEASED,
}connectionStatus_t; 
typedef struct xAPP_CONNECTION{
    name_t * pxSourceInfo;
    name_t * pxDestinationInfo;
    uint8_t  uCdapVersion;
    uint8_t uRibVersion;
    connectionStatus_t xStatus;
    
}appConnection_t;

typedef struct xAPP_CONNECTION_ROW{
    appConnection_t * pxAppConnection;
    portId_t  xN1portId;
    BaseType_t xValid;
}appConnectionTableRow_t;




BaseType_t xRIBDaemonConnectToIpcp( name_t *pxSource, name_t * pxDestInfo, portId_t xN1flowPortId, authPolicy_t auth);
BaseType_t xRIBDaemonDisconnectToIpcp( portId_t xN1flowPortId );
BaseType_t xRIBDaemonProcessLayerManagementPDU ();



#endif
