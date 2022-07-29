idf_component_register(SRCS "SerdesMsg.c" "Enrollment.c" "SerdesMsg"
                    INCLUDE_DIRS "include"
                    REQUIRES IPCP BufferManagement Ribd CdapProto FlowAllocator)

if("$ENV{USE_MOCKS}" STREQUAL "true")
  target_include_directories(${COMPONENT_LIB} BEFORE PUBLIC
    "${CMAKE_SOURCE_DIR}/../components_mocks/mock_Ribd/include"
  )
endif()
