# idf_component_register(
#   SRCS "IpcManager.c" "normalIPCP.c" "IPCP.c" "common.c"
#   INCLUDE_DIRS "include"
#   REQUIRES configSensor NetworkInterface ShimIPCP BufferManagement ARP826 Rmt RINA_API EFCP Enrollment Ribd FlowAllocator)

# if("$ENV{USE_MOCKS}" STREQUAL "true")
#   target_include_directories(${COMPONENT_LIB} BEFORE PUBLIC
#     "${CMAKE_SOURCE_DIR}/../components_mocks/mock_FlowAllocator/include"
#     "${CMAKE_SOURCE_DIR}/../components_mocks/mock_Ribd/include"
#   )
# endif()

if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(
    SRCS "IpcManager.c" "normalIPCP.c" "IPCP.c" "common.c"
    INCLUDE_DIRS "include"
    REQUIRES configSensor NetworkInterface ShimIPCP BufferManagement ARP826 Rmt RINA_API EFCP Enrollment Ribd FlowAllocator)
else()
  rs_plain_component(IPCP
    DIR       "${CMAKE_CURRENT_LIST_DIR}"
    TEST      "${CMAKE_CURRENT_LIST_DIR}/test"
    REQUIRES  configSensor configRINA ARP826 ShimIPCP BufferManagement
              Rmt EFCP Portability Common RINA_API Ribd Enrollment
              Ribd FlowAllocator)
endif()
