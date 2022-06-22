# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(TARGET_TYPE "freertos_idf"
  CACHE STRING "Type of build: 'linux_glibc', 'freertos_idf'")

if("${TARGET_TYPE}" STREQUAL "" OR "${TARGET_TYPE}" STREQUAL "freertos_idf")
  if (NOT "$ENV{IDF_PATH}" STREQUAL "")
    # Set a default target type.
    if("${TARGET_TYPE}" STREQUAL "")
      set(TARGET_TYPE "freertos_idf")
    endif()

    message("Configuring: ESP-IDF build")
    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    add_compile_definitions(configUSE_POSIX_ERRNO=1)
    project(RINA_sensor)
  else()
    message(FATAL_ERROR "The 'freertos_idf' target needs the IDF_PATH environment variable to be defined")
  endif()
elseif (${TARGET_TYPE} STREQUAL "linux_glib")
  project(RINA_sensor)

  message("Configuring: POSIX build")

  include(CTest)

  add_compile_definitions(configUSE_POSIX_ERRNO=1)

  set(CMAKE_VERBOSE_MAKEFILE ON)
  #set(CMAKE_C_COMPILER "/ugcc")

  #
  # Set all include path variables here so as to avoid circular
  # dependencies problem below.
  #
  set(configSensor_INCLUDES     "${CMAKE_CURRENT_SOURCE_DIR}/components/configSensor/include")
  set(Common_INCLUDES           "${CMAKE_CURRENT_SOURCE_DIR}/components/Common/include")
  set(ARP826_INCLUDES           "${CMAKE_CURRENT_SOURCE_DIR}/components/ARP826/include")
  set(BufferManagement_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/components/BufferManagement/include")
  set(IPCP_INCLUDES             "${CMAKE_CURRENT_SOURCE_DIR}/components/IPCP/include")
  set(NetworkInterface_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/components/NetworkInterface/include")
  set(NetworkInterface_MQ_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/components/NetworkInterface/mq/include")

  set(mock_IPCP_INCLUDES        "${CMAKE_CURRENT_SOURCE_DIR}/components/Mocks/IPCP/include")

  #
  # Portability layer, target specific
  #
  include("${CMAKE_CURRENT_SOURCE_DIR}/components/Portability/${TARGET_TYPE}/CMakeLists.txt")

  #
  # Portability layer tests
  #
  include("${CMAKE_CURRENT_SOURCE_DIR}/components/Portability/test/CMakeLists.txt")

  #
  # Mock objects
  #
  include("${CMAKE_CURRENT_SOURCE_DIR}/components/Mocks/CMakeLists.txt")

  #
  # Component: configSensor
  #
  file(GLOB configSensor_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/components/configSensor/*.c"
  )
  add_library(configSensor STATIC ${configSensor_SOURCES})
  target_include_directories(configSensor PUBLIC ${configSensor_INCLUDES})
  set_target_properties(configSensor PROPERTIES EXCLUDE_FROM_ALL TRUE)

  #
  # Component: Common
  #
  set(Common_DIR "${CMAKE_CURRENT_SOURCE_DIR}/components/Common")
  set(Common_TEST_DIR "${Common_DIR}/test")
  file(GLOB Common_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/components/Common/*.c"
  )
  add_library(Common STATIC ${Common_SOURCES})
  target_include_directories(Common PUBLIC
    ${Common_INCLUDES}
    ${Portability_INCLUDES}
    ${configSensor_INCLUDES}
  )
  include("${Common_TEST_DIR}/CMakeLists.txt")

  #
  # Component: BufferManagement
  #
  set(BufferManagement_DIR "${CMAKE_CURRENT_SOURCE_DIR}/components/BufferManagement")
  set(BufferManagement_TEST_DIR "${BufferManagement_DIR}/test")
  file(GLOB BufferManagement_SOURCES
    "${BufferManagement_DIR}/*.c"
  )
  add_library(BufferManagement STATIC ${BufferManagement_SOURCES})
  target_include_directories(BufferManagement PUBLIC
    ${BufferManagement_INCLUDES}
    ${configSensor_INCLUDES}
    ${ARP826_INCLUDES}
    ${Common_INCLUDES}
    #    ${EFCP_INCLUDES}
    ${IPCP_INCLUDES}
    #    ${NetworkInterface_INCLUDES}
    #    ${Rmt_INCLUDES}
    #    ${ShimIPCP_INCLUDES}
    ${Portability_INCLUDES}
  )
  include("${BufferManagement_TEST_DIR}/CMakeLists.txt")

  #
  # Component: NetworkInterface
  #
  # FIXME: The 'mq' NetworkInterface is for testing only. We need to
  # figure out how to support multiple type of NetworkInterface.
  set(NetworkInterface_DIR "${CMAKE_CURRENT_SOURCE_DIR}/components/NetworkInterface")
  file(GLOB NetworkInterface_SOURCES
    "${NetworkInterface_DIR}/mq/*.c"
  )
  add_library(NetworkInterface STATIC ${NetworkInterface_SOURCES})
  target_include_directories(NetworkInterface PUBLIC
    ${Common_INCLUDES}
    ${configRINA_INCLUDES}
    ${BufferManagement_INCLUDES}
    ${NetworkInterface_INCLUDES}
    ${NetworkInterface_MQ_INCLUDES}
    ${ARP826_INCLUDES}
    ${IPCP_INCLUDES}
    ${configSensor_INCLUDES}
    ${Rmt_INCLUDES}
    ${ShimIPCP_INCLUDES}
    ${EFCP_INCLUDES}
    ${Portability_INCLUDES}
  )
  include("${NetworkInterface_DIR}/mq/test/CMakeLists.txt")

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb3 -O0 -include stdbool.h")

  add_subdirectory(unity)

else()
  message("Unknown target ${TARGET_TYPE}")
endif()
