if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(
    SRCS "configRINA.c"
    INCLUDE_DIRS "include")
else()
  rs_plain_component(configRINA
    DIR      "${CMAKE_SOURCE_DIR}/components/configRINA")
endif()
