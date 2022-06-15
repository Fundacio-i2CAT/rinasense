add_executable(test_lists "${CMAKE_CURRENT_LIST_DIR}/test_lists.c")
target_link_libraries(test_lists
  portability
)

add_test(
  NAME test_lists
  COMMAND "./test_lists"
)
