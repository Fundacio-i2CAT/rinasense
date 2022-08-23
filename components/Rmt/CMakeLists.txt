if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(
    SRCS "Rmt.c" "pci.c" "du.c"
    INCLUDE_DIRS "include"
    REQUIRES IPCP configRINA configSensor EFCP Common)
else()
  rs_plain_component(Rmt
    DIR       "${CMAKE_CURRENT_LIST_DIR}"
    REQUIRES  Common BufferManagement EFCP Portability IPCP)
endif()
