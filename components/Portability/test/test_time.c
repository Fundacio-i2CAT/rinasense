#include <stdio.h>
#include "portability/port.h"

#include "unity.h"
#include "common/unity_fixups.h"

RS_TEST_CASE_SETUP(test_time) {}
RS_TEST_CASE_TEARDOWN(test_time) {}

RS_TEST_CASE(TimeOut, "[time]")
{
    struct RsTimeOut xTimeOut;
    useconds_t xTimeLeft;
    bool_t xTimeDone;

    xTimeLeft = 100;
    xTimeDone = false;
    RS_TEST_CASE_BEGIN(test_time);

    TEST_ASSERT(xRsTimeSetTimeOut(&xTimeOut));
    TEST_ASSERT(xRsTimeCheckTimeOut(&xTimeOut, &xTimeLeft));
    usleep(50);
    TEST_ASSERT(xRsTimeCheckTimeOut(&xTimeOut, &xTimeLeft));
    TEST_ASSERT(xTimeLeft <= 100);
    usleep(50);
    TEST_ASSERT(xRsTimeCheckTimeOut(&xTimeOut, &xTimeLeft));
    TEST_ASSERT(xTimeLeft == 0);

    RS_TEST_CASE_END(test_time);
}

#ifndef TEST_CASE
int main() {
    UNITY_BEGIN();
    RS_RUN_TEST(TimeOut);
    return UNITY_END();
}
#endif
