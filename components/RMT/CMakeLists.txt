idf_component_register(SRCS "pci.c" "Rmt.c" "du.c"
                    INCLUDE_DIRS "include"
                    REQUIRES IPCP configRINA configSensor EFCP)

