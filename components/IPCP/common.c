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



void memcheck(void){
	// perform free memory check
	int blockSize = 16;
	int i = 1;
        static int size = 0;
	printf("Checking memory with blocksize %d char ...\n", blockSize);
	while (true) {
		char *p = (char *) malloc(i * blockSize);
		if (p == NULL){
			break;
		}
		free(p);
		++i;
	}
	printf("Ok for %d char\n", (i - 1) * blockSize);
    if (size != (i - 1) * blockSize) printf("There is a possible memory leak because the last memory size was %d and now is %d\n",size,(i - 1) * blockSize);
    size = (i - 1) * blockSize;
}

static int invoke_id = 1;
int get_next_invoke_id( void )
{
    return (invoke_id % INT_MAX == 0) ? (invoke_id = 1) : invoke_id++;
}