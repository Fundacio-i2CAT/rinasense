<<<<<<< HEAD
add_executable(test_rina_name "${CMAKE_CURRENT_LIST_DIR}/test_rina_name.c")
target_link_libraries(test_rina_name
  Common
  portability
)

add_executable(test_rina_gpha "${CMAKE_CURRENT_LIST_DIR}/test_rina_gpha.c")
target_link_libraries(test_rina_gpha
  Common
  portability
)

add_executable(test_bit_array "${CMAKE_CURRENT_LIST_DIR}/test_bit_array.c")
target_link_libraries(test_bit_array
  Common
  portability
)

add_executable(test_num_mgr "${CMAKE_CURRENT_LIST_DIR}/test_num_mgr.c")
target_link_libraries(test_num_mgr
  Common
  portability
)

add_test(
  NAME test_rina_name
  COMMAND "./test_rina_name"
)

add_test(
  NAME test_rina_gpha
  COMMAND "./test_rina_gpha"
)

add_test(
  NAME test_bit_array
  COMMAND "./test_bit_array"
)
=======
if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(SRC_DIRS "."
    INCLUDE_DIRS "."
    REQUIRES unity Common)
else()
  add_executable(test_rina_name "${CMAKE_CURRENT_LIST_DIR}/test_rina_name.c")
  target_link_libraries(test_rina_name
    Common
    portability
  )

  add_executable(test_rina_gpha "${CMAKE_CURRENT_LIST_DIR}/test_rina_gpha.c")
  target_link_libraries(test_rina_gpha
    Common
    portability
    unity
  )

  add_executable(test_bit_array "${CMAKE_CURRENT_LIST_DIR}/test_bit_array.c")
  target_link_libraries(test_bit_array
    Common
    portability
    unity
  )

  add_executable(test_num_mgr "${CMAKE_CURRENT_LIST_DIR}/test_num_mgr.c")
  target_link_libraries(test_num_mgr
    Common
    portability
    unity
  )

  add_test(
    NAME test_rina_name
    COMMAND "./test_rina_name"
  )

  add_test(
    NAME test_rina_gpha
    COMMAND "./test_rina_gpha"
  )

  add_test(
    NAME test_bit_array
    COMMAND "./test_bit_array"
  )
endif()
>>>>>>> 8f2bb95 (Ported "Common" unit tests to the Unity on-device test)
