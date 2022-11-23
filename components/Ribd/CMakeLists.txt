idf_component_register(
  SRCS "Rib.c" "Ribd.c" "CdapMessage.c"
  INCLUDE_DIRS "include"
  REQUIRES FlowAllocator ShimIPCP configSensor IPCP CdapProto Enrollment RINA_API)

