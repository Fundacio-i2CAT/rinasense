set(srcs
  "FlowMessage.pb.c"
  "ApplicationProcessNamingInfoMessage.pb.c"
  "QoSSpecification.pb.c"
  "PolicyDescriptorMessage.pb.c"
  "ConnectionPoliciesMessage.pb.c"
  "CommonMessages.pb.c"
  "CDAP.pb.c"
  "pb_common.c"
  "pb_decode.c"
  "pb_encode.c"
  "EnrollmentInformationMessage.pb.c"
  "NeighborMessage.pb.c")

idf_component_register(
  SRCS ${srcs}
  INCLUDE_DIRS "include")


