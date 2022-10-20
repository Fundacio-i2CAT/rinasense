#include <stdio.h>

#include "common/simple_queue.h"

#include "unity.h"
#include "common/unity_fixups.h"

RS_TEST_CASE(BasicQueue, "[queue]")
{
    RsSimpleQueue_t xQueue;
    int i1 = 20, i2 = 30;
    int *pint;

    TEST_ASSERT(xSimpleQueueInit("Test_Queue", &xQueue));
    TEST_ASSERT(xSimpleQueueSendToBack(&xQueue, &i1));
    TEST_ASSERT(xSimpleQueueSendToBack(&xQueue, &i2));
    TEST_ASSERT(unSimpleQueueLength(&xQueue) == 2);
    pint = pvSimpleQueueReceive(&xQueue);
    TEST_ASSERT(*pint == 20);
    TEST_ASSERT(unSimpleQueueLength(&xQueue) == 1);
    pint = pvSimpleQueueReceive(&xQueue);
    TEST_ASSERT(*pint == 30);
    TEST_ASSERT(unSimpleQueueLength(&xQueue) == 0);
    pint = pvSimpleQueueReceive(&xQueue);
    TEST_ASSERT(pint == NULL);
}

#ifndef TEST_CASE
int main() {
    RS_SUITE_BEGIN();
    RS_RUN_TEST(BasicQueue);
    RS_SUITE_END();
}

#endif
