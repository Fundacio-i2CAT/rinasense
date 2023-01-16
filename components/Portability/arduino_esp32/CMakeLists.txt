set(Portability_INCLUDES
  ${CMAKE_SOURCE_DIR}/components/Portability/include
  ${CMAKE_SOURCE_DIR}/components/Portability/posix/include
  ${CMAKE_SOURCE_DIR}/components/Portability/freertos_idf/include
)

set(Portability_SOURCES
  ${CMAKE_SOURCE_DIR}/components/Portability/arduino_esp32/*.cpp
  ${CMAKE_SOURCE_DIR}/components/Portability/posix/*.c
  ${CMAKE_SOURCE_DIR}/components/Portability/gcc/*.c
)

rs_component(Portability
  SRCS     ${Portability_SOURCES}
  INCLUDES ${Portability_INCLUDES}
  REQUIRES FreeRTOS-Plus-POSIX Common
)
