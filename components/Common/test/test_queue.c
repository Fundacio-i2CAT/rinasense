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
    xQueueParams.xBlock = false;
    xQueueParams.unItemSz = sizeof(int);

    TEST_ASSERT(!ERR_CHK(xRsQueueInit("Basic Queue Test", &xQueue, &xQueueParams)));
    TEST_ASSERT(!ERR_CHK(xRsQueueSendToBack(&xQueue, &i1)));
    TEST_ASSERT(!ERR_CHK(xRsQueueSendToBack(&xQueue, &i2)));
    TEST_ASSERT(!ERR_CHK(xRsQueueSendToBack(&xQueue, &i3)));
    TEST_ASSERT(!ERR_CHK(xRsQueueReceive(&xQueue, (void **)&ii1)));
    TEST_ASSERT(!ERR_CHK(xRsQueueReceive(&xQueue, (void **)&ii2)));
    TEST_ASSERT(!ERR_CHK(xRsQueueReceive(&xQueue, (void **)&ii3)));

    TEST_ASSERT(ii1 == i1);
    TEST_ASSERT(ii2 == i2);
    TEST_ASSERT(ii3 == i3);

    RS_TEST_CASE_END(test_queue);
}

RS_TEST_CASE(BoundedQueue, "[queue]")
{
    RsQueue_t xQueue;
    RsQueueParams_t xQueueParams = {0};
    int i1 = 1, i2 = 2;
    int pi1, pi2;

    RS_TEST_CASE_BEGIN(test_queue);

    xQueueParams.unMaxItemCount = 1;
    xQueueParams.xBlock = false;
    xQueueParams.unItemSz = sizeof(int);

    TEST_ASSERT(!ERR_CHK(xRsQueueInit("Bounded Queue Test", &xQueue, &xQueueParams)));
    TEST_ASSERT(!ERR_CHK(xRsQueueSendToBack(&xQueue, &i1)));
    rsErr_t xStatus = xRsQueueSendToBack(&xQueue, &i2);
    TEST_ASSERT(ERR_IS(xStatus, ERR_OVERFLOW));
    TEST_ASSERT(!ERR_CHK(xRsQueueReceive(&xQueue, (void *)&pi1)));
    TEST_ASSERT(ERR_IS(xRsQueueReceive(&xQueue, (void *)&pi2), ERR_UNDERFLOW));

    RS_TEST_CASE_END(test_queue);
}

RS_TEST_CASE(BlockingQueue, "[queue]")
{
    RsQueue_t xQueue;
    RsQueueParams_t xQueueParams = {0};
    int i1 = 1;
    int pi1;

    RS_TEST_CASE_BEGIN(test_queue);

    xQueueParams.unMaxItemCount = 1;
    xQueueParams.xBlock = true;
    xQueueParams.unItemSz = sizeof(int);

    TEST_ASSERT(!ERR_CHK(xRsQueueInit("Blocking Queue Test", &xQueue, &xQueueParams)));
    TEST_ASSERT(!ERR_CHK(xRsQueueSendToBack(&xQueue, &i1)));

    /* Will not block since there is one item in the queue */
    TEST_ASSERT(!ERR_IS(xRsQueueReceiveTimed(&xQueue, &pi1, 100000), ERR_TIMEDOUT));

    /* Should block and return underflow */
    rsErr_t xStatus = xRsQueueReceiveTimed(&xQueue, &pi1, 1000000);
    vErrorLog("[test]", "Blocking Queue Test");
    TEST_ASSERT(ERR_IS(xStatus, ERR_TIMEDOUT));

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
