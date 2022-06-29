idf_component_register(SRCS "esp32/NetworkInterface.c"
  INCLUDE_DIRS "include"
  REQUIRES BufferManagement ARP826 ShimIPCP configSensor mock_IPCP)
