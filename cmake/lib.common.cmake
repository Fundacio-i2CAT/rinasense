## PRIVATE MACROS.

macro(_rs_add_component component_name)
  get_property(prop_components GLOBAL PROPERTY _RS_COMPONENTS)
  list(APPEND prop_components ${component_name})
  set_property(GLOBAL PROPERTY _RS_COMPONENTS ${prop_components})
endmacro()

macro(_rs_set_component_data component_name data value)
  set(prop_name _RS_COMPONENTS_${component_name}_${data})
  set_property(GLOBAL PROPERTY ${prop_name} ${value})
endmacro()

macro(_rs_get_component_data component_name data var)
  set(prop_name _RS_COMPONENTS_${component_name}_${data})
  get_property(${var} GLOBAL PROPERTY ${prop_name})
endmacro()

macro(rs_get_component_include_dirs component_name var)
  _rs_get_component_data(${component_name} INCLUDES ${var})
endmacro()

function(_rs_get_dep_list component_name target_var)
  _rs_get_component_data(${component_name} REQUIRES component_req)
  foreach(req IN LISTS component_req)
    list(APPEND ${target_var} ${req})
    _rs_get_dep_list(${req} ${target_var})
  endforeach()
endfunction()

function(_rs_add_includes_from_component component_name from_component)
  set(seen_deps)
  set(req_list)

  # Add the include path from the direct dependency
  _rs_get_component_data(${from_component} INCLUDES from_includes)
  target_include_directories(${component_name} PUBLIC ${from_includes})

  # Make a list of ALL the include paths we need to add.
  _rs_get_component_data(${from_component} REQUIRES req_list)
  foreach(req IN LISTS req_list)
    if(NOT ${req} IN_LIST seen_deps)
      list(APPEND seen_deps ${req})
      _rs_get_component_data(${req} REQUIRES more_reqs)
      list(APPEND req_list ${more_reqs})
    endif()
  endforeach()

  # Add all the include paths
  foreach(req IN LISTS req_list)
    _rs_get_component_data(${req} INCLUDES req_includes)
    list(REMOVE_DUPLICATES req_includes)
    foreach(inc IN LISTS req_includes)
      target_include_directories(${component_name} PUBLIC ${inc})
    endforeach()
  endforeach()
endfunction()

## PUBLIC INTERFACE

