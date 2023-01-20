set(SRCS
  "SerDesFlow.c"
  "SerdesMsg.c"
  "SerDesEnrollment.c"
  "SerDesAData.c"
  "SerDesNeighbor.c"
  "SerDesMessage.c"
)

idf_component_register(
  SRCS
  "SerdesMsg.c"
  "SerDesFlow.c"
  "SerDesEnrollment.c"
  "SerDesAData.c"
  "SerDesNeighbor.c"
  "SerDesMessage.c"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES FlowAllocator
  REQUIRES Common Portability CdapProto FlowAllocator)

# As per: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/build-system.html
set_property(TARGET ${COMPONENT_LIB} APPEND PROPERTY LINK_INTERFACE_MULTIPLICITY 3)
