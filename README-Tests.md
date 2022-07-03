# Anatomy of unit tests of the project

We have to jump through a few hoops to get the unit tests to work on
both ESP32 devices and on Linux. The [Unity test
system](http://www.throwtheswitch.org/unity) that the ESP-IDF
developers have picked comes with little overhead, which allows it to
be adapted to many devices, but,  it means it leaves much to the
implementer to do. The developers of the ESP-IDF project have
implemented a system which allows the test to be run on the device. On
Linux, while we have to use the same test files, we cannot make use of
the ESP32 specific code. We have a developed a simple set of macro
which behave differently depending if the code is being compiled for
ESP32 or Linux.

The following is an excerpt of an unit test file.

    #include "unity.h"
    #include "unity_fixups.h"

    RS_TEST_CASE_SETUP(test_lists) {
      /* Setup data structure for the test */
    }

    RS_TEST_CASE_TEARDOWN(test_lists) {
      /* Tear down what was setup for the test */
    }

Those declare per-test *setUp* and *tearDown* functions. The example
below is an example of test case.

    RS_TEST_CASE(TestName, "[list]")
    {
        RS_TEST_CASE_BEGIN(test_lists);

        TEST_ASSERT(...);
        ...

        RS_TEST_CASE_END(test_lists);
    }

Here `RS_TEST_CASE` declares a new test case. The first argument to
the macro is the name of the test *which should not be quoted*. The
second field is documented in the Unity include file as the
"description" of the test but the ESP-IDF Unity runner uses it as a
label which can be used to group tests together. There can be multiple
label, wrapped by `[..]` characters. The serial console runner of the
ESP-IDF environment allows tests to be run by using their label.

`RS_TEST_CASE_BEGIN` and `RS_TEST_CASE_END` are unfortunately
mandatory for the tests `setUp` and `tearDown` functions to be called
on Linux and ESP32. Absent this, the ESP32 IDF kit has no provision
for per-test `setUp` and `tearDown` functions, which are required on
Linux.

Finally, at the end of the program, a `main` function is required to
have the test run in Linux. This function needs to be guarded against
being compiled for the ESP32 by checking if the `TEST_CASE` macro is
defined. This macro is only defined when compiling with the ESP-IDF
build environment.

    #ifndef TEST_CASE
    int main() {
        UNITY_BEGIN();
        RS_RUN_TEST(TestName);
        RS_RUN_TEST(...);
        ...
        return UNITY_END();
    }
    #endif

The main function has to call the `UNITY_BEGIN` and `UNITY_END`
functions which are specific to Unity. They related to the gathering
of test results. The calls to `RS\_RUN\_TEST` run the test cases
individually. There is not yet any auto-registration of tests for
CMake based tests.

# Running tests in Linux

The unit test of the project, when it is compiled for Linux, are
integerated with the CMake CTest utility, which means the test can be
run as a group using `make test` after compilation. The test programs
are also compiled as individual binaires which can be run indepently
of each other. This is useful, for example, to dig the tests using
GDB, or check for memory leaks with Valgrind.

# On-device unit tests

Some component unit test can be run on the device as well as part of
the Linux build. This was made possible by the use of the Unity test
kit. already used within the ESP-IDF development kit. Unity is a
simple unit test library an header file that has been crafted to
usable on embedded devices.

The documentation of the ESP-IDF does not make it clear how to use the
internal unit test framework of the ESP-IDF but there is an [example
project](https://github.com/espressif/esp-idf/tree/master/examples/system/unit_test)
that demonstrates how to use it.

## Building the test image

The *test* directory is a separate project to the main project that
builds an image that can be uploaded on the device.

1. Starting from a fresh checkout, move to the *test* directory.

2. Either run `idf.py build` or `cmake ../CMakeLists.txt && make`

The latter is preferable seeing it has a color output which makes it
easier to see errors and warnings emitted by the compiler.

3. Flash on the device and check the serial output with:

    `$ idf.py flash monitor`

## Running the test

Code to the test runner is in `test/main/test_runner.c` but it simply
calls into the ESP-IDF-specific changes in Unity. On bootup, the test
runner will wait for user input through the serial console. The [menu
code](https://github.com/espressif/esp-idf/blob/master/components/unity/unity_runner.c#L268) offers a few ways to run tests.

- `*` followed by a carriage return will run all registered tests
- `[...]` will run all tests registered with a certain tag.
- `-` show last tests results
- `test name` will run a specific test by name
- Finally, tests can be run using their numbers

A plain carriage return display the menu again.

