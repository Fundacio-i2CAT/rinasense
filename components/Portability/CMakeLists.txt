if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  set(Portability_INCLUDES
    "${COMMON_INCLUDES}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/posix/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/freertos_idf/include")

  idf_component_register(
    SRCS
    "gcc/rserr.c"
    "freertos_idf/rsmem.c"
    "freertos_idf/rslog.c"
    "posix/posix_rstime.c"
    "posix/posix_rssem.c"
    INCLUDE_DIRS ${Portability_INCLUDES}
    REQUIRES FreeRTOS-Plus-POSIX Common)
else()
  include(${CMAKE_CURRENT_LIST_DIR}/${TARGET_TYPE}/CMakeLists.txt)
endif()

