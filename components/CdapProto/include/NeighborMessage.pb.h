/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.6-dev */

#ifndef PB_RINA_MESSAGES_NEIGHBORMESSAGE_PB_H_INCLUDED
#define PB_RINA_MESSAGES_NEIGHBORMESSAGE_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _rina_messages_neighbor_t { /* carries information about a neighbor */
    bool has_apname;
    char apname[20]; 
    bool has_apinstance;
    char apinstance[20]; 
    bool has_aename;
    char aename[20]; 
    bool has_aeinstance;
    char aeinstance[20]; 
    bool has_address;
    uint64_t address; 
    pb_size_t supportingDifs_count;
    char supportingDifs[1][20]; 
} rina_messages_neighbor_t;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define rina_messages_neighbor_t_init_default    {false, "", false, "", false, "", false, "", false, 0, 0, {""}}
#define rina_messages_neighbor_t_init_zero       {false, "", false, "", false, "", false, "", false, 0, 0, {""}}

/* Field tags (for use in manual encoding/decoding) */
#define rina_messages_neighbor_t_apname_tag      1
#define rina_messages_neighbor_t_apinstance_tag  2
#define rina_messages_neighbor_t_aename_tag      3
#define rina_messages_neighbor_t_aeinstance_tag  4
#define rina_messages_neighbor_t_address_tag     5
#define rina_messages_neighbor_t_supportingDifs_tag 6

/* Struct field encoding specification for nanopb */
#define rina_messages_neighbor_t_FIELDLIST(X, a) \
X(a, STATIC,   OPTIONAL, STRING,   apname,            1) \
X(a, STATIC,   OPTIONAL, STRING,   apinstance,        2) \
X(a, STATIC,   OPTIONAL, STRING,   aename,            3) \
X(a, STATIC,   OPTIONAL, STRING,   aeinstance,        4) \
X(a, STATIC,   OPTIONAL, UINT64,   address,           5) \
X(a, STATIC,   REPEATED, STRING,   supportingDifs,    6)
#define rina_messages_neighbor_t_CALLBACK NULL
#define rina_messages_neighbor_t_DEFAULT NULL

extern const pb_msgdesc_t rina_messages_neighbor_t_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define rina_messages_neighbor_t_fields &rina_messages_neighbor_t_msg

/* Maximum encoded size of messages (where known) */
#define rina_messages_neighbor_t_size            117

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif