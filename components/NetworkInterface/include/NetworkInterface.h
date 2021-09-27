


#ifndef NETWORK_INTERFACE_H
    #define NETWORK_INTERFACE_H

    #ifdef __cplusplus
        extern "C" {
    #endif


#include "configSensor.h"

	#define WIFI_CONNECTED_BIT BIT0
	#define WIFI_FAIL_BIT      BIT1
	//#include "ShimIPCP.h"




/* INTERNAL API FUNCTIONS. */
   BaseType_t xNetworkInterfaceInitialise( const MACAddress_t * phyDev );
   BaseType_t xNetworkInterfaceOutput( NetworkBufferDescriptor_t * const pxNetworkBuffer,
                                        BaseType_t xReleaseAfterSend );
   BaseType_t xNetworkInterfaceDisconnect(void);
   esp_err_t xNetworkInterfaceInput( void * buffer, uint16_t len, void * eb );

/* The following function is defined only when BufferAllocation_1.c is linked in the project. */
    void vNetworkInterfaceAllocateRAMToBuffers( NetworkBufferDescriptor_t pxNetworkBuffers[ NUM_NETWORK_BUFFER_DESCRIPTORS ] );

/* The following function is defined only when BufferAllocation_1.c is linked in the project. */
    BaseType_t xGetPhyLinkStatus( void );

    #ifdef __cplusplus
        } /* extern "C" */
    #endif

#endif /* NETWORK_INTERFACE_H */
