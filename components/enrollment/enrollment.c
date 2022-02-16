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

#include "esp_log.h"

BaseType_t xEnrollmentFlowAllocateRequest();

BaseType_t xEnrollmentFlowAllocateRequest()
{
    
}


BaseType_t xEnrollmentInitiate();
BaseType_t xEnrollmentInitiate()
{
    name_t *pxSource;
    name_t *pxDestInfo;
    porId_t xN1FlowPortId;
    authPolicy_t xAuth;

    pxSource = pvPortMalloc(sizeof(*pxSource));
    pxDestInfo = pvPortMalloc(sizeof(*pxDestInfo));

    pxSource->pcEntityInstance = NORMAL_ENTITY_INSTANCE;
    pxSource->pcEntityName = NORMAL_ENTITY_NAME;
    pxSource->pcProcessInstance = NORMAL_PROCESS_INSTANCE;
    pxSource->pcProcessName = NORMAL_PROCESS_NAME;

    pxDestInfo->pcProcessName = NORMAL_DIF_NAME;
    pxDestInfo->pcProcessInstance = "";
    pxDestInfo->pcEntityInstance = "";
    pxDestInfo->pcEntityName = "";

    xAuth.ucAbsSyntax = 1;
    xAuth.ucVersion = 1;
    xAuth.xName = "PSOC_authentication-none";

    //Find PortId

    xRIBDaemonConnectToIpcp(pxSource, pxDestInfo, xN1FlowPortId, xAuth)
}
