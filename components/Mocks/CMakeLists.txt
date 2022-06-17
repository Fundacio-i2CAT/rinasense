# NetworkInterface Mock
#set(Portability_INCLUDES "test")
# if(NOT DEFINED Portability_INCLUDES)
#   message(FATAL_ERROR "Portability_INCLUDES not defined")
# else()
#   message("Portability_INCLUDES is: ${Portability_INCLUDES}")
# endif()

if(NOT ${TARGET_TYPE} STREQUAL "freertos_idf")
  file(GLOB mock_NetworkInterface_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/components/Mocks/NetworkInterface/*.c"
  )
  add_library(mock_NetworkInterface STATIC ${mock_NetworkInterface_SOURCES})
  target_include_directories(mock_NetworkInterface PUBLIC
    ${BufferManagement_INCLUDES}
    ${Portability_INCLUDES}
    ${IPCP_INCLUDES}
  )

  file(GLOB mock_IPCP_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/components/Mocks/IPCP/*.c"
  )
  add_library(mock_IPCP STATIC ${mock_IPCP_SOURCES})
  target_include_directories(mock_IPCP PUBLIC
    ${BufferManagement_INCLUDES}
    ${Portability_INCLUDES}
    ${IPCP_INCLUDES}
  )

  file(GLOB mock_RIBD_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/components/Mocks/Ribd/*.c"
  )
  add_library(mock_Ribd STATIC ${mock_RIBD_SOURCES})
  target_include_directories(mock_Ribd PUBLIC
    ${configSensor_INCLUDES}
    ${Common_INCLUDES}
    ${EFCP_INCLUDES}
    ${BufferManagement_INCLUDES}
    ${Portability_INCLUDES}
    ${IPCP_INCLUDES}
    ${Rmt_INCLUDES}
  )
endif()
