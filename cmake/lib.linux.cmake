macro(_rs_target_specific_ops component_name)
  # Nothing to do.
endmacro()

macro(_rs_target_specific_component_tests component_name test_dir)
  include(${test_dir}/CMakeLists.txt OPTIONAL)
endmacro()
