# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(TARGET_TYPE "freertos_idf"
  CACHE STRING "Type of build: 'linux', 'freertos_idf'")

file(TO_CMAKE_PATH "$ENV{IDF_PATH}" ENV_IDF_PATH)

if("${TARGET_TYPE}" STREQUAL "" OR "${TARGET_TYPE}" STREQUAL "freertos_idf")
  if(NOT "$ENV{IDF_PATH}" STREQUAL "")
    # Set a default target type.

    set(TARGET_TYPE "freertos_idf")
    set(ENV{TARGET_TYPE} "freertos_idf")

    message("Configuring: ESP-IDF build")

    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    add_compile_definitions(configUSE_POSIX_ERRNO=1)
    project(RINA_sensor)
  else()
    message(FATAL_ERROR "The 'freertos_idf' target needs the IDF_PATH environment variable to be defined")
  endif()
elseif (${TARGET_TYPE} STREQUAL "linux")
  project(RINA_sensor)

  set(ENV{TARGET_TYPE} "linux")
  message("Configuring: POSIX build")
  include("CMakeLists.macros.linux")

  file(GLOB component_dirs ${CMAKE_SOURCE_DIR}/components/*)
  file(GLOB mock_component_dirs ${CMAKE_SOURCE_DIR}/components_mocks/*)
  list(APPEND component_dirs ${mock_component_dirs})
  foreach(component_dir IN LISTS component_dirs)
    message("Looking in ${component_dir}")
    set(cmakelist "${component_dir}/CMakeLists.txt")
    if(EXISTS ${cmakelist})
      message("Including ${cmakelist}")
      include(${cmakelist})
    endif()
  endforeach()

  include(CTest)

  add_compile_definitions(configUSE_POSIX_ERRNO=1)

  add_compile_options(
    -Wall -Werror -Wextra

    # Disable some warnings while development is active. It drowns the
    # other more important warnings.
    -Wno-unused-but-set-variable
    -Wno-unused-function
    -Wno-unused-variable
    -Wno-unused-parameter

    # Debugging information
    -ggdb3

    # Disables optimisation
    -O0

    # Automatically add 'stdbool.h' as an include file to all
    # files. It is a bit weird to add it here so maybe I should
    # consider including it in the Portability component, or in Common
    -include stdbool.h
  )

  set(CMAKE_VERBOSE_MAKEFILE ON)

  rs_resolve_dependencies()

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb3 -O0 -include stdbool.h")

  add_subdirectory(unity)
  add_subdirectory(test_linux)

else()
  message("Unknown target ${TARGET_TYPE}")
endif()
