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
endif()
