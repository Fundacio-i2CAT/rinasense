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

#ifndef TEST_CASE
#define NEEDS_MAIN
#define RS_TEST_CASE(name, desc) \
    void test_##name()
#else
#define RS_TEST_CASE(name, desc) \
    TEST_CASE(#name, desc)
#endif
