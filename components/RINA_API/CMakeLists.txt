if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(
    SRCS "RINA_API.c"
    INCLUDE_DIRS "include"
    REQUIRES configSensor BufferManagement IPCP FlowAllocator)
else()
  rs_plain_component(RINA_API
    DIR       "${CMAKE_CURRENT_LIST_DIR}"
    REQUIRES  Common Portability BufferManagement)
endif()
