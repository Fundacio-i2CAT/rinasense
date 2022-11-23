idf_component_register(
  SRCS "ShimIPCP.c" "ARP826.c" "cache.c" "eth.c"
  INCLUDE_DIRS "include"
  REQUIRES NetworkInterface IPCP configRINA configSensor Rmt)

