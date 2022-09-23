# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.14)

set(TARGET_TYPE "freertos_idf"
  CACHE STRING "Type of build: 'linux', 'freertos_idf', 'arduino_esp32'")

file(TO_CMAKE_PATH "$ENV{IDF_PATH}" ENV_IDF_PATH)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# This is the entry point for the ESP/IDF build system. What is
# happening there is managed by the build environment of Espressif.
if("${TARGET_TYPE}" STREQUAL "" OR "${TARGET_TYPE}" STREQUAL "freertos_idf")
  if(NOT "$ENV{IDF_PATH}" STREQUAL "")
    # Set a default target type.

    set(TARGET_TYPE "freertos_idf")
    set(ENV{TARGET_TYPE} "freertos_idf")

    message("Configuring: ESP-IDF build")

    include($ENV{IDF_PATH}/tools/cmake/project.cmake)
    project(RINA_sensor)
  else()
    message(FATAL_ERROR "The 'freertos_idf' target needs the IDF_PATH environment variable to be defined")
  endif()

else()
  if(${TARGET_TYPE} STREQUAL "arduino_esp32")
    # This really needs to be set before the call to project()
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/Arduino-CMake-Toolchain/Arduino-toolchain.cmake)
  endif()

  project(RINA_sensor)

  #
  # This is the entry point for all platform support building through
  # CMake and GCC.
  #

  include(lib.common)
  include(lib.${TARGET_TYPE})

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

  # Common warnings and options.
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

  # Automagically find all the components in the source tree.
  rs_scan_cmakelists()

  # Runs a sanity check on the dependencies and add all the required
  # include paths to each libraries.
  rs_resolve_dependencies()

  #
  # Target specific code starts here.
  #

  if(${TARGET_TYPE} STREQUAL "arduino_esp32")
    #
    # ESP32 + Arduino!
    #

    # The Arduino/ESP32 port needs more component
    list(APPEND shared_library_components
      FreeRTOS-Plus-POSIX
    )

    # Add all components objects in the Linux shared library.
    foreach(component IN LISTS shared_library_components)
      list(APPEND shared_library_objects $<TARGET_OBJECTS:${component}>)
      rs_get_component_include_dirs(${component} component_includes)
      list(APPEND shared_library_includes ${component_includes})
    endforeach()

    # GCC sets this in Linux but the Arduino toolchain apparently does
    # not.
    add_compile_definitions(__LONG_WIDTH__=32)

    add_library(RINA ${shared_library_objects})
    target_include_directories(RINA PUBLIC ${shared_library_includes})
    target_link_arduino_libraries(RINA PRIVATE core)

    add_subdirectory(test_arduino)

  elseif(${TARGET_TYPE} STREQUAL "linux")
    #
    # Linux/POSIX
    #

    include(CTest)
    include(GNUInstallDirs)

    # Add all components objects in the Linux shared library.
    foreach(component IN LISTS shared_library_components)
      list(APPEND shared_library_objects $<TARGET_OBJECTS:${component}>)
      rs_get_component_include_dirs(${component} component_includes)
      list(APPEND shared_library_includes ${component_includes})
      target_compile_options(${component} PRIVATE -fPIC)
    endforeach()

    # Public headers we'll install with the target
    #
    # FIXME: We need a better way than this to identify headers which
    # should be part of the install.
    set(public_headers
      "components/RINA_API/include/RINA_API.h"
      "components/RINA_API/include/RINA_API_flows.h"
      "components/configRINA/include/configRINA.h"
      "components/IPCP/include/IPCP.h"
      "components/Portability/include/portability/port.h"
      "components/Portability/include/portability/rsassert.h"
      "components/Portability/include/portability/rsdefs.h"
      "components/Portability/include/portability/rslog.h"
      "components/Portability/include/portability/rsmem.h"
      "components/Portability/include/portability/rsnet.h"
      "components/Portability/include/portability/rsposix.h"
      "components/Portability/include/portability/rsqueue.h"
      "components/Portability/include/portability/rstime.h"
      "components/Common/include/common/rina_gpha.h"
      "components/Common/include/common/rina_name.h")

    # Declare the shared library.
    add_library(RINA SHARED ${shared_library_objects})
    target_sources(RINA PUBLIC ${public_headers})
    target_include_directories(RINA PUBLIC ${shared_library_includes})

    install(TARGETS RINA)
    install(FILES ${public_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/RINA)

    add_subdirectory(test_linux)

    # We do not want the 'install()' function to work at all anymore.
    rs_disable_install()
    add_subdirectory(unity)
  endif()
endif()
