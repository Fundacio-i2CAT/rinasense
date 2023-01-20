#include <string.h>

#include "common/rinasense_errors.h"
#include "portability/port.h"

#include "common/error.h"
#include "common/mac.h"
#include "common/rina_gpha.h"

/* No used check if this is necessary */
char *xGPAAddressToString(const gpa_t *pxGpa)
{
	char *tmp, *p;

	if (!xIsGPAOK(pxGpa))
		return NULL;

	tmp = pvRsMemAlloc(pxGpa->uxLength + 1);
	if (!tmp)
		return NULL;

	memcpy(tmp, pxGpa->pucAddress, pxGpa->uxLength);

	p = tmp + pxGpa->uxLength;
	*(p) = '\0';

	return tmp;
}

gpa_t *pxNameToGPA(const rname_t *pxName)
{
	gpa_t *pxGpa;
	string_t pcTmp;

    RsAssert(pxName);

	if (!(pcTmp = pcNameToString(pxName)))
		return NULL;

	// Convert the IPCPAddress Concatenated to bits
	pxGpa = pxCreateGPA((buffer_t)pcTmp, strlen(pcTmp) + 1);

	if (!pxGpa)	{
		vRsMemFree(pcTmp);
		return NULL;
	}

	vRsMemFree(pcTmp);

	return pxGpa;
}

buffer_t pucCreateAddress(size_t uxLength)
{
	buffer_t pucAddress;

	pucAddress = pvRsMemAlloc(uxLength);
    memset(pucAddress, 0, uxLength);

	return pucAddress;
}

/** Create a GPA object from a buffer
 *
 * @param pucAddress Address of the buffer
 *
 */
gpa_t *pxCreateGPA(const buffer_t pucAddress, size_t uxLength)
{
	gpa_t *pxGPA;

	if (!pucAddress || uxLength == 0)
		return ERR_SET_NULL(ERR_BAD_ARG);

	if (!(pxGPA = pvRsMemAlloc(sizeof(*pxGPA))))
		return ERR_SET_OOM_NULL;

	pxGPA->uxLength = uxLength;
	pxGPA->pucAddress = pucCreateAddress(uxLength + 1);

	if (!pxGPA->pucAddress)	{
		vRsMemFree(pxGPA);
		return ERR_SET_OOM_NULL;
	}

	memcpy(pxGPA->pucAddress, pucAddress, pxGPA->uxLength);

    if (!xIsGPAOK(pxGPA)) {
        vRsMemFree(pxGPA);
        return ERR_SET_OOM_NULL;
    }

	return pxGPA;
}

rsErr_t xGPAShrink(gpa_t *pxGpa, uint8_t ucFiller)
{
    buffer_t pucNewAddress;
    buffer_t pucPosition;
    size_t uxLength;

    if (!xIsGPAOK(pxGpa))
        return ERR_SET(ERR_GPA_INVALID);

    /* Look for the filler character in the address */
    pucPosition = memchr(pxGpa->pucAddress, ucFiller, pxGpa->uxLength);

    /* Check if there is any needs to shrink */
    if (pucPosition == NULL || pucPosition >= pxGpa->pucAddress + pxGpa->uxLength)
        return SUCCESS;

    uxLength = pucPosition - pxGpa->pucAddress;

    if (!(pucNewAddress = pvRsMemAlloc(uxLength)))
        return ERR_SET_OOM;

    memcpy(pucNewAddress, pxGpa->pucAddress, uxLength);

    vRsMemFree(pxGpa->pucAddress);
    pxGpa->pucAddress = pucNewAddress;
    pxGpa->uxLength = uxLength;

    return SUCCESS;
}

rsErr_t xGPAGrow(gpa_t *pxGpa, size_t uxLength, uint8_t ucFiller)
{
    buffer_t new_address;

    if (!xIsGPAOK(pxGpa))
        return ERR_SET(ERR_GPA_INVALID);

    if (uxLength == 0 || uxLength < pxGpa->uxLength)
        return ERR_SET(ERR_BAD_ARG);

    if (pxGpa->uxLength == uxLength)
        return SUCCESS;

    RsAssert(uxLength > pxGpa->uxLength);

    if (!(new_address = pvRsMemAlloc(uxLength)))
        return ERR_SET_OOM;

    memcpy(new_address, pxGpa->pucAddress, pxGpa->uxLength);
    memset(new_address + pxGpa->uxLength, ucFiller, uxLength - pxGpa->uxLength);
    vRsMemFree(pxGpa->pucAddress);
    pxGpa->pucAddress = new_address;
    pxGpa->uxLength = uxLength;

    return SUCCESS;
}

