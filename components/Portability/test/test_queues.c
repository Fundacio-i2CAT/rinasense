#include <errno.h>
#include <time.h>

#include "portability/port.h"

#include "unity.h"
#include "unity_fixups.h"

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
    struct timespec ts;

    RS_TEST_CASE_BEGIN(test_queues);

    TEST_ASSERT((q = pxRsQueueCreate("testAddRead", 2, sizeof(uint32_t))) != NULL);

    TEST_ASSERT(xRsQueueSendToBack(q, &i1, 1000));
    TEST_ASSERT(xRsQueueSendToBack(q, &i2, 1000));

    TEST_ASSERT(xRsQueueReceive(q, &j1, 1000));
    TEST_ASSERT(xRsQueueReceive(q, &j2, 1000));
    TEST_ASSERT(j1 == i1);
    TEST_ASSERT(j2 == i2);

    vRsQueueDelete(q);

    RS_TEST_CASE_END(test_queues);
}

#ifndef TEST_CASE
int main()
{
    UNITY_BEGIN();
    RS_RUN_TEST(CreateDestroy);
    RS_RUN_TEST(AddRead);
    return UNITY_END();
}
#endif
