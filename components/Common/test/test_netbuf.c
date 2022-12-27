#include <string.h>

#include "common/rsrc.h"
#include "linux_rsmem.h"
#include "portability/port.h"
#include "common/netbuf.h"

#include "unity.h"
#include "common/unity_fixups.h"

bool_t xWasFreeCalled = false;

void vTestFreeNormal(netbuf_t *pxNb){
    xWasFreeCalled = true;
    vNetBufFreeNormal(pxNb);
}

RS_TEST_CASE_SETUP(test_netbuf) {
    xWasFreeCalled = false;
}

RS_TEST_CASE_TEARDOWN(test_netbuf) {}

RS_TEST_CASE(NetBufSimple, "[netbuf]")
{
    rsrcPoolP_t xPool;
    uint8_t buf[10];
    netbuf_t *nb;

    RS_TEST_CASE_BEGIN(test_netbuf);

    TEST_ASSERT((xPool = xNetBufNewPool("NetBufSimple")));
    TEST_ASSERT((nb = pxNetBufNew(xPool, NB_UNKNOWN, buf, sizeof(buf), NETBUF_FREE_DONT)));
    TEST_ASSERT(nb->pxBuf == buf);
    TEST_ASSERT(nb->pxBufStart == buf);
    TEST_ASSERT(nb->pxFirst == nb);
    TEST_ASSERT(nb->pxNext == NULL);
    TEST_ASSERT(nb->unSz == sizeof(buf));
    TEST_ASSERT(!nb->xFreed);
    vNetBufFree(nb);

    RS_TEST_CASE_END(test_netbuf);
}

RS_TEST_CASE(NetBufSimpleFree, "[netbuf]")
{
    rsrcPoolP_t xPool;
    uint8_t *buf;
    netbuf_t *nb;
    size_t sz = 10;

    RS_TEST_CASE_BEGIN(test_netbuf);

    TEST_ASSERT((xPool = xNetBufNewPool("NetBufSimpleFree")));
    TEST_ASSERT((buf = pvRsMemAlloc(sz)));
    TEST_ASSERT((nb = pxNetBufNew(xPool, NB_UNKNOWN, buf, sz, &vTestFreeNormal)));
    vNetBufFree(nb);

    RS_TEST_CASE_END(test_netbuf);
}

RS_TEST_CASE(NetBufSplit, "[netbuf]")
{
    rsrcPoolP_t xPool;
    uint8_t *buf;
    netbuf_t *nb1, *nb2;
    size_t sz1 = 20, sz2 = 10;

    RS_TEST_CASE_BEGIN(test_netbuf);

    TEST_ASSERT((xPool = xNetBufNewPool("NetBufSplit")));
    TEST_ASSERT((buf = pvRsMemAlloc(sz1)));

    TEST_ASSERT((nb1 = pxNetBufNew(xPool, NB_UNKNOWN, buf, sz1, &vTestFreeNormal)));
    TEST_ASSERT(nb1->unSz == 20);
    TEST_ASSERT(!ERR_CHK((xNetBufSplit(nb1, NB_UNKNOWN, sz2))));
    nb2 = pxNetBufNext(nb1);

    TEST_ASSERT(nb1->pxBufStart == nb2->pxBufStart);
    TEST_ASSERT(nb1->unSz == 10);
    TEST_ASSERT(nb2->unSz == 10);
    TEST_ASSERT(nb1->pxFirst == nb1);
    TEST_ASSERT(nb2->pxFirst == nb1);
    TEST_ASSERT(nb2->pxNext == NULL);

    vNetBufFree(nb1);

    TEST_ASSERT(nb1->xFreed);
    TEST_ASSERT(nb1->pxBuf == NULL);
    TEST_ASSERT(nb1->pxBufStart == NULL);
    TEST_ASSERT(!nb2->xFreed);
    TEST_ASSERT(nb2->unSz == sz2);
    TEST_ASSERT(unNetBufCount(nb2) == 1);
    TEST_ASSERT(unNetBufTotalSize(nb2) == sz2);

    vNetBufFree(nb2);

    TEST_ASSERT(xWasFreeCalled);

    RS_TEST_CASE_END(test_netbuf);
}

