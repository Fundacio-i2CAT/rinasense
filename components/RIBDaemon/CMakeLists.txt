idf_component_register(SRCS "RIBDaemon.c"
                    INCLUDE_DIRS "include"
                    REQUIRES BufferManagement ShimIPCP configSensor IPCP CdapProto)

