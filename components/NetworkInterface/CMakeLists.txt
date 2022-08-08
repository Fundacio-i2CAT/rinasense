if("$ENV{NETWORKINTERFACE_MODULE}" STREQUAL "")
  set(NETWORKINTERFACE_MODULE esp32)
else()
  set(NETWORKINTERFACE_MODULE $ENV{NETWORKINTERFACE_MODULE})
endif()

idf_component_register(SRCS "${NETWORKINTERFACE_MODULE}/NetworkInterface.c"
  INCLUDE_DIRS "include" "mq/include"
  REQUIRES Common BufferManagement ARP826 ShimIPCP configSensor FreeRTOS-Plus-POSIX)