gpa_t *pxDupGPA(const gpa_t *pxSourcePa, bool_t nDoShrink, uint8_t ucFiller)
{
    gpa_t *pxTargetPa = NULL;
    buffer_t c;

    RsAssert(pxSourcePa);

    if (!xIsGPAOK(pxSourcePa))
        return ERR_SET_NULL(ERR_GPA_INVALID);

    if (!(pxTargetPa = pvRsMemAlloc(sizeof(gpa_t))))
        return ERR_SET_NULL(ERR_GPA_INVALID);

    if (nDoShrink) {
        pxTargetPa->uxLength = pxSourcePa->uxLength;
        for (c = pxSourcePa->pucAddress + pxSourcePa->uxLength; *(--c) == ucFiller;)
            pxTargetPa->uxLength--;
    } else
        pxTargetPa->uxLength = pxSourcePa->uxLength;

    if (!(pxTargetPa->pucAddress = pvRsMemAlloc(pxSourcePa->uxLength))) {
        vRsMemFree(pxTargetPa);
        return ERR_SET_OOM_NULL;
    }

    memcpy(pxTargetPa->pucAddress, pxSourcePa->pucAddress, pxTargetPa->uxLength);

    return pxTargetPa;
}

gha_t *pxCreateGHA(eGHAType_t xType, const MACAddress_t *pxAddress)
{
	gha_t *pxGha;

	if (xType != MAC_ADDR_802_3)
        return ERR_SET_NULL(ERR_GHA_INVALID);

    if (!(pxGha = pvRsMemAlloc(sizeof(*pxGha))))
		return ERR_SET_OOM_NULL;

	pxGha->xType = xType;
	memcpy(pxGha->xAddress.ucBytes, pxAddress->ucBytes, sizeof(pxGha->xAddress));

	return pxGha;
}

/* Return a duplicate of the source hardware address object */
gha_t *pxDupGHA(const gha_t *pxSourceHa)
{
    gha_t *pxTargetHa;

    RsAssert(pxSourceHa);

    if (!xIsGHAOK(pxSourceHa)) {
        ERR_SET(ERR_GHA_INVALID);
        return NULL;
    }

    RsAssert(pxSourceHa->xType == MAC_ADDR_802_3);

    if ((pxTargetHa = pvRsMemAlloc(sizeof(gha_t))) == NULL)
        return ERR_SET_OOM_NULL;

    memcpy(&pxTargetHa->xAddress, &pxSourceHa->xAddress, sizeof(MACAddress_t));
    pxTargetHa->xType = pxSourceHa->xType;

    return pxTargetHa;
}

bool_t xIsGPAOK(const gpa_t *pxGpa)
{
	if (!pxGpa)
		return false;

	if (pxGpa->pucAddress == NULL)
		return false;

	if (pxGpa->uxLength == 0)
		return false;

	return true;
}

bool_t xIsGHAOK(const gha_t *pxGha)
{
	if (!pxGha)
		return false;

	if (pxGha->xType != MAC_ADDR_802_3)
		return false;

    return true;
}

bool_t xGHACmp(const gha_t *pxHa1, const gha_t *pxHa2)
{
    if (!pxHa1 || !pxHa2)
        return false;

    return memcmp(pxHa1->xAddress.ucBytes, pxHa2->xAddress.ucBytes, sizeof(MACAddress_t)) == 0;
}

/* This returns the actual 'useful' length of the PA, which is without
 * filler character. */
static size_t prvGetActualAddressLength(const gpa_t *pxPa)
{
    for (size_t n = 0; n < pxPa->uxLength; n++)
        if (!(*(pxPa->pucAddress + n)))
            return n;
    return pxPa->uxLength;
}

bool_t xGPACmp(const gpa_t *gpa1, const gpa_t *gpa2)
{
    size_t n1, n2;

    if (!(xIsGPAOK(gpa1) && xIsGPAOK(gpa2)))
        return false;

    n1 = prvGetActualAddressLength(gpa1);
    n2 = prvGetActualAddressLength(gpa2);

    if (n1 != n2) return false;

    return memcmp(gpa1->pucAddress, gpa2->pucAddress, n1) == 0;
}

void vGPADestroy(gpa_t *pxGpa)
{
	if (!xIsGPAOK(pxGpa))
		return;

	vRsMemFree(pxGpa->pucAddress);

    /* Invalidate the GPA so as to not accidentally reuse one that was
     * freed. */
    /* FIXME: Make this part of debug builds only. */
    memset(pxGpa, 0, sizeof(gpa_t));

	vRsMemFree(pxGpa);

	return;
}

void vGHADestroy(gha_t *pxGha)
{
	if (!xIsGHAOK(pxGha))
		return;

#ifndef NDEBUG_
    /* Invalid the GHA so as to not accidentally reuse one that was
     * freed. */
    memset(pxGha, 0, sizeof(gha_t));
#endif

	vRsMemFree(pxGha);

	return;
}
