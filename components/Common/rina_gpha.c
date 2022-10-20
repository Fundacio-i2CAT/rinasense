#include <string.h>

#include "common/mac.h"
#include "common/rina_gpha.h"
#include "linux_rsmem.h"
#include "portability/port.h"

/* No used check if this is necessary */
char *xGPAAddressToString(const gpa_t *pxGpa)
{
	char *tmp, *p;

	if (!xIsGPAOK(pxGpa))
	{
		LOGE(TAG_ARP, "Bad input parameter, "
             "cannot get a meaningful address from GPA");
		return NULL;
	}

	tmp = pvRsMemAlloc(pxGpa->uxLength + 1);
	if (!tmp)
		return NULL;

	memcpy(tmp, pxGpa->pucAddress, pxGpa->uxLength);

	p = tmp + pxGpa->uxLength;
	*(p) = '\0';

	return tmp;
}

gpa_t *pxNameToGPA(const name_t *pcName)
{
	gpa_t *pxGpa;
	string_t pcTmp;

	pcTmp = pcNameToString(pcName);

	if (!pcTmp) {
		LOGI(TAG_SHIM, "Failed to convert name to string");
		return NULL;
	}

	// Convert the IPCPAddress Concatenated to bits
	pxGpa = pxCreateGPA((buffer_t)pcTmp, strlen(pcTmp)); //considering the null terminated

	if (!pxGpa)	{
		LOGI(TAG_SHIM, "GPA was not created correct");
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

	if (!pucAddress || uxLength == 0) {
		LOGI(TAG_SHIM, "Bad input parameters, cannot create GPA");
		return NULL;
	}

	pxGPA = pvRsMemAlloc(sizeof(*pxGPA));

	if (!pxGPA)
		return NULL;

	pxGPA->uxLength = uxLength;
	pxGPA->pucAddress = pucCreateAddress(uxLength + 1);

	if (!pxGPA->pucAddress)	{
		vRsMemFree(pxGPA);
		return NULL;
	}

	memcpy(pxGPA->pucAddress, pucAddress, pxGPA->uxLength);

    if (xIsGPAOK(pxGPA))
        LOGD(TAG_SHIM, "CREATE GPA address: %s, size: %zu", pxGPA->pucAddress, pxGPA->uxLength);
    else {
        vRsMemFree(pxGPA);
        return NULL;
    }

	return pxGPA;
}

bool_t xGPAShrink(gpa_t *pxGpa, uint8_t ucFiller)
{
    buffer_t pucNewAddress;
    buffer_t pucPosition;
    size_t uxLength;

    if (!xIsGPAOK(pxGpa))
        return false;

    /* Look for the filler character in the address */
    pucPosition = memchr(pxGpa->pucAddress, ucFiller, pxGpa->uxLength);

    /* Check if there is any needs to shrink */
    if (pucPosition == NULL || pucPosition >= pxGpa->pucAddress + pxGpa->uxLength)
        return true;

    uxLength = pucPosition - pxGpa->pucAddress;

    pucNewAddress = pvRsMemAlloc(uxLength);
    if (!pucNewAddress)
        return false;

    memcpy(pucNewAddress, pxGpa->pucAddress, uxLength);

    vRsMemFree(pxGpa->pucAddress);
    pxGpa->pucAddress = pucNewAddress;
    pxGpa->uxLength = uxLength;

    return true;
}

bool_t xGPAGrow(gpa_t *pxGpa, size_t uxLength, uint8_t ucFiller)
{
    buffer_t new_address;

    if (!xIsGPAOK(pxGpa))
        return false;

    if (uxLength == 0 || uxLength < pxGpa->uxLength)
               return false;

    if (pxGpa->uxLength == uxLength)
        return true;

    RsAssert(uxLength > pxGpa->uxLength);

    new_address = pvRsMemAlloc(uxLength);
    if (!new_address)
        return false;

    memcpy(new_address, pxGpa->pucAddress, pxGpa->uxLength);
    memset(new_address + pxGpa->uxLength, ucFiller, uxLength - pxGpa->uxLength);
    vRsMemFree(pxGpa->pucAddress);
    pxGpa->pucAddress = new_address;
    pxGpa->uxLength = uxLength;

    return true;
}

gpa_t *pxDupGPA(const gpa_t *pxSourcePa, bool_t nDoShrink, uint8_t ucFiller)
{
    gpa_t *pxTargetPa = NULL;
    buffer_t c;

    RsAssert(pxSourcePa);

    if (!xIsGPAOK(pxSourcePa))
        return NULL;

    if (!(pxTargetPa = pvRsMemAlloc(sizeof(gpa_t))))
        return NULL;

    if (nDoShrink) {
        pxTargetPa->uxLength = pxSourcePa->uxLength;
        for (c = pxSourcePa->pucAddress + pxSourcePa->uxLength; *(--c) == ucFiller;)
            pxTargetPa->uxLength--;
    } else
        pxTargetPa->uxLength = pxSourcePa->uxLength;

    pxTargetPa->pucAddress = pvRsMemAlloc(pxSourcePa->uxLength);
    if (!pxTargetPa->pucAddress) {
        vRsMemFree(pxTargetPa);
        return NULL;
    }

    memcpy(pxTargetPa->pucAddress, pxSourcePa->pucAddress, pxTargetPa->uxLength);

    return pxTargetPa;
}

gha_t *pxCreateGHA(eGHAType_t xType, const MACAddress_t *pxAddress) // Changes to uint8_t
{
	gha_t *pxGha;

	if (xType != MAC_ADDR_802_3)
	{
		LOGE(TAG_SHIM, "Wrong input parameters, cannot create GHA");
		return NULL;
	}

    pxGha = pvRsMemAlloc(sizeof(*pxGha));
	if (!pxGha)
		return NULL;

	pxGha->xType = xType;
	memcpy(pxGha->xAddress.ucBytes, pxAddress->ucBytes, sizeof(pxGha->xAddress));

	return pxGha;
}

/* Return a duplicate of the source hardware address object */
gha_t *pxDupGHA(const gha_t *pxSourceHa)
{
    gha_t *pxTargetHa;

    RsAssert(pxSourceHa);

    if (!xIsGHAOK(pxSourceHa))
        return NULL;

    RsAssert(pxSourceHa->xType == MAC_ADDR_802_3);

    if ((pxTargetHa = pvRsMemAlloc(sizeof(gha_t))) == NULL)
        return NULL;

    memcpy(&pxTargetHa->xAddress, &pxSourceHa->xAddress, sizeof(MACAddress_t));
    pxTargetHa->xType = pxSourceHa->xType;

    return pxTargetHa;
}

bool_t xIsGPAOK(const gpa_t *pxGpa)
{
	if (!pxGpa)
	{
		LOGI(TAG_SHIM, " !Gpa");
		return false;
	}

	if (pxGpa->pucAddress == NULL)
	{
		LOGI(TAG_SHIM, "xIsGPAOK Address is NULL");
		return false;
	}

	if (pxGpa->uxLength == 0)
	{
		LOGI(TAG_SHIM, "Length = 0");
		return false;
	}
	return true;
}

bool_t xIsGHAOK(const gha_t *pxGha)
{
	if (!pxGha)
	{
		LOGI(TAG_SHIM, "No Valid GHA");
		return false;
	}

	if (pxGha->xType != MAC_ADDR_802_3)
	{
		return false;
	}

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
