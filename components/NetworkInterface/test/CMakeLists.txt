if("$ENV{TARGET_TYPE}" STREQUAL "linux" OR "${TARGET}" STREQUAL "linux")
  add_executable(test_networkinterface_io "${CMAKE_CURRENT_LIST_DIR}/test_networkinterface_io.c")
  target_include_directories(test_networkinterface_io PUBLIC
    ${mock_IPCP_INCLUDES}
    ${NetworkInterface_MQ_INCLUDES}
  )
  target_link_libraries(test_networkinterface_io
    mock_IPCP
    RINA
    NetworkInterface_MQ
    unity
  )

  add_test(
    NAME test_networkinterface_io
    COMMAND "./test_networkinterface_io"
  )
endif()
