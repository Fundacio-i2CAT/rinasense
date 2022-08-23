
# if("$ENV{USE_MOCKS}" STREQUAL "true")
#   target_include_directories(${COMPONENT_LIB} BEFORE PUBLIC
#     "${CMAKE_SOURCE_DIR}/../components_mocks/mock_Ribd/include"
#   )
# endif()

if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(
    SRCS "Rib.c" "Ribd.c"
    INCLUDE_DIRS "include"
    REQUIRES FlowAllocator BufferManagement ShimIPCP configSensor IPCP CdapProto Enrollment RINA_API)
else()
  rs_plain_component(Ribd
    DIR       ${CMAKE_CURRENT_LIST_DIR}
    TEST_DIR  ${CMAKE_CURRENT_LIST_DIR}/test
    REQUIRES  CdapProto BufferManagement IPCP Portability Common Enrollment
              EFCP Rmt FlowAllocator)
endif()

