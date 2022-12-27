idf_component_register(
  SRCS
  "Ribd_obj.c"
  "Ribd.c"
  "Ribd_connections.c"
  "CdapMessage.c"
  INCLUDE_DIRS "include"
  REQUIRES FlowAllocator ShimIPCP configSensor IPCP CdapProto Enrollment RINA_API)

