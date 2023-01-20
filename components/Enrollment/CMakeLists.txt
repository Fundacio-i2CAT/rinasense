idf_component_register(
  SRCS "Enrollment.c" "neighbor.c" "Enrollment_Inbound.c"
  INCLUDE_DIRS "include"
  REQUIRES IPCP Ribd CdapProto FlowAllocator SerDes)
