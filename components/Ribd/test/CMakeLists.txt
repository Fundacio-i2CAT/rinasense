if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(SRC_DIRS "."
    INCLUDE_DIRS "."
    REQUIRES unity Common Ribd)
else()
  set(RIBD_DEPS
    RINA
    NetworkInterface_MQ
    unity
  )

  add_executable(test_rib "${CMAKE_CURRENT_LIST_DIR}/test_rib.c")
  target_link_libraries(test_rib
    ${RIBD_DEPS}
  )

  add_test(
    NAME test_rib
    COMMAND "./test_rib"
  )

  add_executable(test_cdap_message "${CMAKE_CURRENT_LIST_DIR}/test_cdap_message.c")
  target_link_libraries(test_cdap_message
    ${RIBD_DEPS}
  )

  add_test(
    NAME test_cdap_message
    COMMAND "./test_cdap_message"
  )
endif()
