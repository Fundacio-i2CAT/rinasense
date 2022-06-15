#include <string.h>

#include "rina_gpha.h"
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

	LOGE(TAG_ARP, "GPA:%s", pxGpa->pucAddress);
	LOGE(TAG_ARP, "GPA:%s", tmp);

	return tmp;
}

gpa_t *pxNameToGPA(const name_t *pcName)
{
	gpa_t *pxGpa;
	char *pcTmp;

	pcTmp = pcNameToString(pcName);

	if (!pcTmp)
	{
		LOGI(TAG_SHIM, "Name to String not correct");
		return NULL;
	}

	// Convert the IPCPAddress Concatenated to bits
	pxGpa = pxCreateGPA(pcTmp, strlen(pcTmp)); //considering the null terminated

	if (!pxGpa)
	{
		LOGI(TAG_SHIM, "GPA was not created correct");
		vRsMemFree(pcTmp);
		return NULL;
	}

	vRsMemFree(pcTmp);

	return pxGpa;
}

char *pucCreateAddress(size_t uxLength)
{
	char *pucAddress;

	pucAddress = pvRsMemAlloc(uxLength);
    memset(pucAddress, 0, uxLength);

	return pucAddress;
}

gpa_t *pxCreateGPA(const char *pucAddress, size_t uxLength)
{
	gpa_t *pxGPA;

	if (!pucAddress || uxLength == 0)
	{
		LOGI(TAG_SHIM, "Bad input parameters, cannot create GPA");
		return NULL;
	}

	pxGPA = pvRsMemAlloc(sizeof(*pxGPA));

	if (!pxGPA)
		return NULL;

	pxGPA->uxLength = uxLength; //strlen of the address without '\0'
	pxGPA->pucAddress = pucCreateAddress(uxLength + 1); //Create an address an include the '\0'

	if (!pxGPA->pucAddress)
	{
		vRsMemFree(pxGPA);
		return NULL;
	}

	memcpy(pxGPA->pucAddress, pucAddress, pxGPA->uxLength);

	LOGI(TAG_SHIM, "CREATE GPA address: %s", pxGPA->pucAddress);
	LOGI(TAG_SHIM, "CREATE GPA size: %d", pxGPA->uxLength);

	return pxGPA;
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

bool_t xGPACmp(const gpa_t *gpa1, const gpa_t *gpa2) {
    return xIsGPAOK(gpa1)
        && xIsGPAOK(gpa2)
        && gpa1->uxLength == gpa2->uxLength
        && memcmp(gpa1->pucAddress, gpa2->pucAddress, gpa1->uxLength) == 0;
}

void vGPADestroy(gpa_t *pxGpa)
{
	if (!xIsGPAOK(pxGpa))
	{
		return;
	}

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
	{
		return;
	}

    /* Invalid the GHA so as to not accidentally reuse one that was
     * freed. */
    /* FIXME: Make part of debug builds only. */
    memset(pxGha, 0, sizeof(gha_t));

	vRsMemFree(pxGha);

	return;
}
