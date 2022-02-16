#include <string.h>

#include "Freertos/FreeRTOS.h"
#include "Freertos/task.h"
#include "Freertos/queue.h"
#include "Freertos/semphr.h"

#include "esp_log.h"

#include "common.h"

portId_t port_id_bad(void)
{
        return PORT_ID_WRONG;
}

BaseType_t is_port_id_ok(portId_t id)
{
        return id >= 0 ? pdTRUE : pdFALSE;
}

BaseType_t is_cep_id_ok(cepId_t id)
{
        return id >= 0 ? pdTRUE : pdFALSE;
}

BaseType_t is_ipcp_id_ok(ipcProcessId_t id)
{
        return id >= 0 ? pdTRUE : pdFALSE;
}

cepId_t cep_id_bad(void)
{
        return CEP_ID_WRONG;
}

BaseType_t is_address_ok(address_t address)
{
        return address != ADDRESS_WRONG ? pdTRUE : pdFALSE;
}

address_t address_bad(void)
{
        return ADDRESS_WRONG;
}

qosId_t qos_id_bad(void)
{
        return QOS_ID_WRONG;
}

BaseType_t is_qos_id_ok(qosId_t id)
{
        return id != QOS_ID_WRONG ? pdTRUE : pdFALSE;
}

name_t *xRinaNameCreate(void);

char *xRINAstrdup(const char *s);

name_t *xRinaNameCreate(void)
{
        name_t *result;

        result = pvPortMalloc(sizeof(name_t));
        if (!result)
                return NULL;
        memset(result, 0, sizeof(name_t));

        return result;
}

void xRinaNameFree(name_t *xName);

void xRinaNameFree(name_t *xName)
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

BaseType_t xRinaNameFromString(const string_t xString, name_t *xName);

BaseType_t xRinaNameFromString(const string_t xString, name_t *xName)
{
        char *apn, *api, *aen, *aei;
        char *strc = xRINAstrdup(xString);
        char *strc_orig = strc;
        char **strp = &strc;

        memset(xName, 0, sizeof(*xName));

        if (!strc)
                return pdFALSE;

        apn = strsep(strp, "|");
        api = strsep(strp, "|");
        aen = strsep(strp, "|");
        aei = strsep(strp, "|");

        
	if (!apn) {
		vPortFree(strc_orig);
		return pdFALSE;
	}



        xName->pcProcessName = (apn && strlen(apn)) ? xRINAstrdup(apn) : NULL;
        xName->pcProcessInstance = (api && strlen(api)) ? xRINAstrdup(api) : NULL;
        xName->pcEntityName = (aen && strlen(aen)) ? xRINAstrdup(aen) : NULL;
        xName->pcEntityInstance = (aei && strlen(aei)) ? xRINAstrdup(aei) : NULL;



        if ((apn && strlen(apn) && !xName->pcProcessName) ||
            (api && strlen(api) && !xName->pcProcessInstance) ||
            (aen && strlen(aen) && !xName->pcEntityName) ||
            (aei && strlen(aei) && !xName->pcEntityInstance))
        {
                xRinaNameFree(xName);
                return pdFALSE;
        }

        vPortFree(strc_orig);

        return pdTRUE;
}

char *xRINAstrdup(const char *s)
{
        size_t len;
        char *buf;

        if (!s)
                return NULL;

        len = strlen(s) + 1;
        buf = pvPortMalloc(len);
        if (buf)
                memcpy(buf, s, len);

        return buf;
}

BaseType_t xRINAStringDup(const string_t *src, string_t **dst)
{
        if (!dst)
        {
                ESP_LOGE(TAG_RINA, "Destination string is NULL, cannot copy");
                return pdFALSE;
        }

        if (src)
        {
                *dst = xRINAstrdup(src);
                if (!*dst)
                {
                        ESP_LOGE(TAG_RINA, "Cannot duplicate source string "
                                           "in kernel-space");
                        return pdFALSE;
                }
        }
        else
        {
                ESP_LOGE(TAG_RINA, "Duplicating a NULL source string ...");
                *dst = NULL;
        }

        return pdTRUE;
}
