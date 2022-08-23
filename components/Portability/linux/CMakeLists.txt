if("$ENV{TARGET_TYPE}" STREQUAL "linux")
  set(local_INCLUDES
    ${CMAKE_SOURCE_DIR}/components/Portability/include
    ${CMAKE_SOURCE_DIR}/components/Portability/posix/include
    ${CMAKE_SOURCE_DIR}/components/Portability/$ENV{TARGET_TYPE}/include)

  set(Portability_SOURCES
    ${CMAKE_SOURCE_DIR}/components/Portability/$ENV{TARGET_TYPE}/*.c
    ${CMAKE_SOURCE_DIR}/components/Portability/posix/*.c
  )

  rs_component(Portability
    SRCS     ${Portability_SOURCES}
    INCLUDES ${local_INCLUDES}
    TEST_DIR ${CMAKE_CURRENT_LIST_DIR}/../test
  )
endif()
