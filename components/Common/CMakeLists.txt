if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(
    SRCS "rsrc.c" "rina_name.c" "rina_gpha.c" "rina_ids.c" "num_mgr.c" "mac.c" "list.c"
    INCLUDE_DIRS "include"
    REQUIRES Portability configSensor)
else()
  rs_plain_component(Common
    DIR       "${CMAKE_CURRENT_LIST_DIR}"
    TEST_DIR  "${CMAKE_CURRENT_LIST_DIR}/test"
    REQUIRES  Portability configSensor)
endif()
