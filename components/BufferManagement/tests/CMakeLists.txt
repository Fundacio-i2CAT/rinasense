add_executable(test_buffer_management "${CMAKE_CURRENT_LIST_DIR}/test_buffer_management.c")
target_link_libraries(test_buffer_management
  BufferManagement
  portability
  pthread
)

add_test(
  NAME test_buffer_management
  COMMAND "./test_buffer_management"
)
