set(Portability_INCLUDES
  ${CMAKE_SOURCE_DIR}/components/Portability/include
  ${CMAKE_SOURCE_DIR}/components/Portability/posix/include
  ${CMAKE_SOURCE_DIR}/components/Portability/linux/include)

set(Portability_SOURCES
  ${CMAKE_SOURCE_DIR}/components/Portability/linux/*.c
  ${CMAKE_SOURCE_DIR}/components/Portability/posix/*.c
)

rs_component(Portability
  SRCS     ${Portability_SOURCES}
  INCLUDES ${Portability_INCLUDES}
  TEST_DIR ${CMAKE_CURRENT_LIST_DIR}/../test
  REQUIRES Common
)
