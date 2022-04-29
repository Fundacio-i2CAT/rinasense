idf_component_register(SRCS "ipcpIdm.c" "cepIdm.c" "pidm.c" "IpcManager.c" "factoryIPCP.c" "normalIPCP.c" "IPCP.c" "common.c"
                    INCLUDE_DIRS "include"
                    REQUIRES configSensor NetworkInterface ShimIPCP BufferManagement ARP826 Rmt RINA_API EFCP Enrollment Ribd FlowAllocator)

