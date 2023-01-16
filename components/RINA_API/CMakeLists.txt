set(RINA_API_REQUIRES
  configSensor
  IPCP
  FlowAllocator)

if(NOT ("$ENV{TARGET_TYPE}" STREQUAL "linux" OR "${TARGET_TYPE}" STREQUAL "linux"))
  list(APPEND RINA_API_REQUIRES FreeRTOS-Plus-POSIX)
endif()

idf_component_register(
  SRCS "RINA_API.c"
  INCLUDE_DIRS "include"
  REQUIRES ${RINA_API_REQUIRES})
