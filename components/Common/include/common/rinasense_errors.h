#ifndef _COMMON_RINASENSE_ERRORS_H
#define _COMMON_RINASENSE_ERRORS_H

/* Basic or general errors */
#define ERR_BASE          0x00000000
#define ERR_ERR           _MK_ERR(ERR_BASE | 0x01) /* Generic error */
#define ERR_OOM           _MK_ERR(ERR_BASE | 0x02) /* Out of memory */
#define ERR_BAD_ARG       _MK_ERR(ERR_BASE | 0x03) /* Invalid argument */
#define ERR_PTHREAD       _MK_ERR(ERR_BASE | 0x04) /* pthread error */

/* GPA/GHA specific errors */
#define ERR_GPA_BASE      0x01000000
#define ERR_GPA_INVALID   _MK_ERR(ERR_GPA_BASE | 0x01)
#define ERR_GHA_INVALID   _MK_ERR(ERR_GPA_BASE | 0x02)

/* Netbufs */
#define ERR_NETBUF_BASE       0x02000000
#define ERR_NETBUF_SPLIT_FAIL _MK_ERR(ERR_NETBUF_BASE | 0x01)

/* SerDes */
#define ERR_SERDES_BASE          0x03000000
#define ERR_SERDES_ENCODING_FAIL _MK_ERR(ERR_SERDES_BASE | 0x01)
#define ERR_SERDES_DECODING_FAIL _MK_ERR(ERR_SERDES_BASE | 0x02)

/* RIB */
#define ERR_RIB_BASE                 0x04000000
#define ERR_RIB_TABLE_FULL           _MK_ERR(ERR_RIB_BASE | 0x01)
#define ERR_RIB_NOT_PENDING          _MK_ERR(ERR_RIB_BASE | 0x02)
#define ERR_RIB_NEIGHBOR_NOT_FOUND   _MK_ERR(ERR_RIB_BASE | 0x03)
#define ERR_RIB_TOO_MANY_CONNECTIONS _MK_ERR(ERR_RIB_BASE | 0x04)
#define ERR_RIB_NO_SUCH_CONNECTION   _MK_ERR(ERR_RIB_BASE | 0x05)
#define ERR_RIB_OBJECT_UNSUPPORTED   _MK_ERR(ERR_RIB_BASE | 0x06)
#define ERR_RIB_OBJECT_UNSUP_METHOD  _MK_ERR(ERR_RIB_BASE | 0x07)
#define ERR_RIB_BAD_CONNECTION_STATE _MK_ERR(ERR_RIB_BASE | 0x08)
#define ERR_RIB_CONNECTION_EXISTS    _MK_ERR(ERR_RIB_BASE | 0x09)
#define ERR_RIB_TIMEOUT              _MK_ERR(ERR_RIB_BASE | 0x10)

/* Enrollment */
#define ERR_ENROLLMENT_BASE          0x05000000
#define ERR_ENROLLMENT_INBOUND_FAIL  _MK_ERR(ERR_ENROLLMENT_BASE | 0x01)

/* Error messages array */
extern char **ErrorMessages[];

#endif /* _COMMON_RINASENSE_ERRORS_H */