RS_TEST_CASE(NetBufSplit2, "[netbuf]")
{
    rsrcPoolP_t xPool;
    uint8_t *buf;
    netbuf_t *nb1, *nb2;
    size_t sz1 = 20, sz2 = 10;

    RS_TEST_CASE_BEGIN(test_netbuf);

    TEST_ASSERT((xPool = xNetBufNewPool("NetBufSplit")));
    TEST_ASSERT((buf = pvRsMemAlloc(sz1)));

    TEST_ASSERT((nb1 = pxNetBufNew(xPool, NB_UNKNOWN, buf, sz1, &vTestFreeNormal)));
    TEST_ASSERT(nb1->unSz == 20);
    TEST_ASSERT(!ERR_CHK((xNetBufSplit(nb1, NB_UNKNOWN, sz2))));
    nb2 = pxNetBufNext(nb1);

    vNetBufFreeAll(nb2);

    TEST_ASSERT(xWasFreeCalled);
}

RS_TEST_CASE(NetBufBuildAppend, "[netbuf]")
{
    rsrcPoolP_t xPool;
    uint8_t *buf1, *buf2;
    size_t sz1 = 11, sz2 = 9;
    netbuf_t *nb1, *nb2;

    RS_TEST_CASE_BEGIN(test_netbuf);

    TEST_ASSERT((xPool = xNetBufNewPool("NetBufBuildAppend")));
    TEST_ASSERT((buf1 = pvRsMemAlloc(sz1)));
    TEST_ASSERT((buf2 = pvRsMemAlloc(sz2)));

    TEST_ASSERT((nb1 = pxNetBufNew(xPool, NB_UNKNOWN, buf1, sz1, &vTestFreeNormal)));
    TEST_ASSERT((nb2 = pxNetBufNew(xPool, NB_UNKNOWN, buf2, sz2, &vTestFreeNormal)));

    TEST_ASSERT(unNetBufTotalSize(nb1) == sz1);
    TEST_ASSERT(unNetBufTotalSize(nb2) == sz2);
    TEST_ASSERT(unNetBufCount(nb1) == 1);
    TEST_ASSERT(unNetBufCount(nb2) == 1);

    vNetBufAppend(nb1, nb2);

    TEST_ASSERT(unNetBufTotalSize(nb1) == sz1 + sz2);
    TEST_ASSERT(unNetBufTotalSize(nb2) == sz1 + sz2);
    TEST_ASSERT(unNetBufCount(nb1) == 2);

    vNetBufFree(nb1);

    TEST_ASSERT(!xWasFreeCalled);
    TEST_ASSERT(unNetBufCount(nb2) == 1);
    TEST_ASSERT(unNetBufTotalSize(nb2) == sz2);

    vNetBufFree(nb2);

    TEST_ASSERT(xWasFreeCalled);

    RS_TEST_CASE_END(test_netbuf);
}

RS_TEST_CASE(NetBufBuildAppend2, "[netbuf]")
{
    rsrcPoolP_t xPool;
    uint8_t *buf1, *buf2;
    size_t sz1 = 11, sz2 = 9;
    netbuf_t *nb1, *nb2;

    RS_TEST_CASE_BEGIN(test_netbuf);

    TEST_ASSERT((xPool = xNetBufNewPool("NetBufBuildAppend2")));
    TEST_ASSERT((buf1 = pvRsMemAlloc(sz1)));
    TEST_ASSERT((buf2 = pvRsMemAlloc(sz2)));

    TEST_ASSERT((nb1 = pxNetBufNew(xPool, NB_UNKNOWN, buf1, sz1, &vTestFreeNormal)));
    TEST_ASSERT((nb2 = pxNetBufNew(xPool, NB_UNKNOWN, buf2, sz2, &vTestFreeNormal)));

    vNetBufAppend(nb1, nb2);

    vNetBufFreeAll(nb2);

    TEST_ASSERT(xWasFreeCalled);

    RS_TEST_CASE_END(test_netbuf);
}

