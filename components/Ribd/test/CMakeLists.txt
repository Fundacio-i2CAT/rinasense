if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(SRC_DIRS "."
    INCLUDE_DIRS "."
    REQUIRES unity Common)
else()
  set(RIBD_DEPS
    # This is odd but there seem to be a circular dependency between
    # Ribd and Enrollment.
    Ribd
    Enrollment
    Ribd

    CdapProto
    Rmt
    BufferManagement
    mock_IPCP
    IPCP
    EFCP
    FlowAllocator
    unity
    rt
    pthread
    Common
    portability
  )

  add_executable(test_rib "${CMAKE_CURRENT_LIST_DIR}/test_rib.c")
  target_link_libraries(test_rib
    ${RIBD_DEPS}
  )

  add_test(
    NAME test_rib
    COMMAND "./test_rib"
  )
endif()
