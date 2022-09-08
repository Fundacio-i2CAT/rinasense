if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(
    SRCS "NetworkInterface.c"
    INCLUDE_DIRS "include"
    REQUIRES Portability Common IPCP)
else()
  rs_plain_component(mock_NetworkInterface
    DIR      ${CMAKE_CURRENT_LIST_DIR}
    REQUIRES Portability Common IPCP)
endif()
