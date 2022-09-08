# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(TARGET_TYPE "freertos_idf"
  CACHE STRING "Type of build: 'linux', 'freertos_idf'")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

file(TO_CMAKE_PATH "$ENV{IDF_PATH}" ENV_IDF_PATH)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

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
  include(lib.linux)
  include(lib.common)
  include(CTest)

  # Automagically find all the components in the source tree.
  rs_scan_cmakelists()

  # Runs a sanity check on the dependencies and add all the required
  # include paths to each libraries.
  rs_resolve_dependencies()

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

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb3 -O0 -include stdbool.h")

  add_subdirectory(unity)
  add_subdirectory(test_linux)

  # Declares which components needs to be built in the shared library.
  set(shared_library_components
    ARP826
    BufferManagement
    CdapProto
    Common
    configRINA
    configSensor
    EFCP
    Enrollment
    FlowAllocator
    IPCP
    Portability
    Ribd
    RINA_API
    Rmt
    ShimIPCP)

  # Add all components objects in the shared library.
  foreach(component IN LISTS shared_library_components)
    list(APPEND shared_library_objects $<TARGET_OBJECTS:${component}>)
    rs_get_component_include_dirs(${component} component_includes)
    list(APPEND shared_library_includes ${component_includes})
  endforeach()

  # Declare the shared library.
  add_library(RINA SHARED ${shared_library_objects})
  target_include_directories(RINA PUBLIC ${shared_library_includes})

else()
  message("Unknown target ${TARGET_TYPE}")
endif()
