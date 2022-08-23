# idf_component_register(SRCS "ShimIPCP.c"
#                     INCLUDE_DIRS "include"
#                     REQUIRES BufferManagement ARP826 NetworkInterface IPCP configRINA configSensor Rmt)

if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(
    SRCS "ShimIPCP.c"
    INCLUDE_DIRS "include"
    REQUIRES BufferManagement ARP826 NetworkInterface IPCP configRINA configSensor Rmt)
else()
  rs_plain_component(ShimIPCP
    DIR       "${CMAKE_CURRENT_LIST_DIR}"
    TEST_DIR  "${CMAKE_CURRENT_LIST_DIR}/test"
    REQUIRES  Common Portability ARP826 BufferManagement EFCP IPCP Rmt NetworkInterface)
endif()

