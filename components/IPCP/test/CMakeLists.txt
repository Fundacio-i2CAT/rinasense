if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(SRC_DIRS "."
    INCLUDE_DIRS "."
    REQUIRES unity Common Portability FlowAllocator BufferManagement EFCP Shim)
else()
  add_executable(test_ipcp "${CMAKE_CURRENT_LIST_DIR}/test_ipcp.c")
  target_link_libraries(test_ipcp
    RINA
    NetworkInterface_MQ
    unity
    rt
  )

  add_test(
    NAME "test_ipcp"
    COMMAND "./test_ipcp"
  )
endif()
