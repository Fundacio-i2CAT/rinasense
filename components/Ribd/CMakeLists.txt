idf_component_register(
  SRCS "Rib.c" "Ribd.c"
  INCLUDE_DIRS "include"
  REQUIRES FlowAllocator BufferManagement ShimIPCP configSensor IPCP CdapProto Enrollment RINA_API)

