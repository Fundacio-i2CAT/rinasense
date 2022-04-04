idf_component_register(SRCS "SerdesMsg.c" "Enrollment.c" "SerdesMsg"
                    INCLUDE_DIRS "include"
                    REQUIRES IPCP BufferManagement Ribd CdapProto)

