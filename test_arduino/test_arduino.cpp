#include <Arduino.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "portability/port.h"
#include "common/rina_ids.h"

#include "RINA_API.h"
#include "IPCP_api.h"

void setup()
{
	RINA_IPCPInit();
}

void loop() {}
