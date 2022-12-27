#include <pthread.h>

#include "Enrollment_api.h"

neighborInfo_t *pxEnrollmentFindNeighbor(Enrollment_t *pxEnrollment, string_t pcRemoteApName)
{
    num_t x = 0;
    neighborInfo_t *pxNeighbor;

    LOGI(TAG_ENROLLMENT, "Looking for '%s'", pcRemoteApName);

    pthread_mutex_lock(&pxEnrollment->xNeighborMutex);

    for (x = 0; x < NEIGHBOR_TABLE_SIZE; x++) {
        if (pxEnrollment->xNeighborsTable[x].xValid == true) {
            pxNeighbor = pxEnrollment->xNeighborsTable[x].pxNeighborInfo;

            LOGI(TAG_ENROLLMENT, "Neighbor checking '%p'", pxNeighbor);
            LOGI(TAG_ENROLLMENT, "Comparing '%s' - '%s'", pxNeighbor->pcApName, pcRemoteApName);

            if (!strcmp(pxNeighbor->pcApName, pcRemoteApName)) {
                LOGI(TAG_ENROLLMENT, "Neighbor '%p' found", pxNeighbor);

                pthread_mutex_unlock(&pxEnrollment->xNeighborMutex);

                return pxNeighbor;
            }
        }
    }

    pthread_mutex_unlock(&pxEnrollment->xNeighborMutex);

    LOGW(TAG_ENROLLMENT, "Neighbor '%s' not found", pcRemoteApName);

    return NULL;
}

bool_t xEnrollmentAddNeighborEntry(Enrollment_t *pxEnrollment, neighborInfo_t *pxNeighbor)
{
    num_t x = 0;

    pthread_mutex_lock(&pxEnrollment->xNeighborMutex);

    for (x = 0; x < NEIGHBOR_TABLE_SIZE; x++) {

        if (pxEnrollment->xNeighborsTable[x].xValid == false) {
            pxEnrollment->xNeighborsTable[x].pxNeighborInfo = pxNeighbor;
            pxEnrollment->xNeighborsTable[x].xValid = true;

            pthread_mutex_unlock(&pxEnrollment->xNeighborMutex);

            return true;
        }
    }

    pthread_mutex_unlock(&pxEnrollment->xNeighborMutex);

    return false;
}

address_t xEnrollmentGetNeighborAddress(Enrollment_t *pxEnrollment, string_t pcRemoteApName)
{
    num_t x;
    neighborInfo_t *pxNeighbor;

    RsAssert(pxEnrollment);
    RsAssert(pcRemoteApName);

    LOGI(TAG_ENROLLMENT, "Looking for '%s'", pcRemoteApName);

    pthread_mutex_lock(&pxEnrollment->xNeighborMutex);

    for (x = 0; x < NEIGHBOR_TABLE_SIZE; x++) {
        if (pxEnrollment->xNeighborsTable[x].xValid == true) {
            pxNeighbor = pxEnrollment->xNeighborsTable[x].pxNeighborInfo;

            if (!strcmp(pxNeighbor->pcApName, pcRemoteApName)) {
                pthread_mutex_unlock(&pxEnrollment->xNeighborMutex);

                return pxNeighbor->xNeighborAddress;
            }
        }
    }

    pthread_mutex_unlock(&pxEnrollment->xNeighborMutex);

    LOGI(TAG_ENROLLMENT, "Neighbor not found");

    return ADDRESS_WRONG; // should be zero but for testing purposes.
}

neighborInfo_t *pxEnrollmentCreateNeighInfo(Enrollment_t *pxEnrollment, string_t pcApName, portId_t xN1Port)
{
    neighborInfo_t *pxNeighInfo;

    if (!(pxNeighInfo = pvRsMemAlloc(sizeof(*pxNeighInfo)))) {
        LOGE(TAG_ENROLLMENT, "Failed to allocate memory for neighbor information");
        return NULL;
    }

    pxNeighInfo->pcApName = strdup(pcApName);
    pxNeighInfo->eEnrollmentState = eENROLLMENT_NONE;
    pxNeighInfo->xNeighborAddress = -1;
    pxNeighInfo->xN1Port = xN1Port;
    pxNeighInfo->pcToken = NULL;

    if (!xEnrollmentAddNeighborEntry(pxEnrollment, pxNeighInfo)) {
        LOGE(TAG_ENROLLMENT, "Failed to add the neighbor to the table");
        vRsMemFree(pxNeighInfo);
        return NULL;
    }
    else LOGI(TAG_ENROLLMENT, "Neighbor added: %s", pcApName);

    return pxNeighInfo;
}
