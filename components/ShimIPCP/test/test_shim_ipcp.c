#include "common/num_mgr.h"
#include "portability/port.h"

#include "IpcManager.h"
#include "ShimIPCP.h"

#include "unity.h"
#include "common/unity_fixups.h"

ipcManager_t ipcMgr;
NumMgr_t pidm;

RS_TEST_CASE_SETUP(test_shim_ipcp) {
    xIpcManagerInit(&ipcMgr);
    TEST_ASSERT(xNumMgrInit(&pidm, 10));
}

RS_TEST_CASE_TEARDOWN(test_shim_ipcp) {}

RS_TEST_CASE(ShimIpcpBasic, "[shimipcp][ipcp]")
{
    name_t n1;
    struct ipcpInstance_t *ipcp;

    RS_TEST_CASE_BEGIN(test_shim_ipcp);

    TEST_ASSERT(xRinaNameFromString("e1|e2|e3", &n1));
    TEST_ASSERT((ipcp = pxIpcManagerCreateShim(&ipcMgr)) != NULL);

    /* We're cheating as we know the first IPCP ID is 1 here. */
    //TEST_ASSERT((ipcp = pxIpcManagerFindInstanceById(1)) != NULL);
    TEST_ASSERT(ipcp->xId == 1);
    TEST_ASSERT(ipcp->xType == eShimWiFi);

    RS_TEST_CASE_END(test_shim_ipcp);
}

#ifndef TEST_CASE
int main()
{
    UNITY_BEGIN();
    RS_RUN_TEST(ShimIpcpBasic);
    return UNITY_END();
}
#endif
