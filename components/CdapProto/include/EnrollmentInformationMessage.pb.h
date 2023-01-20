/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.6 */

#ifndef PB_RINA_MESSAGES_ENROLLMENTINFORMATIONMESSAGE_PB_H_INCLUDED
#define PB_RINA_MESSAGES_ENROLLMENTINFORMATIONMESSAGE_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _rina_messages_enrollmentInformation_t { /* carries information about a member that requests enrollment to a DIF */
    bool has_address;
    uint64_t address;
    pb_size_t supportingDifs_count;
    char supportingDifs[1][20];
    bool has_startEarly;
    bool startEarly;
    bool has_token;
    char token[20]; /* A value that carries a hash */
} rina_messages_enrollmentInformation_t;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define rina_messages_enrollmentInformation_t_init_default {false, 0, 0, {""}, false, 0, false, ""}
#define rina_messages_enrollmentInformation_t_init_zero {false, 0, 0, {""}, false, 0, false, ""}

/* Field tags (for use in manual encoding/decoding) */
#define rina_messages_enrollmentInformation_t_address_tag 1
#define rina_messages_enrollmentInformation_t_supportingDifs_tag 2
#define rina_messages_enrollmentInformation_t_startEarly_tag 3
#define rina_messages_enrollmentInformation_t_token_tag 4

/* Struct field encoding specification for nanopb */
#define rina_messages_enrollmentInformation_t_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, UINT64,   address,           1) \
X(a, STATIC,   REPEATED, STRING,   supportingDifs,    2) \
X(a, STATIC,   OPTIONAL, BOOL,     startEarly,        3) \
X(a, STATIC,   OPTIONAL, STRING,   token,             4)
#define rina_messages_enrollmentInformation_t_CALLBACK NULL
#define rina_messages_enrollmentInformation_t_DEFAULT NULL

extern const pb_msgdesc_t rina_messages_enrollmentInformation_t_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define rina_messages_enrollmentInformation_t_fields &rina_messages_enrollmentInformation_t_msg

/* Maximum encoded size of messages (where known) */
#define rina_messages_enrollmentInformation_t_size 56

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
