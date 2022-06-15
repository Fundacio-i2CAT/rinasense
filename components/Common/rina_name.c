#include <stdio.h>
#include <string.h>

#include "rina_name.h"
#include "configSensor.h"

#include "portability/rslist.h"
#include "portability/rsmem.h"
#include "portability/rslog.h"

#define TAG_RINA_NAME "rina_name"

name_t *pxRStrNameCreate(void)
{
        name_t *pxTmp;

        pxTmp = pvRsMemAlloc(sizeof(name_t));
        if (!pxTmp)
                return NULL;
        memset(pxTmp, 0, sizeof(name_t));

        return pxTmp;
}

void vRstrNameFini(name_t *n)
{
        RsAssert(n);

        if (n->pcProcessName)
        {
                vRsMemFree(n->pcProcessName);
                n->pcProcessName = NULL;
        }
        if (n->pcProcessInstance)
        {
                vRsMemFree(n->pcProcessInstance);
                n->pcProcessInstance = NULL;
        }
        if (n->pcEntityName)
        {
                vRsMemFree(n->pcEntityName);
                n->pcEntityName = NULL;
        }
        if (n->pcEntityInstance)
        {
                vRsMemFree(n->pcEntityInstance);
                n->pcEntityInstance = NULL;
        }

        LOGI(TAG_RINA_NAME, "Name at %pK finalized successfully", n);
}

void vRstrNameDestroy(name_t *pxName)
{
        RsAssert(pxName);

        vRstrNameFini(pxName);

        vRsMemFree(pxName);

        LOGI(TAG_RINA_NAME, "Name at %pK destroyed successfully", pxName);
}

void vRstrNameFree(name_t *xName)
{
        if (!xName)
        {
                return;
        }

        if (xName->pcProcessName)
        {
                vRsMemFree(xName->pcProcessName);
                xName->pcProcessName = NULL;
        }

        if (xName->pcProcessInstance)
        {
                vRsMemFree(xName->pcProcessInstance);
                xName->pcProcessInstance = NULL;
        }

        if (xName->pcEntityName)
        {
                vRsMemFree(xName->pcEntityName);
                xName->pcEntityName = NULL;
        }

        if (xName->pcEntityInstance)
        {
                vRsMemFree(xName->pcEntityInstance);
                xName->pcEntityInstance = NULL;
        }

        vRsMemFree(xName);
}

//BaseType_t xRinaNameFromString(const char * pcString, name_t *xName);

char *pcRstrDup(const char *s)
{
        size_t len;
        char *buf;

        if (!s)
                return NULL;

        len = strlen(s) + 1;
        LOGE(TAG_RINA_NAME, "Len RstrDup:%d", len);
        buf = pvRsMemAlloc(len);
        memset(buf,0,len);
        if (buf)
                memcpy(buf, s, len);

        return buf;
}

bool_t xRstringDup(const char * pxSrc, char * *pxDst)
{
#ifdef __FREERTOS__
        LOGE(TAG_RINA, "Free Heap Size: %d", xPortGetFreeHeapSize());
        LOGE(TAG_RINA, "Minimum Ever Heap Size: %d", xPortGetMinimumEverFreeHeapSize());
        heap_caps_check_integrity_all(true);
#endif
        if (!pxDst)
        {
                LOGE(TAG_RINA_NAME, "Destination string is NULL, cannot copy");
                return false;
        }

        if (pxSrc)
        {
                *pxDst = pcRstrDup(pxSrc);
                if (!*pxDst)
                {
                        LOGE(TAG_RINA_NAME, "Cannot duplicate source string "
                                           "in kernel-space");
                        return false;
                }
        }
        else
        {
                LOGE(TAG_RINA_NAME, "Duplicating a NULL source string ...");
                *pxDst = NULL;
        }

        return true;
}


bool_t xRstrNameCpy(const name_t *pxSrc, name_t *pxDst)
{
        if (!pxSrc || !pxDst)
                return false;

        LOGI(TAG_RINA_NAME, "Copying name %pK into %pK", pxSrc, pxDst);

        vRstrNameFini(pxDst);

        // ASSERT(name_is_initialized(pxDst));

        /* We rely on short-boolean evaluation ... :-) */
        if (xRstringDup(pxSrc->pcProcessName, &pxDst->pcProcessName)
            || xRstringDup(pxSrc->pcProcessInstance, &pxDst->pcProcessInstance)
            || xRstringDup(pxSrc->pcEntityName, &pxDst->pcEntityName)
            || xRstringDup(pxSrc->pcEntityInstance, &pxDst->pcEntityInstance))
        {
                vRstrNameFini(pxDst);
                return false;
        }

        LOGI(TAG_RINA_NAME, "Name %pK copied successfully into %pK", pxSrc, pxDst);

        return true;
}

name_t *xRINANameInitFrom(name_t *pxDst,
                          const char * process_name,
                          const char * process_instance,
                          const char * entity_name,
                          const char * entity_instance)
{
        if (!pxDst)
                return NULL;

        /* Clean up the destination, leftovers might be there ... */
        vRstrNameFini(pxDst);
        // name_fini(pxDst);

        // ASSERT(name_is_initialized(pxDst));

        /* Boolean shortcuits ... */
        if (!xRstringDup(process_name, &pxDst->pcProcessName) ||
            !xRstringDup(process_instance, &pxDst->pcProcessInstance) ||
            !xRstringDup(entity_name, &pxDst->pcEntityName) ||
            !xRstringDup(entity_instance, &pxDst->pcEntityInstance))
        {
                vRstrNameFini(pxDst);
                return NULL;
        }

        return pxDst;
}

