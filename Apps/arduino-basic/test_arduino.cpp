#include <Arduino.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "portability/port.h"
#include "common/rina_ids.h"

#include "RINA_API.h"
#include "IPCP_api.h"
#include "nvs_flash.h"

void setup()
{
    nvs_flash_init();
    vRsLogInit();
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    RINA_Init();
    sleep(10);
}

void loop() {
}
