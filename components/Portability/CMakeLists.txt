set(Portability_INCLUDES
  "${COMMON_INCLUDES}"
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/posix/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_TYPE}/include")

if(NOT ${TARGET_TYPE} STREQUAL "freertos_idf")
  include(${CMAKE_CURRENT_SOURCE_DIR}/${TARGET_TYPE}/CMakeLists.txt)
else()
  idf_component_register(
    SRCS
    "freertos_idf/rslist.c"
    "freertos_idf/rsmem.c"
    "freertos_idf/rslog.c"
    "posix/posix_rsqueue.c"
    "posix/posix_rstime.c"

    INCLUDE_DIRS ${Portability_INCLUDES}
    REQUIRES FreeRTOS-Plus-POSIX)
endif()

