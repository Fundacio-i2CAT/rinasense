set(SRCS
  "SerdesMsg.c"
  "SerDesFlow.c"
  "SerDesEnrollment.c"
  "SerDesAData.c"
  "SerDesNeighbor.c"
  "SerDesMessage.c"
)

idf_component_register(
  SRCS ${SRCS}
  INCLUDE_DIRS "include"
  REQUIRES IPCP CdapProto)
