#ifndef _COMMON_RINA_GPA_H
#define _COMMON_RINA_GPA_H

#include <stdint.h>

#include "rina_name.h"
#include "configSensor.h"

//Structure MAC ADDRESS
typedef struct xMAC_ADDRESS
{
	uint8_t ucBytes[ MAC_ADDRESS_LENGTH_BYTES ]; /**< Byte array of the MAC address */
} MACAddress_t;

//enum MAC Address
typedef enum {
    MAC_ADDR_802_3
} eGHAType_t;

//Structure Generic Protocol Address
typedef struct xGENERIC_PROTOCOL_ADDRESS
{
	string_t 	pucAddress;
	size_t		uxLength;
} gpa_t;

//Structure Generic Hardware Address
typedef struct xGENERIC_HARDWARE_ADDRESS
{
	eGHAType_t 		xType;
	MACAddress_t 	xAddress;
} gha_t;

/* @brief Create a generic protocol address based on an string address*/
gpa_t *pxCreateGPA(const char *pucAddress, size_t xLength);

/* @brief Create a generic hardware address based on the MAC address*/
gha_t *pxCreateGHA(eGHAType_t xType, const MACAddress_t *pxAddress);

/* @brief Check if a generic protocol Address was created correctly*/
bool_t xIsGPAOK(const gpa_t *pxGpa);

/* @brief Check if a generic hardware Address was created correctly*/
bool_t xIsGHAOK(const gha_t *pxGha);

/* @brief Destroy a generic protocol Address to clean up*/
void vGPADestroy(gpa_t *pxGpa );

/* @brief Destroy a generic hardware Address to clean up*/
void vGHADestroy(gha_t *pxGha);

/* Compares two GPA structures, return true if equal, false if not. */
bool_t xGPACmp(const gpa_t *gpa1, const gpa_t *gpa2);

gpa_t *pxNameToGPA(const name_t *xLocalInfo);

string_t xGPAAddressToString(const gpa_t * pxGpa);

#endif // _COMMON_RINA_GPA_H
