/*
 * pci.h
 *
 *  Created on: 1 oct. 2021
 *      Author: i2CAT
 */

#ifndef COMPONENTS_RMT_INCLUDE_PCI_H_
#define COMPONENTS_RMT_INCLUDE_PCI_H_


/**** Constants PCI dataTypes***/



/* CEPIdLength field 1 Byte*/
typedef uint8_t  cepId_t;

/* QoSIdLength field 1 Byte*/
typedef uint8_t  qosId_t;

/* QoSIdLength field 1 Byte*/
typedef uint8_t address_t;

/* SeqNumLength field 4 Byte*/
typedef uint32_t seqNum_t;

/* PDU-Flags field 1 Byte*/
typedef uint8_t pduFlags_t;

/* PDU-Type field 1 Byte*/
typedef uint8_t pduType_t;


#define PDU_FLAGS_EXPLICIT_CONGESTION 0x01
#define PDU_FLAGS_DATA_RUN            0x80
/* To be truely defined; internal to stack, needs to be discussed */
#define PDU_FLAGS_BAD                 0xFF



/* Data Transfer PDUs */
#define PDU_TYPE_DT            0x80 /* Data Transfer PDU */
/* Control PDUs */
#define PDU_TYPE_CACK          0xC0 /* Control Ack PDU */
/* Ack/Flow Control PDUs */
#define PDU_TYPE_ACK           0xC1 /* ACK only */
#define PDU_TYPE_NACK          0xC2 /* Forced Retransmission PDU (NACK) */
#define PDU_TYPE_FC            0xC4 /* Flow Control only */
#define PDU_TYPE_ACK_AND_FC    0xC5 /* ACK and Flow Control */
#define PDU_TYPE_NACK_AND_FC   0xC6 /* NACK and Flow Control */
/* Selective Ack/Nack PDUs */
#define PDU_TYPE_SACK          0xC9 /* Selective ACK */
#define PDU_TYPE_SNACK         0xCA /* Selective NACK */
#define PDU_TYPE_SACK_AND_FC   0xCD /* Selective ACK and Flow Control */
#define PDU_TYPE_SNACK_AND_FC  0xCE /* Selective NACK and Flow Control */
/* Rendezvous PDU */
#define PDU_TYPE_RENDEZVOUS    0xCF /* Rendezvous */
/* Management PDUs */
#define PDU_TYPE_MGMT          0x40 /* Management */
/* Number of different PDU types */
#define PDU_TYPES              12

#define ADDRESS_WRONG -1
#define QOS_ID_WRONG -1



BaseType_t is_address_ok(address_t address)
{ return address != ADDRESS_WRONG ? pdTRUE : pdFALSE; }

BaseType_t is_qos_id_ok(qosId_t id)
{ return id != QOS_ID_WRONG ? pdTRUE :  pdFALSE; }

BaseType_t is_cep_id_ok(cepId_t id)
{ return id >= 0 ? true : false; }

#define pdu_type_is_ok(X)                                \
	((X == PDU_TYPE_DT)         ? pdTRUE :             \
	 ((X == PDU_TYPE_CACK)       ? pdTRUE :            \
	  ((X == PDU_TYPE_SACK)       ? pdTRUE :           \
	   ((X == PDU_TYPE_NACK)       ? pdTRUE :          \
	    ((X == PDU_TYPE_FC)         ? pdTRUE :         \
	     ((X == PDU_TYPE_ACK)        ? pdTRUE :        \
	      ((X == PDU_TYPE_ACK_AND_FC) ? pdTRUE :       \
	       ((X == PDU_TYPE_SACK_AND_FC) ? pdTRUE :     \
		((X == PDU_TYPE_SNACK_AND_FC) ? pdTRUE :   \
		 ((X == PDU_TYPE_RENDEZVOUS)   ? pdTRUE :  \
		  ((X == PDU_TYPE_MGMT)         ? pdTRUE : \
				  pdFALSE)))))))))))

#define pdu_type_is_control(X)                           \
	((X == PDU_TYPE_CACK)       ? true :             \
	 ((X == PDU_TYPE_SACK)       ? true :            \
	  ((X == PDU_TYPE_NACK)       ? true :           \
	   ((X == PDU_TYPE_FC)         ? true :          \
	    ((X == PDU_TYPE_ACK)        ? true :         \
	     ((X == PDU_TYPE_ACK_AND_FC) ? true :        \
	      ((X == PDU_TYPE_SACK_AND_FC) ? true :      \
	       ((X == PDU_TYPE_RENDEZVOUS)  ? true :     \
	        ((X == PDU_TYPE_SNACK_AND_FC) ? true :   \
		 false)))))))))
#if 0
typedef struct xPCI {
	uint8_t * pucH; /* do not move from 1st position */
	size_t uxLen;
}pci_t;
#endif

typedef struct xPCI{

	uint8_t 	ucVersion;

 	address_t   xDestination;
 	address_t   xSource;

 	struct {
 		qosId_t xQosId;
 		cepId_t xDestination;
 		cepId_t xSource;
 	} connectionId_t;

 	pduType_t  xType;
 	pduFlags_t xFlags;
 	uint16_t   xPduLen;
 	seqNum_t   xSequenceNumber;


 	/*struct {
 		seqNum_t last_ctrl_seq_num_rcvd;
 		seqNum_t ack_nack_seq_num;
 		seqNum_t new_rt_wind_edge;
 		seqNum_t new_lf_wind_edge;
 		seqNum_t my_lf_wind_edge;
 		seqNum_t my_rt_wind_edge;
 		u_int32_t sndr_rate;
 		u_int32_t time_frame;
 	} control;*/
 }pci_t;

#endif /* COMPONENTS_RMT_INCLUDE_PCI_H_ */
