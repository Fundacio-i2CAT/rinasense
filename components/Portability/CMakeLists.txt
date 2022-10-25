if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  set(Portability_INCLUDES
    "${COMMON_INCLUDES}"
    "${CMAKE_SOURCE_DIR}/components/Portability/include"
    "${CMAKE_SOURCE_DIR}/components/Portability/posix/include"
    "${CMAKE_SOURCE_DIR}/components/Portability/freertos_idf/include")

  idf_component_register(
    SRCS
    "freertos_idf/rsmem.c"
    "freertos_idf/rslog.c"
    "posix/posix_rsqueue.c"
    "posix/posix_rstime.c"
    INCLUDE_DIRS ${Portability_INCLUDES}
    REQUIRES FreeRTOS-Plus-POSIX)
else()
  include(${CMAKE_CURRENT_LIST_DIR}/${TARGET_TYPE}/CMakeLists.txt)
endif()

