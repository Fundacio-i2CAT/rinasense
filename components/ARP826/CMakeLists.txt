if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(
    SRCS "ARP826.c"
    INCLUDE_DIRS "include"
    REQUIRES NetworkInterface ShimIPCP BufferManagement configSensor Common Portability RINA_API)
else()
  rs_plain_component(ARP826
    DIR       "${CMAKE_CURRENT_LIST_DIR}"
    TEST_DIR  "${CMAKE_CURRENT_LIST_DIR}/test"
    REQUIRES  Common BufferManagement IPCP Portability ShimIPCP)
endif()