RS_TEST_CASE(NetBufBuildPrepend, "[netbuf]")
{
    rsrcPoolP_t xPool;
    uint8_t *buf1, *buf2;
    size_t sz1 = 11, sz2 = 9;
    netbuf_t *nb1, *nb2;

    RS_TEST_CASE_BEGIN(test_netbuf);

    TEST_ASSERT((xPool = xNetBufNewPool("NetBufBuildPrepend")));
    TEST_ASSERT((buf1 = pvRsMemAlloc(sz1)));
    TEST_ASSERT((buf2 = pvRsMemAlloc(sz2)));

    TEST_ASSERT((nb1 = pxNetBufNew(xPool, NB_UNKNOWN, buf1, sz1, &vTestFreeNormal)));
    TEST_ASSERT((nb2 = pxNetBufNew(xPool, NB_UNKNOWN, buf2, sz2, &vTestFreeNormal)));

    vNetBufAppend(nb2, nb1);

    TEST_ASSERT(unNetBufTotalSize(nb1) == sz1 + sz2);
    TEST_ASSERT(unNetBufTotalSize(nb2) == sz1 + sz2);
    TEST_ASSERT(unNetBufCount(nb1) == 2);

    vNetBufFree(nb1);

    TEST_ASSERT(!xWasFreeCalled);
    TEST_ASSERT(unNetBufCount(nb2) == 1);
    TEST_ASSERT(unNetBufTotalSize(nb2) == sz2);

    vNetBufFree(nb2);

    RS_TEST_CASE_END(test_netbuf);
}

RS_TEST_CASE(NetBufBuild3, "[netbuf]")
{
    rsrcPoolP_t xPool;
    uint8_t *buf1, *buf2, *buf3;
    size_t sz1 = 11, sz2 = 9, sz3 = 20;
    netbuf_t *nb1, *nb2, *nb3;

    RS_TEST_CASE_BEGIN(test_netbuf);

    TEST_ASSERT((xPool = xNetBufNewPool("NetBufBuildPrepend")));
    TEST_ASSERT((buf1 = pvRsMemAlloc(sz1)));
    TEST_ASSERT((buf2 = pvRsMemAlloc(sz2)));
    TEST_ASSERT((buf3 = pvRsMemAlloc(sz3)));

    TEST_ASSERT((nb1 = pxNetBufNew(xPool, NB_UNKNOWN, buf1, sz1, &vTestFreeNormal)));
    TEST_ASSERT((nb2 = pxNetBufNew(xPool, NB_UNKNOWN, buf2, sz2, &vTestFreeNormal)));
    TEST_ASSERT((nb3 = pxNetBufNew(xPool, NB_UNKNOWN, buf3, sz3, &vTestFreeNormal)));

    /* nb1 -> nb2 -> nb3 */
    vNetBufLink(nb1, nb2, nb3, 0);

    TEST_ASSERT(unNetBufTotalSize(nb1) == sz1 + sz2 + sz3);
    TEST_ASSERT(unNetBufTotalSize(nb3) == sz1 + sz2 + sz3);
    TEST_ASSERT(unNetBufCount(nb1) == 3);

    vNetBufFree(nb1);

    TEST_ASSERT(!xWasFreeCalled);
    TEST_ASSERT(unNetBufCount(nb2) == 2);
    TEST_ASSERT(unNetBufTotalSize(nb2) == sz2 + sz3);

    vNetBufFree(nb2);

    TEST_ASSERT(!xWasFreeCalled);
    TEST_ASSERT(unNetBufCount(nb2) == 1);
    TEST_ASSERT(unNetBufTotalSize(nb3) == sz3);

    vNetBufFree(nb3);

    TEST_ASSERT(xWasFreeCalled);

    RS_TEST_CASE_END(test_netbuf);
}

