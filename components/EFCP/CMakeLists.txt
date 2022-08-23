idf_component_register(
  SRCS "connection.c" "delim.c" "EFCP.c" "dtp.c" "delim.c"
  INCLUDE_DIRS "include"
  REQUIRES IPCP Rmt FlowAllocator Portability)
