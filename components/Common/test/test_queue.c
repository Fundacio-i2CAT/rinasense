#include <stdio.h>

#include "portability/port.h"

#include "common/error.h"
#include "common/queue.h"
#include "common/rinasense_errors.h"

#include "unity.h"
#include "common/unity_fixups.h"

RS_TEST_CASE_SETUP(test_queue) {}
RS_TEST_CASE_TEARDOWN(test_queue) {}

RS_TEST_CASE(BasicQueue, "[queue]")
{
    RsQueue_t xQueue;
    RsQueueParams_t xQueueParams = {0};
    int i1 = 1, i2 = 2, i3 = 3;
    int ii1 = 0, ii2 = 0, ii3 = 0;

    RS_TEST_CASE_BEGIN(test_queue);

    xQueueParams.unMaxItemCount = 0;
    xQueueParams.xNoBlock = true;
    xQueueParams.unItemSz = sizeof(int);

    TEST_ASSERT(!ERR_CHK(xQueueInit("Basic Queue Test", &xQueue, &xQueueParams)));
    TEST_ASSERT(!ERR_CHK(xQueueSendToBack(&xQueue, &i1)));
    TEST_ASSERT(!ERR_CHK(xQueueSendToBack(&xQueue, &i2)));
    TEST_ASSERT(!ERR_CHK(xQueueSendToBack(&xQueue, &i3)));
    TEST_ASSERT(!ERR_CHK(xQueueReceive(&xQueue, (void **)&ii1)));
    TEST_ASSERT(!ERR_CHK(xQueueReceive(&xQueue, (void **)&ii2)));
    TEST_ASSERT(!ERR_CHK(xQueueReceive(&xQueue, (void **)&ii3)));

    TEST_ASSERT(ii1 == i1);
    TEST_ASSERT(ii2 == i2);
    TEST_ASSERT(ii3 == i3);

    RS_TEST_CASE_END(test_queue);
}

RS_TEST_CASE(BoundedQueue, "[queue]")
{
    RsQueue_t xQueue;
    RsQueueParams_t xQueueParams = {0};
    int i1 = 1, i2 = 2, i3 = 4;
    int pi1, pi2;

    RS_TEST_CASE_BEGIN(test_queue);

    xQueueParams.unMaxItemCount = 1;
    xQueueParams.xNoBlock = true;
    xQueueParams.unItemSz = sizeof(int);

    TEST_ASSERT(!ERR_CHK(xQueueInit("Bounded Queue Test", &xQueue, &xQueueParams)));
    TEST_ASSERT(!ERR_CHK(xQueueSendToBack(&xQueue, &i1)));
    TEST_ASSERT(ERR_IS(xQueueSendToBack(&xQueue, &i2), ERR_OVERFLOW));
    TEST_ASSERT(!ERR_CHK(xQueueReceive(&xQueue, (void *)&pi1)));
    TEST_ASSERT(ERR_IS(xQueueReceive(&xQueue, (void *)&pi2), ERR_UNDERFLOW));

    RS_TEST_CASE_END(test_queue);
}

RS_TEST_CASE(BlockingQueue, "[queue]")
{
    RsQueue_t xQueue;
    RsQueueParams_t xQueueParams = {0};
    int i1 = 1, i2 = 2, i3 = 4;
    int pi1;

    RS_TEST_CASE_BEGIN(test_queue);

    xQueueParams.unMaxItemCount = 1;
    xQueueParams.xNoBlock = false;
    xQueueParams.unItemSz = sizeof(int);

    TEST_ASSERT(!ERR_CHK(xQueueInit("Blocking Queue Test", &xQueue, &xQueueParams)));
    TEST_ASSERT(!ERR_CHK(xQueueSendToBack(&xQueue, &i1)));

    /* Will not block since there is one item in the queue */
    TEST_ASSERT(!ERR_CHK(xQueueReceiveTimed(&xQueue, &pi1, 100000)));

    /* Should block */
    TEST_ASSERT(ERR_IS(xQueueReceiveTimed(&xQueue, &pi1, 1000000), ERR_TIMEOUT));

    RS_TEST_CASE_END(test_queue);
}

#ifndef TEST_CASE
int main() {
    RS_SUITE_BEGIN();
    RS_RUN_TEST(BasicQueue);
    RS_RUN_TEST(BoundedQueue);
    RS_RUN_TEST(BlockingQueue);
    RS_SUITE_END();
}
#endif
