#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "portability/port.h"
#include "common/rina_ids.h"

#include "IPCP_api.h"
#include "RINA_API.h"

#define TAG_APP "[Sensor-APP]"

int main(void)
{
	portId_t xAppPortId;
	struct rinaFlowSpec_t *xFlowSpec;
	uint8_t Flags = 1;
	int32_t xBytes;
	int i = 0;
	char json[200];
	void *buffer;
	size_t xLenBuffer = 1024;
	char *data;

	RINA_IPCPInit();

	assert((buffer = malloc(xLenBuffer)) != NULL);
    assert((xFlowSpec = malloc(sizeof(*xFlowSpec))) != NULL);

	memset(buffer, 0, xLenBuffer);

    sleep(1);

	LOGI(TAG_APP, "----------- Requesting a Flow ----- ");

	xAppPortId = RINA_flow_alloc("normal.DIF", "STH1", "sensor1", xFlowSpec, Flags);

	LOGI(TAG_APP, "Flow Port id: %d ", xAppPortId);

	if (xAppPortId != PORT_ID_WRONG)
	{
		while (i < 100)
		{
			sprintf(json, "Temperature: 30 C\n");

			LOGI(TAG_APP, "json:%s", json);
			if (RINA_flow_write(xAppPortId, (void *)json, strlen(json)))
				LOGI(TAG_APP, "Sent Data successfully");

			xBytes = RINA_flow_read(xAppPortId, (void *)buffer, xLenBuffer);

			if (xBytes > 0)
			{
				assert((data = strdup(buffer)) != NULL);
				LOGI(TAG_APP, "Receive data");
				LOGI(TAG_APP, "Buffer: %s", data);
				LOGI(TAG_APP, "Bytes received: %d", xBytes);
			}
			if (xBytes == 0)
				LOGI(TAG_APP, "It was an error receiving the buffer");

			sleep(2);

			i = i + 1;
		}
	}
}
