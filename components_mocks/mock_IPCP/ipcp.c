#include <string.h>
#include <pthread.h>

#include "portability/port.h"
#include "portability/posix/semaphore.h"
#include "IPCP_events.h"
#include "IPCP_frames.h"

static RINAStackEvent_t sentEvent = { 0 };
static bool_t isCallingFromIPCPTask = false;

pthread_mutex_t evLock;
sem_t evSem;

#define TAG_MOCK_IPCP "mock-IPCP"

/* Mock IPCP public API */

bool_t xIsCallingFromIPCPTask(void) {
    return isCallingFromIPCPTask;
}

bool_t xSendEventToIPCPTask(eRINAEvent_t eEvent) {
    return true;
}

bool_t xSendEventStructToIPCPTask(const RINAStackEvent_t *pxEvent,
                                  struct timespec *uxTimeout)
{
    pthread_mutex_lock(&evLock);

    RsAssert(pxEvent->eEventType == eNetworkRxEvent ||
             pxEvent->eEventType == eNetworkTxEvent);

    sentEvent.eEventType = pxEvent->eEventType;
    sentEvent.pvData = pxEvent->pvData;

    sem_post(&evSem);

    LOGD(TAG_MOCK_IPCP, "Sending event: %d", pxEvent->eEventType);

    pthread_mutex_unlock(&evLock);

    return true;
}

/* Utility mock functions */

RINAStackEvent_t *pxMockGetLastSentEvent()
{
    struct timespec ts;

    if (clock_gettime(CLOCK_REALTIME, &ts) < 0)
        return NULL;

    ts.tv_sec += 1;

    if (sem_timedwait(&evSem, &ts) < 0) {
        LOGD(TAG_MOCK_IPCP, "pxMockGetLastSentEvent: timed out");
        return NULL;
    }

    return &sentEvent;
}

void vMockClearLastSentEvent()
{
    memset(&sentEvent, 0, sizeof(sentEvent));
}

void vMockSetIsCallingFromIPCPTask(bool_t v)
{
    isCallingFromIPCPTask = v;
}

/* Initialize the semaphores we'll use to wait on events. */
bool_t xMockIPCPInit() {
    if (pthread_mutex_init(&evLock, NULL) < 0)
        return false;
    if (sem_init(&evSem, 0, 0) < 0)
        return false;

    return true;
}

void vMockIPCPClean() {
    pthread_mutex_destroy(&evLock);
    sem_destroy(&evSem);
}

struct rmt_t *pxIPCPGetRmt(void)
{
    return NULL;
}

eFrameProcessingResult_t eConsiderFrameForProcessing(const uint8_t *const pucEthernetBuffer)
{
    return eProcessBuffer;
}