name_t *xRINAstringToName(const char * pxInput)
{
        name_t *pxName;

        char * tmp1 = NULL;
        char * tmp_pn = NULL;
        char * tmp_pi = NULL;
        char * tmp_en = NULL;
        char * tmp_ei = NULL;

        LOGE(TAG_RINA_NAME, "pxInput: %s", pxInput);

        if (pxInput)
        {
                char * tmp2;

                xRstringDup(pxInput, &tmp1);
                if (!tmp1)
                {
                        return NULL;
                }
                tmp2 = tmp1;

                tmp_pn = strsep(&tmp2, DELIMITER);
                tmp_pi = strsep(&tmp2, DELIMITER);
                tmp_en = strsep(&tmp2, DELIMITER);
                tmp_ei = strsep(&tmp2, DELIMITER);
        }

        LOGE(TAG_RINA_NAME, "tmp_pn: %s", tmp_pn);
        pxName = pxRStrNameCreate();
        if (!pxName)
        {
                if (tmp1)
                        vRsMemFree(tmp1);
                return NULL;
        }

        if (!xRINANameInitFrom(pxName, tmp_pn, tmp_pi, tmp_en, tmp_ei))
        {
                vRstrNameFini(pxName);
                if (tmp1)
                        vRsMemFree(tmp1);
                return NULL;
        }

        if (tmp1)
                vRsMemFree(tmp1);

        return pxName;
}

bool_t xRinaNameFromString(const char * pcString, name_t *pxName)
{
        LOGE(TAG_RINA, "Calling: %s", __func__);

        char *apn, *api, *aen, *aei;
        char *strc = pcRstrDup(pcString);
        char *strc_orig = strc;
        char **strp = &strc;

        memset(pxName, 0, sizeof(*pxName));

        if (!strc)
                return false;

        apn = strsep(strp, "|");
        api = strsep(strp, "|");
        aen = strsep(strp, "|");
        aei = strsep(strp, "|");

        if (!apn)
        {
                vRsMemFree(strc_orig);
                return false;
        }

        pxName->pcProcessName = (apn && strlen(apn)) ? pcRstrDup(apn) : NULL;
        pxName->pcProcessInstance = (api && strlen(api)) ? pcRstrDup(api) : NULL;
        pxName->pcEntityName = (aen && strlen(aen)) ? pcRstrDup(aen) : NULL;
        pxName->pcEntityInstance = (aei && strlen(aei)) ? pcRstrDup(aei) : NULL;

        LOGE(TAG_FA, "RinaNameFromString - pcProcessName:%s", pxName->pcProcessName);
        LOGE(TAG_FA, "RinaNameFromString - pcProcessInstance:%s", pxName->pcProcessInstance);
        LOGE(TAG_FA, "RinaNameFromString - pcEntityName:%s", pxName->pcEntityName);
        LOGE(TAG_FA, "RinaNameFromString - pcEntityInstance:%s", pxName->pcEntityInstance);

        if ((apn && strlen(apn) && !pxName->pcProcessName) ||
            (api && strlen(api) && !pxName->pcProcessInstance) ||
            (aen && strlen(aen) && !pxName->pcEntityName) ||
            (aei && strlen(aei) && !pxName->pcEntityInstance))
        {
                vRstrNameFini(pxName);
                return false;
        }

        vRsMemFree(strc_orig);

        return true;
}

/**
 * @brief Duplicate a Rina Name structure
 *
 * @param pxSrc Pointer to the Rina Name structure
 * @return Pointer to the Rina Name duplicated.
 */
name_t *pxRstrNameDup(const name_t *pxSrc)
{
        name_t *pxTmp;

        if (!pxSrc)
                return NULL;

        pxTmp = pxRStrNameCreate();
        if (!pxTmp)
                return NULL;
        if (xRstrNameCpy(pxSrc, pxTmp))
        {
                vRstrNameDestroy(pxTmp);
                return NULL;
        }

        return pxTmp;
}

char * pcNameToString(const name_t *n)
{
    char *       tmp;
    size_t       size;
    const char * none     = "";
    size_t       none_len = strlen(none);

    if (!n)
        return NULL;

    size  = 0;

    size += (n->pcProcessName                 ?
             strlen(n->pcProcessName)     : none_len);
    size += strlen(DELIMITER);

    size += (n->pcProcessInstance             ?
             strlen(n->pcProcessInstance) : none_len);
    size += strlen(DELIMITER);

    size += (n->pcEntityName                  ?
             strlen(n->pcEntityName)      : none_len);
    size += strlen(DELIMITER);

    size += (n->pcEntityInstance              ?
             strlen(n->pcEntityInstance)  : none_len);
    size += strlen(DELIMITER);

    tmp = pvRsMemAlloc(size);
    memset(tmp, 0, sizeof(*tmp));

    if (!tmp)
        return NULL;

    if (snprintf(tmp, size,
                 "%s%s%s%s%s%s%s",
                 (n->pcProcessName     ? n->pcProcessName     : none),
                 DELIMITER,
                 (n->pcProcessInstance ? n->pcProcessInstance : none),
                 DELIMITER,
                 (n->pcEntityName      ? n->pcEntityName      : none),
                 DELIMITER,
                 (n->pcEntityInstance  ? n->pcEntityInstance  : none)) !=
        size - 1) {
        vRsMemFree(tmp);
        return NULL;
    }

    return tmp;
}
