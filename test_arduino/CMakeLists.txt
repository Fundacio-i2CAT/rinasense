if ("${TARGET_TYPE}" STREQUAL "arduino_esp32")
  add_executable(test_arduino
    test_arduino.cpp
  )
  target_link_libraries(test_arduino PUBLIC
    RINA
    NetworkInterface_Arduino_ESP32
  )
  target_enable_arduino_upload(test_arduino)
else()
  message(FATAL_ERROR "This CMakeLists file is to use with the 'arduino' target only.")
endif()

