#include <stdio.h>
#include <stdint.h>
#include "BufferManagement.h"

int main() {
    size_t s = 10;
    uint8_t * p1, * p2, * p3;
    NetworkBufferDescriptor_t * pB1, * pB2;
    struct timespec ts = { 0, 0 };

    xNetworkBuffersInitialise();

    p1 = pucGetNetworkBuffer( &s );
    //vReleaseNetworkBuffer( p1 );

    p2 = pucGetNetworkBuffer( &s );
    //vReleaseNetworkBuffer( p2 );

    p3 = pucGetNetworkBuffer( &s );

    printf( "p1: %p -- p2: %p -- p3: %p\n", p1, p2, p3 );
    printf( "uxGetNumberOfFreeNetworkBuffers: %d\n", uxGetNumberOfFreeNetworkBuffers() );
    printf( "uxGetMinimumFreeNetworkBuffers: %d\n", uxGetMinimumFreeNetworkBuffers() );

    pB1 = pxGetNetworkBufferWithDescriptor( s, &ts );
    pB2 = pxGetNetworkBufferWithDescriptor( s, &ts );

    printf( "b1: %p -- b2: %p\n", pB1, pB2);
}
