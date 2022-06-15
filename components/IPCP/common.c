#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp_log.h"

#include "rina_common.h"

portId_t port_id_bad(void)
{
        return PORT_ID_WRONG;
}

BaseType_t is_port_id_ok(portId_t id)
{
        return id > 0 ? pdTRUE : pdFALSE;
}

BaseType_t is_cep_id_ok(cepId_t id)
{
        return id > 0 ? pdTRUE : pdFALSE;
}

BaseType_t is_ipcp_id_ok(ipcProcessId_t id)
{
        return id > 0 ? pdTRUE : pdFALSE;
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

/*name_t *xRinaNameCreate(void);

char *xRINAstrdup(const char *s);

name_t *xRinaNameCreate(void)
{
        name_t *result;

        result = pvPortMalloc(sizeof(*result));
        if (!result)
                return NULL;
        memset(result, 0, sizeof(*result));

        return result;
}


void vRINANameFini(name_t * n)
{
        configASSERT(n);

        if (n->pcProcessName) {
                vPortFree(n->pcProcessName);
                n->pcProcessName = NULL;
        }
        if (n->pcProcessInstance) {
                vPortFree(n->pcProcessInstance);
                n->pcProcessInstance = NULL;
        }
        if (n->pcEntityName) {
                vPortFree(n->pcEntityName);
                n->pcEntityName = NULL;
        }
        if (n->pcEntityInstance) {
                vPortFree(n->pcEntityInstance);
                n->pcEntityInstance = NULL;
        }

        ESP_LOGI(TAG_RINA,"Name at %pK finalized successfully", n);
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

BaseType_t xRinaNameFromString(const string_t pcString, name_t *xName);

BaseType_t xRinaNameFromString(const string_t pcString, name_t *xName)
{
        char *apn, *api, *aen, *aei;
        char *strc = xRINAstrdup(pcString);
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
}*/
#if 0
name_t *xRINANameInitFrom(name_t *dst,
					   const string_t *process_name,
					   const string_t *process_instance,
					   const string_t *entity_name,
					   const string_t *entity_instance)
{
	if (!dst)
		return NULL;

	/* Clean up the destination, leftovers might be there ... */
	xRinaNameFree(dst);
        //name_fini(dst);

	//ASSERT(name_is_initialized(dst));

	/* Boolean shortcuits ... */
	if (xRINAStringDup(process_name, &dst->pcProcessName) ||
		xRINAStringDup( process_instance, &dst->pcProcessInstance) ||
		xRINAStringDup( entity_name, &dst->pcEntityName) ||
		xRINAStringDup( entity_instance, &dst->pcEntityInstance))
	{
		vRINANameFini(dst);
		return NULL;
	}

	return dst;
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

		xRINAStringDup(pxInput, &tmp1);
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
	pxName = xRinaNameCreate( );
	if (!pxName)
	{
		if (tmp1)
			vPortFree(tmp1);
		return NULL;
	}

	if (!xRINANameInitFrom(pxName, tmp_pn, tmp_pi, tmp_en, tmp_ei))
	{
		xRinaNameFree(pxName);
		if (tmp1)
			vPortFree(tmp1);
		return NULL;
	}

	if (tmp1)
		vPortFree(tmp1);

	return pxName;
}
#endif
void memcheck(void)
{
        // perform free memory check
        int blockSize = 16;
        int i = 1;
        static int size = 0;
        printf("Checking memory with blocksize %d char ...\n", blockSize);
        while (true)
        {
                char *p = (char *)malloc(i * blockSize);
                if (p == NULL)
                {
                        break;
                }
                free(p);
                ++i;
        }
        printf("Ok for %d char\n", (i - 1) * blockSize);
        if (size != (i - 1) * blockSize)
                printf("There is a possible memory leak because the last memory size was %d and now is %d\n", size, (i - 1) * blockSize);
        size = (i - 1) * blockSize;
}

static int invoke_id = 1;
int get_next_invoke_id(void)
{
        return (invoke_id % INT_MAX == 0) ? (invoke_id = 1) : invoke_id++;
}
