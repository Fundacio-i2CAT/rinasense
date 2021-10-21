idf_component_register(SRCS "IPCP.c"
                    INCLUDE_DIRS "include"
                    REQUIRES configSensor NetworkInterface ShimIPCP BufferManagement ARP826 RMT)

