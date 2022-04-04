idf_component_register(SRCS "Rib.c" "Ribd.c"
                    INCLUDE_DIRS "include"
                    REQUIRES BufferManagement ShimIPCP configSensor IPCP CdapProto Enrollment)

