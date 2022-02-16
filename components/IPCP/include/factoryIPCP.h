#ifndef FACTORY_IPCP_H_
#define FACTORY_IPCP_H_

#include "IPCP.h"


struct ipcpFactoryData_t;

/* TO add other kind of factories could be: IP, EThernet, ZigBee, BLE, etc*/
typedef enum TYPE_IPCP_FACTORY
{
	eFActoryShimWiFi = 0, 
    eFactoryShimBLE,
	eFactoryNormal,

}ipcpFactoryType_t;


typedef struct xIPCP_FACTORY_OPS {
   
        BaseType_t                    (* init)(struct ipcpFactoryData_t * pxData);
        
        BaseType_t                    (* fini)(struct ipcpFactoryData * pxData);

        ipcpInstance_t *              (* create)(struct ipcpFactoryData_t * pxData, ipcProcessId_t xIpcpId);

        #if 0
        int                    (* destroy)(struct ipcp_factory_data * data,
                                           struct ipcp_instance *     inst);

                                           
    #endif
    string_t TEST;

}ipcpFactoryOps_t;

typedef struct xIPCP_FACTORY{
    ListItem_t  xIPCPFactoryItem;
    ipcpFactoryType_t    xFactoryType;
    struct ipcpFactoryData_t *pxFactoryData;
    ipcpFactoryOps_t *pxFactoryOps;
}ipcpFactory_t;

typedef struct xFACTORIES_{
    List_t xFactoriesList;
}factories_t;

BaseType_t xFactoryIPCPInit(ipcpFactory_t * pxFactory);


ipcpFactory_t * pxFactoryIPCPFind(factories_t * pxFactoriesList, ipcpFactoryType_t xType);

BaseType_t xFactoryIPCPRegister(factories_t * pxFactoriesList,
                                     ipcpFactoryType_t         xType,
                                     struct ipcpFactoryData_t *      pxFactoryData,
                                     ipcpFactoryOps_t * pxFactoryOps);


#endif