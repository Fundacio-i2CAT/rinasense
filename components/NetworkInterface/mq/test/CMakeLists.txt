add_executable(test_networkinterface_io "${CMAKE_CURRENT_LIST_DIR}/test_networkinterface_io.c")
target_include_directories(test_networkinterface_io PUBLIC
  ${mock_IPCP_INCLUDES}
  ${NetworkInterface_MQ_INCLUDES}
)
target_link_libraries(test_networkinterface_io
  NetworkInterface
  BufferManagement
  portability
  pthread
  rt
  mock_IPCP
)

add_test(
  NAME test_networkinterface_io
  COMMAND "./test_networkinterface_io"
)
