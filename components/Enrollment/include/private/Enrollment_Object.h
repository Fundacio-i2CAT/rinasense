#ifndef _ENROLLMENT_PRIVATE_ENROLLMENT_OBJECT_H_INCLUDED
#define _ENROLLMENT_PRIVATE_ENROLLMENT_OBJECT_H_INCLUDED

#include <pthread.h>

#include "portability/port.h"

#include "Ribd_defs.h"
#include "Enrollment_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* This is a private data structure for the enrollment RIB object. */
typedef struct {
    pthread_t xEnrollmentThread;

    /* RIB daemon object */
    Ribd_t *pxRibd;

    /* Enrollment object */
    Enrollment_t *pxEnrollment;

    ribObject_t *pxThis;

    union {
        /* Data specific to inbound enrollment */
        struct {
            portId_t unPort;
            invokeId_t unStartInvokeId;
        };

        /* Data specific to outbound enrollment */
        /* ... */
    };

} enrollmentObjectData_t;

void *xEnrollmentInboundProcess(void *pxThis);

#ifdef __cplusplus
}
#endif

#endif /* _ENROLLMENT_PRIVATE_ENROLLMENT_OBJECT_H_INCLUDED */
