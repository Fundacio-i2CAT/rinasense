idf_component_register(SRCS "ARP826.c"
  INCLUDE_DIRS "include"
  REQUIRES NetworkInterface ShimIPCP BufferManagement configSensor Common Portability)

if("$ENV{USE_MOCKS}" STREQUAL "true")
  target_include_directories(${COMPONENT_LIB} BEFORE PUBLIC
    "${CMAKE_SOURCE_DIR}/../components_mocks/mock_NetworkInterface/include"  
  )
endif()

