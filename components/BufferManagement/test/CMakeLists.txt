if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(SRC_DIRS "."
    INCLUDE_DIRS "."
    REQUIRES unity BufferManagement)
else()
  add_executable(test_buffer_management "${CMAKE_CURRENT_LIST_DIR}/test_buffer_management.c")
  target_link_libraries(test_buffer_management
    BufferManagement
    Portability
    pthread
    unity
    rt
  )

  add_test(
    NAME test_buffer_management
    COMMAND "./test_buffer_management"
  )
endif()

