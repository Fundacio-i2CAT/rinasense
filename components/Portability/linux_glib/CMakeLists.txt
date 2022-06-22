if(NOT ${TARGET_TYPE} STREQUAL "freertos_idf")
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(deps REQUIRED IMPORTED_TARGET glib-2.0)

  file(GLOB portability_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/components/Portability/${TARGET_TYPE}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/Portability/posix/*.c"
  )
  set(Portability_INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}/components/Portability/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/Portability/posix/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/Portability/${TARGET_TYPE}/include"
    "${deps_INCLUDE_DIRS}"
  )
  add_library(portability STATIC ${portability_SOURCES})
  target_link_libraries(portability PkgConfig::deps)
  target_include_directories(portability PUBLIC
    ${Portability_INCLUDES}
    ${Common_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR}
  )
endif()
