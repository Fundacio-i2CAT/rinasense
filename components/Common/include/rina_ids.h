#ifndef _COMMON_RINA_IDS_H
#define _COMMON_RINA_IDS_H

#include <stdint.h>
#include "portability/port.h"

#define PORT_ID_WRONG -1
#define CEP_ID_WRONG -1
#define ADDRESS_WRONG -1
#define QOS_ID_WRONG -1

typedef int32_t  portId_t;

typedef uint32_t seqNum_t;

/* CEPIdLength field 1 Byte*/
typedef uint8_t  cepId_t;

/* QoSIdLength field 1 Byte*/
typedef uint8_t  qosId_t;

/* QoSIdLength field 1 Byte*/
typedef uint8_t address_t;

typedef uint16_t ipcProcessId_t;

typedef uint16_t ipcpInstanceId_t;

typedef unsigned int uint_t; // WHY?
typedef unsigned int timeout_t;

static inline portId_t port_id_bad(void)
{
        return PORT_ID_WRONG;
}

static inline bool_t is_port_id_ok(portId_t id)
{
        return id >= 0 ? true : false;
}

static inline bool_t is_cep_id_ok(cepId_t id)
{
        return id >= 0 ? true : false;
}

static inline bool_t is_ipcp_id_ok(ipcProcessId_t id)
{
        return id >= 0 ? true : false;
}

static inline bool_t is_address_ok(address_t address)
{
        return address != ADDRESS_WRONG ? true : false;
}

static inline bool_t is_qos_id_ok(qosId_t id)
{
        return id != QOS_ID_WRONG ? true : false;
}

/* Kept for porting, but, really, are those necessary? */

static inline cepId_t cep_id_bad(void)
{
        return CEP_ID_WRONG;
}

static inline address_t address_bad(void)
{
        return ADDRESS_WRONG;
}

static inline qosId_t qos_id_bad(void)
{
        return QOS_ID_WRONG;
}


#endif // _COMMON_RINA_IDS_H
