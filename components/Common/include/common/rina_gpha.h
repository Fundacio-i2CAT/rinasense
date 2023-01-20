#ifndef _COMMON_RINA_GPA_H
#define _COMMON_RINA_GPA_H

#include <stdint.h>

#include "error.h"
#include "rina_name.h"
#include "mac.h"
#include "configRINA.h"

#ifdef __cplusplus
extern "C" {
#endif

//Structure Generic Protocol Address
typedef struct xGENERIC_PROTOCOL_ADDRESS
{
	buffer_t pucAddress;
	size_t uxLength;
} gpa_t;

//Structure Generic Hardware Address
typedef struct xGENERIC_HARDWARE_ADDRESS
{
	eGHAType_t 		xType;
	MACAddress_t 	xAddress;
} gha_t;

/* @brief Create a generic protocol address based on an string address*/
gpa_t *pxCreateGPA(const buffer_t pucAddress, size_t xLength);

/* @brief Create a generic hardware address based on the MAC address*/
gha_t *pxCreateGHA(eGHAType_t xType, const MACAddress_t *pxAddress);

gpa_t *pxDupGPA(const gpa_t *pxSourcePa, bool_t nDoShrink, uint8_t ucFiller);

gha_t *pxDupGHA(const gha_t *pxSourceHa);

rsErr_t xGPAShrink(gpa_t *pxGpa, uint8_t ucFiller);

rsErr_t xGPAGrow(gpa_t *pxGpa, size_t uxLength, uint8_t ucFiller);

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

bool_t xGHACmp(const gha_t *pxHa1, const gha_t *pxHa2);

gpa_t *pxNameToGPA(const rname_t *xLocalInfo);

char* xGPAAddressToString(const gpa_t * pxGpa);

#ifdef __cplusplus
}
#endif

#endif // _COMMON_RINA_GPA_H
