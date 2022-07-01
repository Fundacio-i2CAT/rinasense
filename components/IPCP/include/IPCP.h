#ifndef IPCP_H_
#define IPCP_H_

#include "configSensor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "ARP826.h"
#include "pci.h"
#include "rina_common.h"
#include "rina_name.h"
//#include "Rmt.h"

/*-----------------------------------------------------------*/
/* Miscellaneous structure and definitions. */
/*-----------------------------------------------------------*/

#define IS_PORT_ID_OK(id) (id >= 0 ? pdTRUE : pdFALSE)

typedef struct xQUEUE_FIFO
{
    QueueHandle_t xQueue;

} rfifo_t;


// typedef uint16_t ipcProcessId_t;

typedef uint16_t ipcpInstanceId_t;

typedef int32_t portId_t;

/**
 * The software timer struct for various IPCP functions
 */
typedef struct xIPCP_TIMER
{
    uint32_t
        bActive : 1,            /**< This timer is running and must be processed. */
        bExpired : 1;           /**< Timer has expired and a task must be processed. */
    TimeOut_t xTimeOut;         /**< The timeout value. */
    TickType_t ulRemainingTime; /**< The amount of time remaining. */
    TickType_t ulReloadTime;    /**< The value of reload time. */
} IPCPTimer_t;

#endif
