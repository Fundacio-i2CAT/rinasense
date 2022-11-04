idf_component_register(
  SRCS "SerdesMsg.c" "SerDesFlow.c" "SerDesEnrollment.c" "SerDesAData.c" "SerDesNeighbor.c" "SerDesMessage.c" "Enrollment.c"
  INCLUDE_DIRS "include"
  REQUIRES IPCP BufferManagement Ribd CdapProto FlowAllocator)
