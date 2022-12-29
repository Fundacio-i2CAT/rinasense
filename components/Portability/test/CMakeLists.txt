if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(SRC_DIRS "."
    INCLUDE_DIRS "."
    REQUIRES unity Common Portability)
else()
  add_executable(test_time "${CMAKE_CURRENT_LIST_DIR}/test_time.c")
  target_link_libraries(test_time
    Portability
    Common
    unity
  )

  add_test(
    NAME test_time
    COMMAND "./test_time"
  )
endif()
