#include "configSensor.h"
#include "portability/port.h"
#include "portability/posix/semaphore.h"
#include "rina_buffers.h"

//Structure used to store buffer used by the Interface Wifi (Change)
// Some attributes are not needed.

#ifndef pdTRUE_SIGNED
       /* Temporary solution: eventually the defines below will appear in 'Source\include\projdefs.h' */
       #define pdTRUE_SIGNED       pdTRUE
       #define pdFALSE_SIGNED      pdFALSE
       #define pdTRUE_UNSIGNED     ( 1U )
       #define pdFALSE_UNSIGNED    ( 0U )
       #define ipTRUE_BOOL         ( 1 == 1 )
       #define ipFALSE_BOOL        ( 1 == 2 )
#endif



#ifndef BUFFER_MANAGEMENT_H
    #define BUFFER_MANAGEMENT_H

    #ifdef __cplusplus
        extern "C" {
    #endif



/* NOTE PUBLIC API FUNCTIONS. */
    bool_t xNetworkBuffersInitialise( void );
    NetworkBufferDescriptor_t * pxGetNetworkBufferWithDescriptor( size_t xRequestedSizeBytes,
                                                                  useconds_t xTimeOutUS);
    NetworkBufferDescriptor_t * pxGetNetworkBufferWithDescriptorFor (const char *name,
                                                             size_t xRequestedSizeBytes,
                                                             useconds_t xTimeOutUS );                                                             

/* The definition of the below function is only available if BufferAllocation_2.c has been linked into the source. */
    NetworkBufferDescriptor_t * pxNetworkBufferGetFromISR( size_t xRequestedSizeBytes );
    void vReleaseNetworkBufferAndDescriptor( NetworkBufferDescriptor_t * const pxNetworkBuffer );

/* The definition of the below function is only available if BufferAllocation_2.c has been linked into the source. */
    bool_t vNetworkBufferReleaseFromISR( NetworkBufferDescriptor_t * const pxNetworkBuffer );
    uint8_t * pucGetNetworkBuffer( size_t * pxRequestedSizeBytes );
    uint8_t * pucGetNetworkBufferFor (const char *name, size_t * pxRequestedSizeBytes); // preferred
    void vReleaseNetworkBuffer( uint8_t * pucEthernetBuffer );

/* Get the current number of free network buffers. */
    size_t uxGetNumberOfFreeNetworkBuffers( void );

/* Get the lowest number of free network buffers. */
    size_t uxGetMinimumFreeNetworkBuffers( void );

/* Copy a network buffer into a bigger buffer. */
    NetworkBufferDescriptor_t * pxDuplicateNetworkBufferWithDescriptor( const NetworkBufferDescriptor_t * const pxNetworkBuffer,
                                                                        size_t uxNewLength );

/* Increase the size of a Network Buffer.
 * In case BufferAllocation_2.c is used, the new space must be allocated. */
    NetworkBufferDescriptor_t * pxResizeNetworkBufferWithDescriptor( NetworkBufferDescriptor_t * pxNetworkBuffer,
                                                                     size_t xNewSizeBytes );
    NetworkBufferDescriptor_t * pxResizeNetworkBufferWithDescriptorFor (const char *name,
                                                                NetworkBufferDescriptor_t *pxNetworkBuffer,
                                                                size_t xNewSizeBytes);

    #if ipconfigTCP_IP_SANITY

/*
 * Check if an address is a valid pointer to a network descriptor
 * by looking it up in the array of network descriptors
 */
        UBaseType_t bIsValidNetworkDescriptor( const NetworkBufferDescriptor_t * pxDesc );
        BaseType_t prvIsFreeBuffer( const NetworkBufferDescriptor_t * pxDescr );
    #endif

    #ifdef __cplusplus
        } /* extern "C" */
    #endif

#endif /* BUFFER_MANAGEMENT_H */
