idf_component_register(SRCS "pci.c" "RMT.c" "du.c"
                    INCLUDE_DIRS "include"
                    REQUIRES IPCP configRINA configSensor EFCP)

