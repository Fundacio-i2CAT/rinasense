idf_component_register(SRCS "FlowAllocator.c"
                    INCLUDE_DIRS "include"
                    REQUIRES IPCP BufferManagement Ribd CdapProto)

if("$ENV{USE_MOCKS}" STREQUAL "true")
  target_include_directories(${COMPONENT_LIB} BEFORE PUBLIC
    "${CMAKE_SOURCE_DIR}/../components_mocks/mock_Ribd/include" 
  )
endif()


