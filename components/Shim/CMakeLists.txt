if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  set(Manager_INCLUDES
    "${COMMON_INCLUDES}"
    "${CMAKE_SOURCE_DIR}/components/Shim/Arp826/include"
    "${CMAKE_SOURCE_DIR}/components/Shim/include"
    "${CMAKE_SOURCE_DIR}/components/Shim/NetworkInterface/include")

  idf_component_register(
    SRCS

    "Arp826/Arp826.c"
    "shim.c"
    "NetworkInterface/esp32/NetworkInterface.c"

    INCLUDE_DIRS ${Manager_INCLUDES}
    REQUIRES BufferManagement IPCP configRINA configSensor Rmt)
else()
  include(${CMAKE_CURRENT_LIST_DIR}/${TARGET_TYPE}/CMakeLists.txt)
endif()