function(rs_plain_component component_name)
  set(single_value DIR TEST_DIR)
  set(multi_values REQUIRES)
  cmake_parse_arguments(_ "${options}" "${single_value}" "${multi_values}" ${ARGN})
  rs_component(${component_name}
    SRCS      ${__DIR}/*.c
    INCLUDES  ${__DIR}/include
    TEST_DIR  ${__TEST_DIR}
    REQUIRES  ${__REQUIRES})
endfunction()

function(rs_include_component component_name)
  set(single_value DIR TEST_DIR)
  set(multi_values REQUIRES)
  cmake_parse_arguments(_ "${options}" "${single_value}" "${multi_values}" ${ARGN})
  rs_component(${component_name}
    INCLUDES  ${__DIR}/include
    TEST_DIR  ${__TEST_DIR}
    REQUIRES  ${__REQUIRES})
endfunction()

function(rs_component component_name)
  set(single_value TEST_DIR)
  set(multi_values SRCS INCLUDES REQUIRES)
  cmake_parse_arguments(_ "${options}" "${single_value}" "${multi_values}" ${ARGN})

  if(DEFINED __SRCS)
    # Plain static library
    set(${component_name}_SOURCES)

    foreach(src IN LISTS __SRCS)
      set(file ${CMAKE_CURRENT_LIST_DIR}/${src})
      if(EXISTS ${file})
        # If it's a file that exists, add it as-is.
        list(APPEND ${component_name}_SOURCES ${file})
      else()
        # If it's not a file, assume it's globbing patern.
        set(files)
        # Execute the globbing pattern
        file(GLOB files ${src})
        # Append the result to the list of source files for this
        # target.
        list(APPEND ${component_name}_SOURCES ${files})
      endif()
    endforeach()

    # We're making OBJECT libraries because CMake doesn't make it
    # super easy link static libraries together as a shared library.
    add_library(${component_name} OBJECT ${${component_name}_SOURCES})

    # Calls the macro charged to do anything that might be target
    # specific to the target.
    _rs_target_specific_ops(${component_name})

    # Add the locally defined include directories.
    target_include_directories(${component_name} PUBLIC ${__INCLUDES})

    # Save the list of include directories for resolving dependencies.
    _rs_set_component_data(${component_name} INCLUDES "${__INCLUDES}")
    _rs_set_component_data(${component_name} SOURCES ${${component_name}_SOURCES})
  else()
    # Include-only component

    add_library(${component_name} INTERFACE)
    target_include_directories(${component_name} INTERFACE ${__INCLUDES})

    # Save the list of include directories for resolving dependencies.
    _rs_set_component_data(${component_name} INCLUDES "${__INCLUDES}")
  endif()

  _rs_add_component(${component_name})

  if(DEFINED __REQUIRES)
    set(comp_requirements)
    foreach(req_name ${__REQUIRES})
      list(APPEND comp_requirements ${req_name})
    endforeach()

    _rs_set_component_data(${component_name} REQUIRES "${comp_requirements}")
  endif()

  if(DEFINED __TEST_DIR)
    _rs_target_specific_component_tests(${component_name} ${__TEST_DIR})
  endif()
endfunction()

function(rs_resolve_dependencies)
  # Get the list of components
  get_property(components GLOBAL PROPERTY _RS_COMPONENTS)

  # Iterate through all components to make sure that their dependency
  # list is sound.
  foreach(component_name ${components})
    message("Resolving dependencies of ${component_name}")

    _rs_get_component_data(${component_name} REQUIRES component_req_list)

    foreach(component_req IN LISTS component_req_list)
      if(${component_req} IN_LIST components)
        message("... requires ${component_req}: OK")
        _rs_add_includes_from_component(${component_name} ${component_req})
      else()
        message(FATAL_ERROR "... requires ${component_req}: NOT FOUND")
      endif()
    endforeach()
  endforeach()
endfunction()

macro(rs_scan_cmakelists)
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
endmacro()

# This removes the 'install' macro.
macro(rs_disable_install)
  macro(install)
  endmacro()
endmacro()

#
# ESP-IDF compatibility function.
#
# This does not support everything that the ESP-IDF build system has
# to offer.
#
function(idf_component_register)
  set(single_value DIR)
  set(multi_values SRCS SRC_DIRS INCLUDE_DIRS REQUIRES)
  cmake_parse_arguments(_ "${options}" "${single_value}" "${multi_values}" ${ARGN})

  cmake_path(GET CMAKE_CURRENT_LIST_DIR FILENAME comp_name)

  # Automatically add the 'test' directory as TEST_DIR if it exists.
  set(test_dir ${CMAKE_CURRENT_LIST_DIR}/test)
  if(NOT EXISTS ${test_dir})
    set(test_dir "")
  endif()

  set(new_srcs)
  foreach(src IN LISTS __SRCS)
    if(EXISTS ${src})
      list(APPEND new_srcs ${src})
    else()
      list(APPEND new_srcs ${CMAKE_CURRENT_LIST_DIR}/${src})
    endif()
  endforeach()

  set(new_includes)
  foreach(inc IN LISTS __INCLUDE_DIRS)
    if(EXISTS ${inc})
      list(APPEND new_includes ${inc})
    else()
      list(APPEND new_includes ${CMAKE_CURRENT_LIST_DIR}/${inc})
    endif()
  endforeach()

  # Call into our own component functions
  rs_component(${comp_name}
    SRCS     ${new_srcs}
    INCLUDES ${new_includes}
    TEST_DIR ${test_dir}
    REQUIRES ${__REQUIRES})
endfunction()
