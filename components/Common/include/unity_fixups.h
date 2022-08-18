/* This work around the lack of TEST_CASE macro in the base unity.h
 * header. The ESP test running mandates the use of this special macro
 * to get the list of test to run. In the Linux world, we do not need
 * to use this.
 *
 * This introduces a RS_TEST_CASE(name, desc) macro which is akin to
 * the ESP IDF TEST_CASE macro but for which the first argument must
 * not be a string. If there is no TEST_CASE macro, ie: in the Linux
 * build, we output a function returning nothing which is prefixed
 * with test_. The main() function of the individual test is not
 * required when building for the IDF so you can wrap it with #ifdef
 * TEST_CASE as I've did for the BufferManagement test code.
 *
 * #ifndef TEST_CASE
 * int main() {
 *     UNITY_BEGIN();
 *     RUN_TEST(test_testFn01);
 *     RUN_TEST(test_testFn02);
 *     ...
 *     return UNITY_END();
 * }
 * #endif */

#include <stdlib.h> /* For exit() below */

#define RS_TEST_CASE_SETUP(name) \
    void local_testcase_setUp_##name()

#define RS_TEST_CASE_TEARDOWN(name) \
    void local_testcase_tearDown_##name()

#define RS_TEST_CASE_BEGIN(name) \
    local_testcase_setUp_##name();

#define RS_TEST_CASE_END(name) \
    local_testcase_tearDown_##name();

#ifndef TEST_CASE /* We're running in a plain environment */
void setUp() {}
void tearDown() {}

#define RS_SUITE_BEGIN() \
    vRsLogInit(); \
    vRsLogSetLevel("*", LOG_VERBOSE); \
    UNITY_BEGIN()

#define RS_SUITE_END() \
    exit(UNITY_END());

#define RS_TEST_CASE(name, desc) \
    void test_##name()

#define RS_RUN_TEST(name) \
    RUN_TEST(test_##name);

#else /* We're running in the ESP-IDF */

#define RS_SUITE_BEGIN() goto _error_dont_use_this_;

#define RS_SUITE_END()  goto _error_dont_use_this_;

#define RS_TEST_CASE(name, desc) \
    TEST_CASE(#name, desc)

#define RS_RUN_TEST(name) \
    RUN_TEST(name); \

#endif
