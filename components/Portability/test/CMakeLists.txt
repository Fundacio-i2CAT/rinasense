if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(SRC_DIRS "."
    INCLUDE_DIRS "."
    REQUIRES unity Common Portability)
else()
  add_executable(test_lists "${CMAKE_CURRENT_LIST_DIR}/test_lists.c")
  target_link_libraries(test_lists
    portability
    unity
  )

  add_test(
    NAME test_lists
    COMMAND "./test_lists"
  )

  add_executable(test_queues "${CMAKE_CURRENT_LIST_DIR}/test_queues.c")
  target_link_libraries(test_queues
    portability
    unity
    pthread
    rt
  )

  add_test(
    NAME test_queues
    COMMAND "./test_queues"
  )
endif()
