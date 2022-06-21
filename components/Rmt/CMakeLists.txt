idf_component_register(SRCS "Rmt.c" "pci.c" "du.c"
                    INCLUDE_DIRS "include"
                    REQUIRES IPCP configRINA configSensor EFCP Common)

