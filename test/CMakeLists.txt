cmake_minimum_required(VERSION 3.16)

if(NOT $ENV{IDF_PATH} STREQUAL "")
  # The IDF has its own version of Unity and its own way to use it.

  set(TEST_COMPONENTS
    "ARP826"
    "BufferManagement"
    "Common"
    "Portability"
    "ShimIPCP"
    CACHE STRING "List of components to test")

  # Include path for unity_config.h
  include_directories(${CMAKE_CURRENT_SOURCE_DIR})

  add_compile_definitions(configUSE_POSIX_ERRNO=1)

  set(ENV{TARGET_TYPE} "freertos_idf")
  set(TARGET_TYPE $ENV{TARGET_TYPE})

  set(EXTRA_COMPONENT_DIRS
    "../components/"
    "../components_mocks/")

  set(ENV{USE_MOCKS} "true")

  include($ENV{IDF_PATH}/tools/cmake/project.cmake)
  project(unit_test)
else()
  message(FATAL "This CMakeLists.txt file is to use with the ESP IDF only!")
endif()

