set(reqs
  EFCP
  Rmt
  NetworkInterface
  ShimIPCP
  BufferManagement
  configSensor
  Common
  Portability
  RINA_API
  SerDes
  IPCP)

idf_component_register(
  SRCS "ARP826.c" "cache.c"
  INCLUDE_DIRS "include"
  REQUIRES ${reqs})