RS_TEST_CASE(NetBufRead1, "[netbuf]")
{
    rsrcPoolP_t xPool;
    uint8_t *buf1, *buf2;
    size_t sz1 = 2, sz2 = 2;
    netbuf_t *nb1;

    RS_TEST_CASE_BEGIN(test_netbuf);

    TEST_ASSERT((xPool = xNetBufNewPool("NetBufRead")));
    TEST_ASSERT((buf1 = pvRsMemAlloc(sz1)));
    TEST_ASSERT((buf2 = pvRsMemAlloc(sz2)));
    TEST_ASSERT((nb1 = pxNetBufNew(xPool, NB_UNKNOWN, buf1, sz1, &vTestFreeNormal)));

    buf1[0] = 1; buf1[1] = 2;

    /* Fully reading the buffer */
    TEST_ASSERT(unNetBufRead(nb1, buf2, 0, sz2) == sz1);
    TEST_ASSERT(buf2[0] + buf2[1] == 3);

    buf2[0] = buf2[1] = 0;

    /* Reading part of the buffer */
    TEST_ASSERT(unNetBufRead(nb1, buf2, 0, 1) == 1);
    TEST_ASSERT(buf2[0] == 1);

    buf2[0] = buf2[1] = 0;

    /* Reading past the buffer (we ask for 10 bytes but there is only
     * 2 in the netbuf so it should work) */
    TEST_ASSERT(unNetBufRead(nb1, buf2, 0, 10) == 2);
    TEST_ASSERT(buf2[0] == 1 && buf2[1] == 2);

    buf2[0] = buf2[1] = 0;

    /* Read offset 1 */
    TEST_ASSERT(unNetBufRead(nb1, buf2, 1, 1) == 1);
    TEST_ASSERT(buf2[0] == 2);

    /* Offset too far == nothing read */
    TEST_ASSERT(unNetBufRead(nb1, buf2, 10, 1) == 0);

    vNetBufFree(nb1);
    vRsMemFree(buf2);

    RS_TEST_CASE_END(test_netbuf);
}

RS_TEST_CASE(NetBufRead2, "[netbuf]")
{
    rsrcPoolP_t xPool;
    uint8_t *buf1, *buf2, *bufw;
    size_t sz1 = 2, sz2 = 2;
    netbuf_t *nb1, *nb2;

    RS_TEST_CASE_BEGIN(test_netbuf);

    TEST_ASSERT((xPool = xNetBufNewPool("NetBufRead")));
    TEST_ASSERT((buf1 = pvRsMemAlloc(sz1)));
    TEST_ASSERT((buf2 = pvRsMemAlloc(sz2)));
    TEST_ASSERT((bufw = pvRsMemAlloc(sz1 + sz2)));
    TEST_ASSERT((nb1 = pxNetBufNew(xPool, NB_UNKNOWN, buf1, sz1, &vTestFreeNormal)));
    TEST_ASSERT((nb2 = pxNetBufNew(xPool, NB_UNKNOWN, buf2, sz2, &vTestFreeNormal)));

    buf1[0] = 1; buf1[1] = 2;
    buf2[0] = 3; buf2[1] = 4;

    vNetBufLink(nb1, nb2, 0);

    /* Fully reading the buffers */
    TEST_ASSERT(unNetBufRead(nb1, bufw, 0, sz1 + sz2) == sz1 + sz2);
    TEST_ASSERT(bufw[0] == 1 && bufw[1] == 2 && bufw[2] == 3 && bufw[3] == 4);

    memset(bufw, 0, sz1 + sz2);

    /* Reading 2 bytes from offset 2, from the second buffer */
    TEST_ASSERT(unNetBufRead(nb1, bufw, sz1, sz2) == sz2);
    TEST_ASSERT(bufw[0] == 3 && bufw[1] == 4);

    memset(bufw, 0, sz1 + sz2);

    /* Reading too much bytes again */
    TEST_ASSERT(unNetBufRead(nb1, bufw, 0, 10) == sz1 + sz2);
    TEST_ASSERT(bufw[0] == 1 && bufw[1] == 2 && bufw[2] == 3 && bufw[3] == 4);

    memset(bufw, 0, sz1 + sz2);

    /* Reading 3 bytes from offset 1, spanning 2 buffers */
    TEST_ASSERT(unNetBufRead(nb1, bufw, 1, sz2 + 1) == sz2 + 1);
    TEST_ASSERT(bufw[0] == 2 && bufw[1] == 3 && bufw[2] == 4);

    vNetBufFree(nb1);
    vNetBufFree(nb2);
    vRsMemFree(bufw);

    RS_TEST_CASE_END(test_netbuf);
}

