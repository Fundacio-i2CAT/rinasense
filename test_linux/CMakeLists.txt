if (${TARGET_TYPE} STREQUAL "linux")
  set(NETWORK_INTERFACE_MODULE "linux_tap")

  add_executable(test_linux
    test_linux.c
  )
  target_link_libraries(test_linux PUBLIC
    ShimIPCP
    IPCP
    NetworkInterface_TAP
    configSensor
    BufferManagement
    ARP826
    Rmt
    EFCP
    Enrollment
    Ribd
    FlowAllocator
  )
else()
  message(FATAL_ERROR "This CMakeLists file is to use with the 'linux' target only.")
endif()
