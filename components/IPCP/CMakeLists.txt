<<<<<<< HEAD
idf_component_register(SRCS "rstr.c" "ipcpIdm.c" "cepIdm.c" "pidm.c" "IpcManager.c" "normalIPCP.c" "IPCP.c" "common.c"
    INCLUDE_DIRS "include"
    REQUIRES configSensor NetworkInterface ShimIPCP BufferManagement ARP826 Rmt RINA_API EFCP Enrollment Ribd FlowAllocator)
=======
idf_component_register(
  SRCS "IpcManager.c" "normalIPCP.c" "IPCP.c" "common.c"
  INCLUDE_DIRS "include"
  REQUIRES configSensor NetworkInterface ShimIPCP BufferManagement ARP826 Rmt RINA_API EFCP Enrollment Ribd FlowAllocator)

if("$ENV{USE_MOCKS}" STREQUAL "true")
  target_include_directories(${COMPONENT_LIB} BEFORE PUBLIC
    "${CMAKE_SOURCE_DIR}/../components_mocks/mock_FlowAllocator/include"
    "${CMAKE_SOURCE_DIR}/../components_mocks/mock_Ribd/include"
  )
endif()

>>>>>>> 910101ca13013cc4fdce00c47d2b7544c5f57567
