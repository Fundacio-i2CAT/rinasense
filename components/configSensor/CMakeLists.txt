if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(
    SRCS "configSensor.c"
    INCLUDE_DIRS "include")
else()
  rs_plain_component(configSensor
    DIR      "${CMAKE_SOURCE_DIR}/components/configSensor")
endif()

