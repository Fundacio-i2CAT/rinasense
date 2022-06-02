idf_component_register(SRCS "rmt.c" "pci.c" "du.c"
                    INCLUDE_DIRS "include"
                    REQUIRES IPCP configRINA configSensor EFCP)

