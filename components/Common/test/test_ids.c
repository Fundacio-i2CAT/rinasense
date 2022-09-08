#include <limits.h>

#include "common/rina_ids.h"

#include "unity.h"
#include "common/unity_fixups.h"

RS_TEST_CASE_SETUP(test_ids) {}
RS_TEST_CASE_TEARDOWN(test_ids) {}

RS_TEST_CASE(ValidInvalid, "[ids]")
{
    /* Good port values. */
    TEST_ASSERT(is_port_id_ok(0));
    TEST_ASSERT(is_port_id_ok(((portId_t)-1) - 1));

    /* Bad port values */
    TEST_ASSERT(!is_port_id_ok((portId_t)-1));
    TEST_ASSERT(!is_port_id_ok(PORT_ID_WRONG));

    /* Good CEP ID values. */
    TEST_ASSERT(is_cep_id_ok(0));
    TEST_ASSERT(is_cep_id_ok((cepId_t)-1) - 1);

    /* Bad CEP ID values. */
    TEST_ASSERT(!is_cep_id_ok((cepId_t)-1));
    TEST_ASSERT(!is_cep_id_ok(CEP_ID_WRONG));

    /* Good IPCP ID values. */
    TEST_ASSERT(is_ipcp_id_ok(0));
    TEST_ASSERT(is_ipcp_id_ok(((ipcProcessId_t)-1) - 1));

    /* Bad IPCP ID Values. */
    TEST_ASSERT(!is_ipcp_id_ok((ipcProcessId_t)-1));
    TEST_ASSERT(!is_ipcp_id_ok(IPCP_ID_WRONG));

    /* Good address values */
    TEST_ASSERT(is_address_ok(0));
    TEST_ASSERT(is_address_ok(((address_t)-1) - 1));

    /* Bad address values */
    TEST_ASSERT(!is_address_ok((address_t)-1));
    TEST_ASSERT(!is_address_ok(ADDRESS_WRONG));

    /* Good QOS ID values. */
    TEST_ASSERT(is_qos_id_ok(0));
    TEST_ASSERT(is_qos_id_ok(((qosId_t)-1) - 1));

    /* Bad QOS ID values. */
    TEST_ASSERT(!is_qos_id_ok((qosId_t)-1));
    TEST_ASSERT(!is_qos_id_ok(QOS_ID_WRONG));
}

#ifndef TEST_CASE
int main()
{
    RS_SUITE_BEGIN();
    RS_RUN_TEST(ValidInvalid);
    RS_SUITE_END();
}
#endif
