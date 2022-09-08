if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(SRC_DIRS "."
    INCLUDE_DIRS "."
    REQUIRES IPCP NetworkInterface unity ARP826)
else()
  add_executable(test_arp826 "${CMAKE_CURRENT_LIST_DIR}/test_arp826.c")
  target_link_libraries(test_arp826 PUBLIC
    RINA
    NetworkInterface_MQ
    unity
  )

  add_test(
    NAME test_arp826
    COMMAND "./test_arp826"
  )
endif()
