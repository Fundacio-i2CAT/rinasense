#include <stdio.h>
#include <string.h>
#include <limits.h> // For INT_MAX

#include "portability/port.h"
#include "configSensor.h"
#include "rina_common_port.h"

void memcheck(void)
{
        // perform free memory check
        int blockSize = 16;
        int i = 1;
        static int size = 0;
        LOGD(TAG_IPCPMANAGER, "Checking memory with blocksize %d char ...\n", blockSize);
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
        LOGD(TAG_IPCPMANAGER, "Ok for %d char\n", (i - 1) * blockSize);
        if (size != (i - 1) * blockSize)
            LOGE(TAG_IPCPMANAGER, "There is a possible memory leak because the last memory size was %d and now is %d\n", size, (i - 1) * blockSize);
        size = (i - 1) * blockSize;
}

void vPrintBytes(void *ptr, int size)
{
        unsigned char *p = ptr;
        int i;
        for (i = 0; i < size; i++)
        {
                printf("%02hhX ", p[i]);
        }
        printf("\n");
}

static int invoke_id = 1;

int get_next_invoke_id(void)
{
        return (invoke_id % INT_MAX == 0) ? (invoke_id = 1) : invoke_id++;
}
