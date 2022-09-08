#include <errno.h>
#include <time.h>

#include "portability/port.h"

#include "portability/rsqueue.h"
#include "unity.h"
#include "common/unity_fixups.h"

RS_TEST_CASE_SETUP(test_queues) {}
RS_TEST_CASE_TEARDOWN(test_queues) {}

RS_TEST_CASE(CreateDestroy, "[queues]")
{
    RsQueue_t *q;

    RS_TEST_CASE_BEGIN(test_queues);

    TEST_ASSERT((q = pxRsQueueCreate("testCreateDestroy", 2, sizeof(uint32_t))) != NULL);
    vRsQueueDelete(q);

    RS_TEST_CASE_END(test_queues);
}

RS_TEST_CASE(AddRead, "[queues]")
{
    RsQueue_t *q;
    uint32_t i1 = 1, i2 = 2;
    uint32_t j1, j2;

    RS_TEST_CASE_BEGIN(test_queues);

    TEST_ASSERT((q = pxRsQueueCreate("testAddRead", 2, sizeof(uint32_t))) != NULL);

    /* Tests that we can send simple integers to the queue. */
    TEST_ASSERT(xRsQueueSendToBack(q, &i1, sizeof(i1), 1000));
    TEST_ASSERT(xRsQueueSendToBack(q, &i2, sizeof(i2), 1000));

    /* Tests that we can read those same integers. */
    TEST_ASSERT(xRsQueueReceive(q, &j1, sizeof(j1), 1000));
    TEST_ASSERT(xRsQueueReceive(q, &j2, sizeof(j2), 1000));
    TEST_ASSERT(j1 == i1);
    TEST_ASSERT(j2 == i2);

    vRsQueueDelete(q);

    RS_TEST_CASE_END(test_queues);
}

RS_TEST_CASE(SizeErrors, "[queues]")
{
    RsQueue_t *q;
    uint32_t i1 = 1;
    uint32_t j1;

    RS_TEST_CASE_BEGIN(test_queues);

    TEST_ASSERT((q = pxRsQueueCreate("testSizeErrors", 2, sizeof(uint32_t))) != NULL);

    /* Too much data sent, should fail. */
    TEST_ASSERT(!xRsQueueSendToBack(q, &i1, sizeof(i1) + 5, 1000));

    /* Send just an interger */
    TEST_ASSERT(xRsQueueSendToBack(q, &j1, sizeof(j1), 1000));

    /* Not enough space in target buffer, should fail */
    TEST_ASSERT(!xRsQueueReceive(q, &j1, sizeof(j1) - 1, 1000));

    vRsQueueDelete(q);

    RS_TEST_CASE_END(test_queues);
}

#ifndef TEST_CASE
int main()
{
    UNITY_BEGIN();
    RS_RUN_TEST(CreateDestroy);
    RS_RUN_TEST(AddRead);
    RS_RUN_TEST(SizeErrors);
    return UNITY_END();
}
#endif
