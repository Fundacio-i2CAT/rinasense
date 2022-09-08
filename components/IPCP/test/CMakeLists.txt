if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(SRC_DIRS "."
    INCLUDE_DIRS "."
    REQUIRES unity Common Portability ARP826 FlowAllocator BufferManagement EFCP ShimIPCP)
else()
  add_executable(test_ipcp "${CMAKE_CURRENT_LIST_DIR}/test_ipcp.c")
  target_link_libraries(test_ipcp
    IPCP
    FlowAllocator
    BufferManagement
    EFCP
    ShimIPCP
    ARP826
    Enrollment
    Ribd
    IPCP
    CdapProto
    Rmt
    pthread
    portability
    NetworkInterface_MQ
    Common
    unity
    rt
  )

  add_test(
    NAME "test_ipcp"
    COMMAND "./test_ipcp"
  )
endif()
