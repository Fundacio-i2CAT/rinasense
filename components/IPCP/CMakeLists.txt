idf_component_register(
    SRCS "IpcManager.c" "normalIPCP.c" "IPCP.c" "common.c"
    INCLUDE_DIRS "include"
    REQUIRES configSensor Shim BufferManagement Rmt RINA_API EFCP Enrollment Ribd FlowAllocator)
