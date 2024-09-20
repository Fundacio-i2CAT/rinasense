set(requirements
  Shim
  Portability
  Common
)

# The FreeRTOS-Plus-POSIX module is only necessary for ESP-IDF
# on-device builds.
if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  list(APPEND requirements FreeRTOS-Plus-POSIX)
endif()

idf_component_register(
  SRCS "BufferManagement.c"
  INCLUDE_DIRS "include"
  REQUIRES ${requirements}
)
