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
  set(CdapProto_INCLUDES        "${CMAKE_CURRENT_SOURCE_DIR}/components/CdapProto/include")
  set(configSensor_INCLUDES     "${CMAKE_CURRENT_SOURCE_DIR}/components/configSensor/include")
  set(configRINA_INCLUDES       "${CMAKE_CURRENT_SOURCE_DIR}/components/configRINA/include")
  set(Common_INCLUDES           "${CMAKE_CURRENT_SOURCE_DIR}/components/Common/include")
  set(ARP826_INCLUDES           "${CMAKE_CURRENT_SOURCE_DIR}/components/ARP826/include")
  set(BufferManagement_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/components/BufferManagement/include")
  set(EFCP_INCLUDES             "${CMAKE_CURRENT_SOURCE_DIR}/components/EFCP/include")
  set(IPCP_INCLUDES             "${CMAKE_CURRENT_SOURCE_DIR}/components/IPCP/include")
  set(NetworkInterface_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/components/NetworkInterface/include")
  set(NetworkInterface_MQ_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/components/NetworkInterface/mq/include")
  set(RINA_API_INCLUDES         "${CMAKE_CURRENT_SOURCE_DIR}/components/RINA_API/include")
  set(Rmt_INCLUDES              "${CMAKE_CURRENT_SOURCE_DIR}/components/Rmt/include")
  set(ShimIPCP_INCLUDES         "${CMAKE_CURRENT_SOURCE_DIR}/components/ShimIPCP/include")
  set(FlowAllocator_INCLUDES    "${CMAKE_CURRENT_SOURCE_DIR}/components/FlowAllocator/include")
  set(Ribd_INCLUDES             "${CMAKE_CURRENT_SOURCE_DIR}/components/Ribd/include")
  set(Enrollment_INCLUDES       "${CMAKE_CURRENT_SOURCE_DIR}/components/Enrollment/include")

  set(mock_IPCP_INCLUDES        "${CMAKE_CURRENT_SOURCE_DIR}/components_mocks/mock_IPCP/include")
  set(mock_NetworkInterface_INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}/components_mocks/mock_NetworkInterface/include")
  set(mock_FlowAllocator_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/components_mocks/mock_FlowAllocator/include")
  set(mock_Ribd_INCLUDES        "${CMAKE_CURRENT_SOURCE_DIR}/components_mocks/mock_Ribd/include")

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
  # FIXME: THIS IS BORING AND REPETITIVE, MOVE THAT ELSEWHERE!
  #
  set(mock_NetworkInterface_INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}/components_mocks/mock_NetworkInterface/include")
  file(GLOB mock_NetworkInterface_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/components_mocks/mock_NetworkInterface/*.c"
  )
  add_library(mock_NetworkInterface STATIC ${mock_NetworkInterface_SOURCES})
  target_include_directories(mock_NetworkInterface PUBLIC
    ${BufferManagement_INCLUDES}
    ${Portability_INCLUDES}
    ${IPCP_INCLUDES}
    ${Common_INCLUDES}
    ${configSensor_INCLUDES}
  )

  file(GLOB mock_IPCP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/components_mocks/mock_IPCP/*.c"
  )
  add_library(mock_IPCP STATIC ${mock_IPCP_SOURCES})
  target_include_directories(mock_IPCP PUBLIC
    ${BufferManagement_INCLUDES}
    ${Portability_INCLUDES}
    ${IPCP_INCLUDES}
  )

  file(GLOB mock_RIBD_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/components_mocks/mock_Ribd/*.c"
  )
  add_library(mock_Ribd STATIC ${mock_RIBD_SOURCES})
  target_include_directories(mock_Ribd PUBLIC
    ${configSensor_INCLUDES}
    ${Common_INCLUDES}
    ${EFCP_INCLUDES}
    ${BufferManagement_INCLUDES}
    ${Portability_INCLUDES}
    ${IPCP_INCLUDES}
    ${Rmt_INCLUDES}
  )

  file(GLOB mock_FlowAllocator_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/components_mocks/mock_FlowAllocator/*.c"
  )
  add_library(mock_FlowAllocator STATIC ${mock_FlowAllocator_SOURCES})
  target_include_directories(mock_FlowAllocator PUBLIC
    ${configSensor_INCLUDES}
    ${Common_INCLUDES}
    ${EFCP_INCLUDES}
    ${BufferManagement_INCLUDES}
    ${Portability_INCLUDES}
    ${IPCP_INCLUDES}
    ${Rmt_INCLUDES}
    ${RINA_API_INCLUDES}
  )

  file(GLOB mock_EFCP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/components_mocks/mock_EFCP/*.c"
  )
  add_library(mock_EFCP STATIC ${mock_EFCP_SOURCES})
  target_include_directories(mock_EFCP PUBLIC
    ${configSensor_INCLUDES}
    ${Common_INCLUDES}
    ${EFCP_INCLUDES}
    ${BufferManagement_INCLUDES}
    ${Portability_INCLUDES}
    ${IPCP_INCLUDES}
    ${Rmt_INCLUDES}
    ${RINA_API_INCLUDES}
  )

  #
  # Component: CDAP
  #
  file(GLOB CdapProto_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/components/CdapProto/*.c"
  )
  add_library(CdapProto STATIC ${CdapProto_SOURCES})
  target_include_directories(CdapProto PUBLIC ${CdapProto_INCLUDES})
  set_target_properties(CdapProto PROPERTIES EXCLUDE_FROM_ALL TRUE)

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
    ${mock_IPCP_INCLUDES}
    ${IPCP_INCLUDES}
    ${configSensor_INCLUDES}
    ${Rmt_INCLUDES}
    ${ShimIPCP_INCLUDES}
    ${EFCP_INCLUDES}
    ${Portability_INCLUDES}
  )
  include("${NetworkInterface_DIR}/test/CMakeLists.txt")

  # Component: ARP826
  #
  set(ARP826_DIR "${CMAKE_CURRENT_SOURCE_DIR}/components/ARP826")
  set(ARP826_TEST_DIR "${ARP826_DIR}/test")
  file(GLOB ARP826_SOURCES "${ARP826_DIR}/*.c")
  add_library(ARP826 STATIC ${ARP826_SOURCES})
  target_include_directories(ARP826 PUBLIC
    ${configSensor_INCLUDES}
    ${NetworkInterface_INCLUDES}
    ${Common_INCLUDES}
    ${BufferManagement_INCLUDES}
    ${ARP826_INCLUDES}
    ${mock_IPCP_INCLUDES}
    ${IPCP_INCLUDES}
    ${Portability_INCLUDES}
    ${ShimIPCP_INCLUDES}
  )
  include("${ARP826_TEST_DIR}/CMakeLists.txt")

  #
  # Component: Rmt
  #
  file(GLOB Rmt_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/components/Rmt/*.c"
  )
  add_library(Rmt STATIC ${Rmt_SOURCES})
  target_include_directories(Rmt PUBLIC
    ${configSensor_INCLUDES}
    ${Common_INCLUDES}
    ${ARP826_INCLUDES}
    ${BufferManagement_INCLUDES}
    ${Rmt_INCLUDES}
    ${EFCP_INCLUDES}
    ${Portability_INCLUDES}
    ${IPCP_INCLUDES}
  )

  #
  # Component: EFCP
  #
  file(GLOB EFCP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/components/EFCP/*.c"
  )
  add_library(EFCP STATIC ${EFCP_SOURCES})
  target_include_directories(EFCP PUBLIC
    ${Portability_INCLUDES}
    ${Common_INCLUDES}
    ${ARP826_INCLUDES}
    ${IPCP_INCLUDES}
    ${EFCP_INCLUDES}
    ${Rmt_INCLUDES}
    ${configSensor_INCLUDES}
  )

  #
  # Component: ShimIPCP
  #
  set(ShimIPCP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/components/ShimIPCP")
  set(ShimIPCP_TESTS_DIR "${ShimIPCP_DIR}/test")
  file(GLOB ShimIPCP_SOURCES "${ShimIPCP_DIR}/*.c")
  add_library(ShimIPCP STATIC ${ShimIPCP_SOURCES})
  target_include_directories(ShimIPCP PUBLIC
    ${ShimIPCP_INCLUDES}
    ${Common_INCLUDES}
    ${Portability_INCLUDES}
    ${configSensor_INCLUDES}
    ${configRINA_INCLUDES}
    ${ARP826_INCLUDES}
    ${BufferManagement_INCLUDES}
    ${EFCP_INCLUDES}
    ${mock_IPCP_INCLUDES}
    ${IPCP_INCLUDES}
    ${NetworkInterface_INCLUDES}
    ${Rmt_INCLUDES}
  )
  include("${ShimIPCP_TESTS_DIR}/CMakeLists.txt")

  #
  # Component: IPCP
  #
  set(IPCP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/components/IPCP")
  set(IPCP_TEST_DIR "${IPCP_DIR}/test")
  set(IPCP_SOURCES "${IPCP_DIR}/IpcManager.c" "${IPCP_DIR}/normalIPCP.c" "${IPCP_DIR}/common.c")
  add_library(IPCP STATIC ${IPCP_SOURCES})
  target_include_directories(IPCP PUBLIC
    # Temporary
    ${mock_IPCP_INCLUDES}
    ${mock_FlowAllocator_INCLUDES}
    ${mock_Ribd_INCLUDES}

    ${configSensor_INCLUDES}
    ${configRINA_INCLUDES}
    ${ARP826_INCLUDES}
    ${NetworkInterface_INCLUDES}
    ${ShimIPCP_INCLUDES}
    ${BufferManagement_INCLUDES}
    ${IPCP_INCLUDES}
    ${Rmt_INCLUDES}
    ${EFCP_INCLUDES}
    ${Portability_INCLUDES}
    ${Common_INCLUDES}
    ${RINA_API_INCLUDES}
  )

  #
  # Component: Ribd
  #
  set(Ribd_DIR "${CMAKE_CURRENT_SOURCE_DIR}/components/Ribd")
  set(Ribd_TEST_DIR "${Ribd_DIR}/test")
  file(GLOB Ribd_SOURCES "${Ribd_DIR}/*.c")
  add_library(Ribd STATIC ${Ribd_SOURCES})
  target_include_directories(Ribd PUBLIC
    # Temporary
    ${mock_IPCP_INCLUDES}

    ${CdapProto_INCLUDES}
    ${BufferManagement_INCLUDES}
    ${configRINA_INCLUDES}
    ${configSensor_INCLUDES}
    ${IPCP_INCLUDES}
    ${Ribd_INCLUDES}
    ${Portability_INCLUDES}
    ${Common_INCLUDES}
    ${Enrollment_INCLUDES}
    ${EFCP_INCLUDES}
    ${Rmt_INCLUDES}
    ${RINA_API_INCLUDES}
    ${FlowAllocator_INCLUDES}
  )
  include("${Ribd_TEST_DIR}/CMakeLists.txt")

  #
  # Component: FlowAllocator
  #
  file(GLOB FlowAllocator_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/components/FlowAllocator/*.c"
  )
  add_library(FlowAllocator STATIC ${FlowAllocator_SOURCES})
  target_include_directories(FlowAllocator PUBLIC
    ${FlowAllocator_INCLUDES}
    ${configSensor_INCLUDES}
    ${configRINA_INCLUDES}
    ${Common_INCLUDES}
    ${Portability_INCLUDES}
    ${EFCP_INCLUDES}
    ${ARP826_INCLUDES}
    ${CdapProto_INCLUDES}
    ${Enrollment_INCLUDES}
    ${IPCP_INCLUDES}
    ${Ribd_INCLUDES}
    ${RINA_API_INCLUDES}
    ${Rmt_INCLUDES}
  )

  #
  # Component: Enrollment
  #
  file(GLOB Enrollment_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/components/Enrollment/*.c"
  )
  add_library(Enrollment STATIC ${Enrollment_SOURCES})
  target_include_directories(Enrollment PUBLIC
    ${configSensor_INCLUDES}
    ${configRINA_INCLUDES}
    ${Common_INCLUDES}
    ${Portability_INCLUDES}
    ${ARP826_INCLUDES}
    ${CdapProto_INCLUDES}
    ${Enrollment_INCLUDES}
    ${FlowAllocator_INCLUDES}
    ${IPCP_INCLUDES}
    ${Ribd_INCLUDES}
    ${RINA_API_INCLUDES}
    ${Rmt_INCLUDES}
  )

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb3 -O0 -include stdbool.h")

  add_subdirectory(unity)

else()
  message("Unknown target ${TARGET_TYPE}")
endif()
