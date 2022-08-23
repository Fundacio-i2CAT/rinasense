if("$ENV{TARGET_TYPE}" STREQUAL "freertos_idf")
  idf_component_register(
    SRCS "FlowMessage.pb.c" "ApplicationProcessNamingInfoMessage.pb.c" "QoSSpecification.pb.c" "PolicyDescriptorMessage.pb.c" "ConnectionPoliciesMessage.pb.c" "CommonMessages.pb.c" "CDAP.pb.c" "pb_common.c" "pb_decode.c" "pb_encode.c" "EnrollmentInformationMessage.pb.c" "NeighborMessage.pb.c"
    INCLUDE_DIRS "include")
else()
  rs_plain_component(CdapProto
    DIR     "${CMAKE_CURRENT_LIST_DIR}")
endif()