RS_TEST_CASE(NetBufReplace1, "[netbuf]")
{
    rsrcPoolP_t xPool;
    uint8_t *buf1, *buf2, *buf3;
    size_t sz1 = 2, sz2 = 2, sz3 = 10;
    netbuf_t *nb1, *nb2, *nb3;

    RS_TEST_CASE_BEGIN(test_netbuf);

    TEST_ASSERT((xPool = xNetBufNewPool("NetBufReplace1")));
    TEST_ASSERT((buf1 = pvRsMemAlloc(sz1)));
    TEST_ASSERT((buf2 = pvRsMemAlloc(sz2)));
    TEST_ASSERT((buf3 = pvRsMemAlloc(sz3)));
    TEST_ASSERT((nb1 = pxNetBufNew(xPool, NB_UNKNOWN, buf1, sz1, &vTestFreeNormal)));
    TEST_ASSERT((nb2 = pxNetBufNew(xPool, NB_UNKNOWN, buf2, sz2, &vTestFreeNormal)));
    TEST_ASSERT((nb3 = pxNetBufNew(xPool, NB_UNKNOWN, buf3, sz3, &vTestFreeNormal)));

    vNetBufLink(nb1, nb2, 0);

    TEST_ASSERT(unNetBufTotalSize(nb1) == sz1 + sz2);

    vNetBufFree(nb2);

    vNetBufAppend(nb1, nb3);

    TEST_ASSERT(unNetBufTotalSize(nb1) == sz1 + sz3);
    TEST_ASSERT(unNetBufTotalSize(nb3) == sz1 + sz3);
    TEST_ASSERT(pxNetBufNext(nb1) == nb3);

    vNetBufFree(nb1);
    vNetBufFree(nb3);

    RS_TEST_CASE_END(test_netbuf);
}

RS_TEST_CASE(NetBufReplace2, "[netbuf]")
{
    rsrcPoolP_t xPool;
    uint8_t *buf1, *buf2, *buf3;
    size_t sz1 = 2, sz2 = 2, sz3 = 10;
    netbuf_t *nb1, *nb2, *nb3;

    RS_TEST_CASE_BEGIN(test_netbuf);

    TEST_ASSERT((xPool = xNetBufNewPool("NetBufReplace2")));
    TEST_ASSERT((buf1 = pvRsMemAlloc(sz1)));
    TEST_ASSERT((buf2 = pvRsMemAlloc(sz2)));
    TEST_ASSERT((buf3 = pvRsMemAlloc(sz3)));
    TEST_ASSERT((nb1 = pxNetBufNew(xPool, NB_UNKNOWN, buf1, sz1, &vTestFreeNormal)));
    TEST_ASSERT((nb2 = pxNetBufNew(xPool, NB_UNKNOWN, buf2, sz2, &vTestFreeNormal)));
    TEST_ASSERT((nb3 = pxNetBufNew(xPool, NB_UNKNOWN, buf3, sz3, &vTestFreeNormal)));

    vNetBufLink(nb1, nb2, 0);

    TEST_ASSERT(unNetBufTotalSize(nb1) == sz1 + sz2);

    vNetBufFree(nb1);

    vNetBufAppend(nb3, nb2);

    TEST_ASSERT(unNetBufTotalSize(nb2) == sz2 + sz3);
    TEST_ASSERT(unNetBufTotalSize(nb3) == sz2 + sz3);
    TEST_ASSERT(pxNetBufNext(nb3) == nb2);

    vNetBufFree(nb2);
    vNetBufFree(nb3);

    RS_TEST_CASE_END(test_netbuf);
}

#ifndef TEST_CASE
int main() {
    RS_SUITE_BEGIN();
    RS_RUN_TEST(NetBufSimple);
    RS_RUN_TEST(NetBufSimpleFree);
    RS_RUN_TEST(NetBufSplit);
    RS_RUN_TEST(NetBufSplit2);
    RS_RUN_TEST(NetBufBuildAppend);
    RS_RUN_TEST(NetBufBuildAppend2);
    RS_RUN_TEST(NetBufBuild3);
    RS_RUN_TEST(NetBufRead1);
    RS_RUN_TEST(NetBufRead2);
    RS_RUN_TEST(NetBufReplace1);
    RS_RUN_TEST(NetBufReplace2);
    RS_SUITE_END();
}
#endif
