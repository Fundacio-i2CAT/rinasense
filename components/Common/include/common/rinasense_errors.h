#ifndef _COMMON_RINASENSE_ERRORS_H
#define _COMMON_RINASENSE_ERRORS_H

/*
 * ERROR CODE FORMAT: AABBCCDD
 *
 *   AA -- First level index (ie: Category)
 *   BB -- Extra (pthread or errno)
 *   CC -- Free
 *   DD -- Second level index (ie: specific error)
 */

/* Basic or general errors */
#define EC_BASE           _MK_CAT(0x00)
#define ERR_ERR           _MK_ERR(EC_BASE, 0x01) /* Generic error */
#define ERR_OOM           _MK_ERR(EC_BASE, 0x02) /* Out of memory */
#define ERR_BAD_ARG       _MK_ERR(EC_BASE, 0x03) /* Invalid argument */
#define ERR_PTHREAD       _MK_ERR(EC_BASE, 0x04) /* pthread error */
#define ERR_ERRNO         _MK_ERR(EC_BASE, 0x05) /* errno */
#define ERR_OVERFLOW      _MK_ERR(EC_BASE, 0x06) /* Target resource full */
#define ERR_UNDERFLOW     _MK_ERR(EC_BASE, 0x07) /* Target res. empty */
#define ERR_TIMEDOUT      _MK_ERR(EC_BASE, 0x08) /* Generic timeout */

/* GPA/GHA specific errors */
#define EC_GPA            _MK_CAT(0x01)
#define ERR_GPA_INVALID   _MK_ERR(EC_GPA, 0x01)
#define ERR_GHA_INVALID   _MK_ERR(EC_GPA, 0x02)

/* Netbufs */
#define EC_NETBUF             _MK_CAT(0x02)
#define ERR_NETBUF_SPLIT_FAIL _MK_ERR(EC_NETBUF, 0x01)

/* SerDes */
#define EC_SERDES                _MK_CAT(0x03)
#define ERR_SERDES_ENCODING_FAIL _MK_ERR(EC_SERDES, 0x01)
#define ERR_SERDES_DECODING_FAIL _MK_ERR(EC_SERDES, 0x02)

/* RIB */
#define EC_RIB                       _MK_CAT(0x04)
#define ERR_RIB_TABLE_FULL           _MK_ERR(EC_RIB, 0x01)
#define ERR_RIB_NOT_PENDING          _MK_ERR(EC_RIB, 0x02)
#define ERR_RIB_NEIGHBOR_NOT_FOUND   _MK_ERR(EC_RIB, 0x03)
#define ERR_RIB_TOO_MANY_CONNECTIONS _MK_ERR(EC_RIB, 0x04)
#define ERR_RIB_NO_SUCH_CONNECTION   _MK_ERR(EC_RIB, 0x05)
#define ERR_RIB_OBJECT_UNSUPPORTED   _MK_ERR(EC_RIB, 0x06)
#define ERR_RIB_OBJECT_UNSUP_METHOD  _MK_ERR(EC_RIB, 0x07)
#define ERR_RIB_BAD_CONNECTION_STATE _MK_ERR(EC_RIB, 0x08)
#define ERR_RIB_CONNECTION_EXISTS    _MK_ERR(EC_RIB, 0x09)
#define ERR_RIB_TIMEOUT              _MK_ERR(EC_RIB, 0x10)

/* Enrollment */
#define EC_ENROLLMENT                _MK_CAT(0x05)
#define ERR_ENROLLMENT_INBOUND_FAIL  _MK_ERR(EC_ENROLLMENT, 0x01)

/* Error messages array */
extern char **ErrorMessages[];

#endif /* _COMMON_RINASENSE_ERRORS_H */
