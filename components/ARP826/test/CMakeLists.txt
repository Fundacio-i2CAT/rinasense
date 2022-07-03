if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(SRC_DIRS "."
    INCLUDE_DIRS "."
    REQUIRES unity ARP826 mock_IPCP mock_NetworkInterface)
else()
  add_executable(test_arp826 "${CMAKE_CURRENT_LIST_DIR}/test_arp826.c")
  target_include_directories(test_arp826
    BEFORE PUBLIC ${mock_NetworkInterface_INCLUDES}
  )
  target_link_libraries(test_arp826
    ARP826
    BufferManagement
    pthread
    portability
    Common
    mock_NetworkInterface
    mock_IPCP
    unity
    rt
  )

  add_test(
    NAME test_arp826
    COMMAND "./test_arp826"
  )
endif()
