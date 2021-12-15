
#include "Freertos/FreeRTOS.h"

portId_t port_id_bad(void)
{ return PORT_ID_WRONG; }


BaseType_t is_port_id_ok(port_id_t id)
{ return id >= 0 ? pdTRUE : pdFALSE; }


BaseType_t is_cep_id_ok(cep_id_t id)
{ return id >= 0 ? pdTRUE : pdFALSE; }


cepId_t cep_id_bad(void)
{ return CEP_ID_WRONG; }


BaseType_t is_address_ok(address_t address)
{ return address != ADDRESS_WRONG ? pdTRUE : pdFALSE; }


address_t address_bad(void)
{ return ADDRESS_WRONG; }


qosId_t qos_id_bad(void)
{ return QOS_ID_WRONG; }


BaseType_t is_qos_id_ok(qos_id_t id)
{ return id != QOS_ID_WRONG ? pdTRUE : pdFALSE; }
