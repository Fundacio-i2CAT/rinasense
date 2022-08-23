if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(SRC_DIRS "."
    INCLUDE_DIRS "."
    REQUIRES mock_IPCP NetworkInterface unity Common Portability)

  # target_include_directories(${COMPONENT_LIB} BEFORE PUBLIC
  #   "/home/neumann/W/Tria/rinasense-merge/components_mocks/mock_NetworkInterface/include"
  # )
else()
  add_executable(test_shim_ipcp "${CMAKE_CURRENT_LIST_DIR}/test_shim_ipcp.c")
  target_include_directories(test_shim_ipcp BEFORE PRIVATE
    ${mock_NetworkInterface_INCLUDES}
  )

  target_link_libraries(test_shim_ipcp
    unity
    IPCP
    Rmt
    BufferManagement
    ShimIPCP
    ARP826
    Portability
    pthread
    Common
    rt

    mock_NetworkInterface
    mock_FlowAllocator
    mock_IPCP
  )
endif()
