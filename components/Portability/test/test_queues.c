#include <errno.h>
#include <time.h>

#include "portability/port.h"

#include "unity.h"
#include "unity_fixups.h"

#ifndef TEST_CASE
void setUp() {}
void tearDown() {}
#endif

static void inNSeconds(int n, struct timespec *ts)
{
    int r;

    /* Don't ask. Debugging something. */
    TEST_ASSERT_NOT_NULL(ts);

    r = clock_gettime(CLOCK_REALTIME, ts);
    TEST_ASSERT_EQUAL_INT(0, errno);
    TEST_ASSERT_EQUAL_INT(0, r);

    ts->tv_sec += n;
}

RS_TEST_CASE(CreateDestroy, "Queues - Creation/Destruction")
{
    RsQueue_t *q;

    TEST_ASSERT((q = pxRsQueueCreate("testCreateDestroy", 2, sizeof(uint32_t))) != NULL);
    vRsQueueDelete(q);
}

RS_TEST_CASE(AddRead, "Queues - Add & Read")
{
    RsQueue_t *q;
    uint32_t i1 = 1, i2 = 2;
    uint32_t j1, j2;
    struct timespec ts;

    TEST_ASSERT((q = pxRsQueueCreate("testAddRead", 2, sizeof(uint32_t))) != NULL);

    inNSeconds(1, &ts);
    TEST_ASSERT(xRsQueueSendToBack(q, &i1, &ts));
    TEST_ASSERT(xRsQueueSendToBack(q, &i2, &ts));

    inNSeconds(1, &ts);
    TEST_ASSERT(xRsQueueReceive(q, &j1, &ts));
    TEST_ASSERT(xRsQueueReceive(q, &j2, &ts));
    TEST_ASSERT(j1 == i1);
    TEST_ASSERT(j2 == i2);

    vRsQueueDelete(q);
}

#ifndef TEST_CASE
int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_CreateDestroy);
    RUN_TEST(test_AddRead);
    return UNITY_END();
}
#endif
