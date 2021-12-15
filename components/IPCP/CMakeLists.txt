idf_component_register(SRCS "factoryIPCP.c" "normalIPCP.c" "IPCP.c"
                    INCLUDE_DIRS "include"
                    REQUIRES configSensor NetworkInterface ShimIPCP BufferManagement ARP826 RMT)

