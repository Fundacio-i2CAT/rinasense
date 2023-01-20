#include <pthread.h>
#include <esp_pthread.h>

#include "portability/port.h"
#include "common/error.h"

bool_t xRsThreadCanSetName() { return false; }

rsErr_t xRsThreadPresetNameClear() {
    return xRsThreadPresetName("");
}

rsErr_t xRsThreadPresetName(const string_t pcThreadName)
{
    esp_pthread_cfg_t xThreadCfg;

    if (esp_pthread_get_cfg(&xThreadCfg) != ESP_OK)
        return ERR_SET(ERR_ERR);

    xThreadCfg.thread_name = pcThreadName;

    if (esp_pthread_set_cfg(&xThreadCfg) != ESP_OK)
        return ERR_SET(ERR_ERR);

    return SUCCESS;
}

rsErr_t xRsThreadSetName(pthread_t *pxThread, const string_t pcThreadName)
{
    LOGE("[critical]", "xRsThreadSetName is not supported on this platform");
    abort();

    return FAIL;
}
