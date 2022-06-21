add_executable(test_arp826 "${CMAKE_CURRENT_LIST_DIR}/test_arp826.c")
target_link_libraries(test_arp826
  ARP826
  BufferManagement
  pthread
  portability
  Common
  mock_NetworkInterface
  mock_IPCP
)

add_test(
  NAME test_arp826
  COMMAND "./test_arp826"
)
