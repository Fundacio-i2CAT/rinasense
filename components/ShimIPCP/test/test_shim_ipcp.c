#include "common/num_mgr.h"
#include "common/rina_name.h"
#include "portability/port.h"

#include "IpcManager.h"
#include "ShimIPCP.h"

#include "unity.h"
#include "common/unity_fixups.h"

/* FIXME: Shim testing needs to be rewriten. */

#ifndef TEST_CASE
int main()
{
    UNITY_BEGIN();
    return UNITY_END();
}
#endif
