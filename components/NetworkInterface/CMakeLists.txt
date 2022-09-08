if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  if("$ENV{NETWORKINTERFACE_MODULE}" STREQUAL "")
    set(NETWORKINTERFACE_MODULE esp32)
  else()
    set(NETWORKINTERFACE_MODULE $ENV{NETWORKINTERFACE_MODULE})
  endif()

  idf_component_register(SRCS "${NETWORKINTERFACE_MODULE}/NetworkInterface.c"
    INCLUDE_DIRS "include" "mq/include"
    REQUIRES Common BufferManagement ARP826 ShimIPCP configSensor FreeRTOS-Plus-POSIX)
else()
  rs_include_component(NetworkInterface
    DIR       "${CMAKE_SOURCE_DIR}/components/NetworkInterface")

  rs_component(NetworkInterface_TAP
    SRCS     "${CMAKE_SOURCE_DIR}/components/NetworkInterface/linux_tap/*.c"
    INCLUDES "${CMAKE_SOURCE_DIR}/components/NetworkInterface/include"
    REQUIRES Common configRINA BufferManagement ARP826 IPCP configSensor Rmt ShimIPCP
             EFCP Portability RINA_API)

  rs_component(NetworkInterface_MQ
    SRCS     "${CMAKE_SOURCE_DIR}/components/NetworkInterface/mq/*.c"
    INCLUDES "${CMAKE_SOURCE_DIR}/components/NetworkInterface/include"
             "${CMAKE_SOURCE_DIR}/components/NetworkInterface/mq/include"
    REQUIRES Common configRINA BufferManagement ARP826 IPCP configSensor Rmt ShimIPCP
             EFCP Portability RINA_API)
endif()
