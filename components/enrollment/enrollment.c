/*Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

/* RINA includes. */
#include "common.h"
#include "configSensor.h"
#include "configRINA.h"
#include "Ribd.h"

#include "esp_log.h"

BaseType_t xEnrollmentFlowAllocateRequest();



void xEnrollmentInit(portId_t xPortId );
void xEnrollmentInit(portId_t xPortId  )
{
   
    name_t *pxSource;
    name_t *pxDestInfo;
    //porId_t xN1FlowPortId;
    authPolicy_t * pxAuth;
    appConnection_t * test;
    test = pvPortMalloc(sizeof(*test));

    pxSource = pvPortMalloc(sizeof(*pxSource));
    pxDestInfo = pvPortMalloc(sizeof(*pxDestInfo));
    pxAuth = pvPortMalloc(sizeof(*pxAuth));

    pxSource->pcEntityInstance = MANAGEMENT_AE;
    pxSource->pcEntityName = NORMAL_ENTITY_NAME;
    pxSource->pcProcessInstance = NORMAL_PROCESS_INSTANCE;
    pxSource->pcProcessName = NORMAL_PROCESS_NAME;

    pxDestInfo->pcProcessName = NORMAL_DIF_NAME;
    pxDestInfo->pcProcessInstance = "";
    pxDestInfo->pcEntityInstance = MANAGEMENT_AE;
    pxDestInfo->pcEntityName ="";

    pxAuth->ucAbsSyntax = 1;
    pxAuth->pcVersion = "1";
    pxAuth->pcName = "PSOC_authentication-none";

    //Find PortId


   xRibdConnectToIpcp(pxSource, pxDestInfo, xPortId, pxAuth);
}
