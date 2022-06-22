#include <time.h>

#include "portability/port.h"

static void inNSeconds(int n, struct timespec *ts)
{
    RsAssert(!clock_gettime(CLOCK_REALTIME, ts));
    ts->tv_sec += n;
}

void testCreateDestroy()
{
    RsQueue_t *q;

    RsAssert((q = pxRsQueueCreate("testCreateDestroy", 2, sizeof(uint32_t))) != NULL);
    vRsQueueDelete(q);
}

void testAddRead()
{
    RsQueue_t *q;
    uint32_t i1 = 1, i2 = 2;
    uint32_t j1, j2;
    struct timespec ts;

    RsAssert((q = pxRsQueueCreate("testAddRead", 2, sizeof(uint32_t))) != NULL);

    inNSeconds(1, &ts);
    RsAssert(xRsQueueSendToBack(q, &i1, &ts));
    RsAssert(xRsQueueSendToBack(q, &i2, &ts));

    inNSeconds(1, &ts);
    RsAssert(xRsQueueReceive(q, &j1, &ts));
    RsAssert(xRsQueueReceive(q, &j2, &ts));
    RsAssert(j1 == i1);
    RsAssert(j2 == i2);

    vRsQueueDelete(q);
}

int main()
{
    testCreateDestroy();
    testAddRead();
}
