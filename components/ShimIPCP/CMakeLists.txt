idf_component_register(
  SRCS "ShimIPCP.c"
  INCLUDE_DIRS "include"
  REQUIRES BufferManagement ARP826 NetworkInterface IPCP configRINA configSensor Rmt)

