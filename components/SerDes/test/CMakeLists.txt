if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(SRC_DIRS "."
    INCLUDE_DIRS "."
    REQUIRES unity Common SerDEs)
else()
  add_executable(test_SerDesMessage "${CMAKE_CURRENT_LIST_DIR}/test_SerDesMessage.c")
  target_link_libraries(test_SerDesMessage
    RINA
    SerDes
    NetworkInterface_MQ
    unity
  )

  add_test(
    NAME test_SerDesMessage
    COMMAND "./test_SerDesMessage"
  )
endif()
