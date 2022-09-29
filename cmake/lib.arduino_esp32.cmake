macro(_rs_target_specific_ops component_name)
  ## Add the 'core' library to the individual targets to allow them to
  ## use Arduino specific things.Tru
  target_link_arduino_libraries(${component_name} PRIVATE core)
endmacro()

macro(_rs_target_specific_component_tests component_name test_dir)
endmacro()
