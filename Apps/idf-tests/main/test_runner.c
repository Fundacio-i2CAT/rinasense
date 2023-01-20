/* Example test application for testable component.
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>

#include "portability/port.h"

#include "unity.h"
#include "IPCP_api.h"
#include "RINA_API.h"
#include "soc/rtc_wdt.h"

void app_main(void)
{
    vRsLogInit();
    vRsLogSetLevel("*", LOG_VERBOSE);

    /* FIXME: I'm not sure this should be initialized here but I found
       out it's easier for the moment. */
    //RINA_Init();
    unity_run_menu();
}
