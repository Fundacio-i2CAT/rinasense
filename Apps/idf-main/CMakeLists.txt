cmake_minimum_required(VERSION 3.16)

if(NOT $ENV{IDF_PATH} STREQUAL "")
  set(EXTRA_COMPONENT_DIRS "../../components/")

  set(ENV{TARGET_TYPE} "freertos_idf")
  set(TARGET_TYPE $ENV{TARGET_TYPE})

  include($ENV{IDF_PATH}/tools/cmake/project.cmake)

  project(idf-main)

else()
  message(FATAL "This CMakeLists.txt file is to use with the ESP IDF only!")
endif()
