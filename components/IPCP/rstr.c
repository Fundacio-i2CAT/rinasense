#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "esp_heap_caps.h"

#include "common.h"

#include "esp_log.h"

name_t *pxRStrNameCreate(void)
{
        name_t *pxTmp;

        pxTmp = pvPortMalloc(sizeof(*pxTmp));
        if (!pxTmp)
                return NULL;
        memset(pxTmp, 0, sizeof(*pxTmp));

        return pxTmp;
}

void vRstrNameFini(name_t *n)
{
        // configASSERT(n);

        if (n->pcProcessName)
        {
                vPortFree(n->pcProcessName);
                n->pcProcessName = NULL;
        }
        if (n->pcProcessInstance)
        {
                vPortFree(n->pcProcessInstance);
                n->pcProcessInstance = NULL;
        }
        if (n->pcEntityName)
        {
                vPortFree(n->pcEntityName);
                n->pcEntityName = NULL;
        }
        if (n->pcEntityInstance)
        {
                vPortFree(n->pcEntityInstance);
                n->pcEntityInstance = NULL;
        }

        ESP_LOGI(TAG_IPCPMANAGER, "Name at %pK finalized successfully", n);
}

void vRstrNameDestroy(name_t *pxName)
{
        // configASSERT(pxName);

        vRstrNameFini(pxName);

        vPortFree(pxName);

        ESP_LOGI(TAG_IPCPMANAGER, "Name at %pK destroyed successfully", pxName);
}

void vRstrNameFree(name_t *xName)
{
        if (!xName)
        {
                return;
        }

        if (xName->pcProcessName)
        {
                vPortFree(xName->pcProcessName);
                xName->pcProcessName = NULL;
        }

        if (xName->pcProcessInstance)
        {
                vPortFree(xName->pcProcessInstance);
                xName->pcProcessInstance = NULL;
        }

        if (xName->pcEntityName)
        {
                vPortFree(xName->pcEntityName);
                xName->pcEntityName = NULL;
        }

        if (xName->pcEntityInstance)
        {
                vPortFree(xName->pcEntityInstance);
                xName->pcEntityInstance = NULL;
        }

        vPortFree(xName);
}

BaseType_t xRinaNameFromString(const string_t pcString, name_t *xName);

char *pcRstrDup(const char *s)
{
        size_t len;
        char *buf;

        if (!s)
                return NULL;

        len = strlen(s) + 1;
        // ESP_LOGE(TAG_RIB, "Len RstrDup:%d", len);
        buf = pvPortMalloc(len);
        memset(buf, 0, len);
        if (buf)
                memcpy(buf, s, len);

        return buf;
}

BaseType_t xRstringDup(const string_t *pxSrc, string_t **pxDst)
{
        // ESP_LOGE(TAG_RINA, "Free Heap Size: %d", xPortGetFreeHeapSize());
        // ESP_LOGE(TAG_RINA, "Minimum Ever Heap Size: %d", xPortGetMinimumEverFreeHeapSize());
        heap_caps_check_integrity_all(pdTRUE);
        if (!pxDst)
        {
                ESP_LOGE(TAG_RINA, "Destination string is NULL, cannot copy");
                return pdFALSE;
        }

        if (pxSrc)
        {
                *pxDst = pcRstrDup(pxSrc);
                if (!*pxDst)
                {
                        ESP_LOGE(TAG_RINA, "Cannot duplicate source string "
                                           "in kernel-space");
                        return pdFALSE;
                }
        }
        else
        {
                ESP_LOGE(TAG_RINA, "Duplicating a NULL source string ...");
                *pxDst = NULL;
        }

        return pdTRUE;
}

static BaseType_t xRstrNameCpy(const name_t *pxSrc, name_t *pxDst)
{
        if (!pxSrc || !pxDst)
                return pdFALSE;

        ESP_LOGI(TAG_IPCPMANAGER, "Copying name %pK into %pK", pxSrc, pxDst);

        vRstrNameFini(pxDst);

        // ASSERT(name_is_initialized(pxDst));

        /* We rely on short-boolean evaluation ... :-) */
        if (xRstringDup(pxSrc->pcProcessName, &pxDst->pcProcessName) ||
            xRstringDup(pxSrc->pcProcessInstance, &pxDst->pcProcessInstance) ||
            xRstringDup(pxSrc->pcEntityName, &pxDst->pcEntityName) ||
            xRstringDup(pxSrc->pcEntityInstance, &pxDst->pcEntityInstance))
        {
                vRstrNameFini(pxDst);
                return pdFALSE;
        }

        ESP_LOGI(TAG_IPCPMANAGER, "Name %pK copied successfully into %pK", pxSrc, pxDst);

        return pdTRUE;
}

