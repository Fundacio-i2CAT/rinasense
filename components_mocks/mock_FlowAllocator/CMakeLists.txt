if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(
    SRCS "flowAllocator.c"
    INCLUDE_DIRS "include"
    REQUIRES Portability Common IPCP mock_FlowAllocator)
else()
  rs_plain_component(mock_FlowAllocator
    DIR      ${CMAKE_CURRENT_LIST_DIR}
    REQUIRES Portability Common IPCP
  )
endif()
