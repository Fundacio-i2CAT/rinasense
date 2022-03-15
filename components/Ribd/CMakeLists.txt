idf_component_register(SRCS "Ribd.c"
                    INCLUDE_DIRS "include"
                    REQUIRES BufferManagement ShimIPCP configSensor IPCP CdapProto Enrollment)

