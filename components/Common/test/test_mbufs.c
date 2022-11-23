#include "portability/port.h"
#include "common/mbuf.h"

#include "unity.h"
#include "common/unity_fixups.h"

RS_TEST_CASE_SETUP(test_mbufs) {}
RS_TEST_CASE_TEARDOWN(test_mbufs) {}

RS_TEST_CASE(MbufSimple, "[mbuf]")
{
    mbuf_t *m;

    
}

#ifndef TEST_CASE
int main() {
    RS_SUITE_BEGIN();
    RS_RUN_TEST(MbufSimple);
    RS_SUITE_END();
}
#endif
