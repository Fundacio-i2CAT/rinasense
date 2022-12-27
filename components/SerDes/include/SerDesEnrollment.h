#ifndef SERDES_ENROLLMENT_H_INCLUDED
#define SERDES_ENROLLMENT_H_INCLUDED

#include "portability/port.h"
#include "common/rsrc.h"
#include "common/error.h"

#include "SerDes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    rsrcPoolP_t xPoolEnc;
    rsrcPoolP_t xPoolDec;

    pthread_mutex_t xPoolEncMutex;

    pthread_mutex_t xPoolDecMutex;

} EnrollmentSerDes_t;

typedef struct
{
    /*Address*/
    uint64_t ullAddress;

    string_t pcSupportingDifs;

    bool_t xStartEarly;

    string_t pcToken;

} enrollmentMessage_t;

rsMemErr_t xSerDesEnrollmentInit(EnrollmentSerDes_t *pxSD);

serObjectValue_t *pxSerDesEnrollmentEncode(EnrollmentSerDes_t *pxSD, enrollmentMessage_t *pxMsg);

enrollmentMessage_t *pxSerDesEnrollmentDecode(EnrollmentSerDes_t *pxSD,
                                              uint8_t *pucBuffer, size_t xMessageLength);

#ifdef __cplusplus
}
#endif

#endif /* SERDES_ENROLLMENT_H_INCLUDED */
