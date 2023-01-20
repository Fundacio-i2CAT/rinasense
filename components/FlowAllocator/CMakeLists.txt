idf_component_register(
  SRCS "FlowAllocator.c"
  INCLUDE_DIRS "include"
  REQUIRES SerDes IPCP Ribd CdapProto Rmt)
