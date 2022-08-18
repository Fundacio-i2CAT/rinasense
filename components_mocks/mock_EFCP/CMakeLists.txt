idf_component_register(
  SRCS "efcp.c"
  INCLUDE_DIRS "include"
  REQUIRES Portability Common IPCP mock_FlowAllocator)
