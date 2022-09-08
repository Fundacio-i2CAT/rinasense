if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(
    SRC_DIRS "."
    INCLUDE_DIRS "include"
    REQUIRES Portability Common IPCP)
else()
  rs_plain_component(mock_IPCP
    DIR      ${CMAKE_CURRENT_LIST_DIR}
    REQUIRES Portability Common IPCP)
endif()
