# idf_component_register(SRCS "FlowAllocator.c"
#   INCLUDE_DIRS "include"
#   REQUIRES IPCP BufferManagement Ribd CdapProto Rmt)

# if("$ENV{USE_MOCKS}" STREQUAL "true")
#   target_include_directories(${COMPONENT_LIB} BEFORE PUBLIC
#     "${CMAKE_SOURCE_DIR}/../components_mocks/mock_Ribd/include"
#   )
# endif()

if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(
    SRCS "FlowAllocator.c"
    INCLUDE_DIRS "include"
    REQUIRES IPCP BufferManagement Ribd CdapProto Rmt)
else()
  rs_plain_component(FlowAllocator
    DIR       "${CMAKE_CURRENT_LIST_DIR}"
    REQUIRES  Common Portability EFCP ARP826 CdapProto Enrollment IPCP Ribd Rmt)
endif()
