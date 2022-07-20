if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(SRC_DIRS "."
    INCLUDE_DIRS "."
    REQUIRES mock_IPCP mock_NetworkInterface unity ARP826)

  if("$ENV{USE_MOCKS}" STREQUAL "true")
    target_include_directories(${COMPONENT_LIB} BEFORE PUBLIC
      "${CMAKE_SOURCE_DIR}/../components_mocks/mock_NetworkInterface/include"  
    )
  endif()
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
    mock_IPCP
    unity
    rt
    mock_NetworkInterface
  )

  add_test(
    NAME test_arp826
    COMMAND "./test_arp826"
  )
endif()
