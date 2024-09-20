set(reqs
  Rina-manager
  Portability
)

idf_component_register(
  SRCS "Arp826.c"
  INCLUDE_DIRS "include"
  REQUIRES ${reqs})
