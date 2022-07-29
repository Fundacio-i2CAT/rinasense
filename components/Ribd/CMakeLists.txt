idf_component_register(SRCS "Rib.c" "Ribd.c"
                    INCLUDE_DIRS "include"
                    REQUIRES BufferManagement ShimIPCP configSensor IPCP CdapProto Enrollment RINA_API)

if("$ENV{USE_MOCKS}" STREQUAL "true")
  target_include_directories(${COMPONENT_LIB} BEFORE PUBLIC
    "${CMAKE_SOURCE_DIR}/../components_mocks/mock_Ribd/include"  
  )
endif()

