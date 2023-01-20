if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(SRC_DIRS "."
    INCLUDE_DIRS "."
    REQUIRES mock_IPCP NetworkInterface unity Common Portability)
else()
  add_executable(test_shim_ipcp "${CMAKE_CURRENT_LIST_DIR}/test_shim_ipcp.c")
  target_include_directories(test_shim_ipcp BEFORE PRIVATE
    ${mock_NetworkInterface_INCLUDES}
  )

  target_link_libraries(test_shim_ipcp
    RINA
    Portability
    unity
    pthread
    Common
    rt

    mock_NetworkInterface
    mock_FlowAllocator
    mock_IPCP
  )

  add_test(
    NAME test_shim_ipcp
    COMMAND "./test_shim_ipcp"
  )

  add_executable(test_arp826_cache "${CMAKE_CURRENT_LIST_DIR}/test_arp826_cache.c")
  target_include_directories(test_arp826_cache BEFORE PRIVATE)

  target_link_libraries(test_arp826_cache
    RINA
    NetworkInterface_MQ
    Portability
    unity
    pthread
    Common
    rt
  )

  add_test(
    NAME test_arp826_cache
    COMMAND "./test_arp826_cache"
  )
endif()