name_t *xRINANameInitFrom(name_t *pxDst,
                          const string_t *process_name,
                          const string_t *process_instance,
                          const string_t *entity_name,
                          const string_t *entity_instance)
{
        if (!pxDst)
                return NULL;

        /* Clean up the destination, leftovers might be there ... */
        vRstrNameFree(pxDst);
        // name_fini(pxDst);

        // ASSERT(name_is_initialized(pxDst));

        /* Boolean shortcuits ... */
        if (xRstringDup(process_name, &pxDst->pcProcessName) ||
            xRstringDup(process_instance, &pxDst->pcProcessInstance) ||
            xRstringDup(entity_name, &pxDst->pcEntityName) ||
            xRstringDup(entity_instance, &pxDst->pcEntityInstance))
        {
                vRstrNameFini(pxDst);
                return NULL;
        }

        return pxDst;
}

name_t *xRINAstringToName(const string_t *pxInput)
{
        name_t *pxName;

        string_t *tmp1 = NULL;
        string_t *tmp_pn = NULL;
        string_t *tmp_pi = NULL;
        string_t *tmp_en = NULL;
        string_t *tmp_ei = NULL;

        ESP_LOGE(TAG_RINA, "pxInput: %s", *pxInput);

        if (pxInput)
        {
                string_t *tmp2;

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

        ESP_LOGE(TAG_RINA, "tmp_pn: %s", *tmp_pn);
        pxName = pxRStrNameCreate();
        if (!pxName)
        {
                if (tmp1)
                        vPortFree(tmp1);
                return NULL;
        }

        if (!xRINANameInitFrom(pxName, tmp_pn, tmp_pi, tmp_en, tmp_ei))
        {
                vRstrNameFree(pxName);
                if (tmp1)
                        vPortFree(tmp1);
                return NULL;
        }

        if (tmp1)
                vPortFree(tmp1);

        return pxName;
}

/**
 * @brief Create a Rina Name Structure using a string separated
 * by the character "|".
 *
 * @param pcString String the type "name|instance|name|instance"
 * @param xName pointer name struct to be filled with the string.
 * @return BaseType_t
 */
BaseType_t xRinaNameFromString(const string_t pcString, name_t *pxName)
{
        // ESP_LOGE(TAG_RINA, "Calling: %s", __func__);
        char *apn, *api, *aen, *aei;
        char *strc = pcRstrDup(pcString);
        char *strc_orig = strc;
        char **strp = &strc;

        memset(pxName, 0, sizeof(*pxName));

        if (!strc)
                return pdFALSE;

        apn = strsep(strp, "|");
        api = strsep(strp, "|");
        aen = strsep(strp, "|");
        aei = strsep(strp, "|");

        if (!apn)
        {
                vPortFree(strc_orig);
                return pdFALSE;
        }

        pxName->pcProcessName = (apn && strlen(apn)) ? pcRstrDup(apn) : NULL;
        pxName->pcProcessInstance = (api && strlen(api)) ? pcRstrDup(api) : NULL;
        pxName->pcEntityName = (aen && strlen(aen)) ? pcRstrDup(aen) : NULL;
        pxName->pcEntityInstance = (aei && strlen(aei)) ? pcRstrDup(aei) : NULL;

        if ((apn && strlen(apn) && !pxName->pcProcessName) ||
            (api && strlen(api) && !pxName->pcProcessInstance) ||
            (aen && strlen(aen) && !pxName->pcEntityName) ||
            (aei && strlen(aei) && !pxName->pcEntityInstance))
        {
                vRstrNameFree(pxName);
                return pdFALSE;
        }

        vPortFree(strc_orig);

        return pdTRUE;
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
