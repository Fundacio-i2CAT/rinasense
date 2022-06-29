if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(SRC_DIRS "."
    INCLUDE_DIRS "."
    REQUIRES BufferManagement unity Common mock_IPCP)
else()
  add_executable(test_networkinterface_io "${CMAKE_CURRENT_LIST_DIR}/test_networkinterface_io.c")
  target_include_directories(test_networkinterface_io PUBLIC
    ${mock_IPCP_INCLUDES}
    ${NetworkInterface_MQ_INCLUDES}
  )
  target_link_libraries(test_networkinterface_io
    mock_IPCP
    NetworkInterface
    BufferManagement
    portability
    pthread
    rt
    unity
  )

  add_test(
    NAME test_networkinterface_io
    COMMAND "./test_networkinterface_io"
  )
endif()
