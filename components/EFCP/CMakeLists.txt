idf_component_register(SRCS "connection.c" "delim.c" "EFCP.c" "dtp.c" "delim.c"
    INCLUDE_DIRS "include"
    REQUIRES IPCP Rmt FlowAllocator Portability)

if("$ENV{USE_MOCKS}" STREQUAL "true")
  target_include_directories(${COMPONENT_LIB} BEFORE PUBLIC
    "${CMAKE_SOURCE_DIR}/../components_mocks/mock_FlowAllocator/include"
  )

  # Per https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html#advanced-workaround-undefined-symbols
  #
  # I tried this randomly to fix a link error and it kind of magically
  # worked so I'm leaving that here. I don't claim to understand the
  # problem.
  target_link_libraries(${COMPONENT_LIB} INTERFACE "-u mock_FlowAllocator_xFlowAllocatorDuPost")
endif()
