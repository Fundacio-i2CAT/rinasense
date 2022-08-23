if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(
    SRCS "BufferManagement.c"
    INCLUDE_DIRS "include"
    REQUIRES ShimIPCP NetworkInterface ARP826 Portability FreeRTOS-Plus-POSIX)
else()
  rs_plain_component(BufferManagement
    DIR       "${CMAKE_CURRENT_LIST_DIR}"
    TEST_DIR  "${CMAKE_CURRENT_LIST_DIR}/test"
    REQUIRES  configSensor Common ARP826)
endif()


