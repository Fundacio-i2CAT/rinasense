#ifndef _COMMON_RINA_IDS_H
#define _COMMON_RINA_IDS_H

#include <stdint.h>
#include "portability/port.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t invokeId_t;

typedef uint32_t portId_t;

typedef uint32_t seqNum_t;

/* CEPIdLength field 1 Byte*/
typedef uint8_t  cepId_t;

/* QoSIdLength field 1 Byte*/
typedef uint8_t  qosId_t;

/* QoSIdLength field 1 Byte*/
typedef uint8_t address_t;

typedef uint16_t ipcProcessId_t;

#define PORT_ID_WRONG (portId_t)-1
#define CEP_ID_WRONG  (cepId_t)-1
#define ADDRESS_WRONG (address_t)-1
#define QOS_ID_WRONG  (qosId_t)-1
#define IPCP_ID_WRONG (ipcProcessId_t)-1

static inline bool_t is_port_id_ok(portId_t id)
{
    return id != PORT_ID_WRONG;
}

static inline bool_t is_cep_id_ok(cepId_t id)
{
    return id != CEP_ID_WRONG;
}

static inline bool_t is_ipcp_id_ok(ipcProcessId_t id)
{
    return id != IPCP_ID_WRONG;
}

static inline bool_t is_address_ok(address_t address)
{
    return address != ADDRESS_WRONG;
}

static inline bool_t is_qos_id_ok(qosId_t id)
{
    return id != QOS_ID_WRONG;
}

/* Unclear if the definitions below are useful. */

typedef unsigned int uint_t; 
typedef unsigned int timeout_t;
typedef uint16_t ipcpInstanceId_t;

#ifdef __cplusplus
}
#endif

#endif // _COMMON_RINA_IDS_H
