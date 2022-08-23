idf_component_register(
  SRCS "ARP826.c"
  INCLUDE_DIRS "include"
  REQUIRES NetworkInterface ShimIPCP BufferManagement configSensor Common Portability RINA_API)
