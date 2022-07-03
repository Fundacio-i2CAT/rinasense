#include <string.h>

#include "portability/port.h"
#include "portability/posix/mqueue.h"

#include "rina_gpha.h"

#include "IPCP_api.h"
#include "IPCP_events.h"
#include "BufferManagement.h"
#include "NetworkInterface.h"
#include "NetworkInterface_mq.h"

/* The 'mq' NetworkInterface variant uses POSIX messages queues to
 * emulate a network interface. This is to be used for testing
 * only. */

/* Incoming queue. */
static mqd_t mqIn;

/* Outgoing queue */
static mqd_t mqOut;

static struct mq_attr mqInitAttr = {
    .mq_flags = 0,
    .mq_maxmsg = 6,
    .mq_msgsize = 1500,
    .mq_curmsgs = 0
};

/* Test API */

/*
 * Read a message from the message queue.
 *
 * Use this to check what was written by xNetworkInterfaceOutput.
 */
bool_t xMqNetworkInterfaceReadOutput(string_t data, size_t sz)
{
    struct timespec ts = {
        .tv_sec = 5,
        .tv_nsec = 0
    };

    /* Make it a programming error not to have SZ at least big enough
       for a standard Ethernet packet. */
    RsAssert(sz >= mqInitAttr.mq_msgsize);

    if (mq_timedreceive(mqOut, data, sz, NULL, &ts) < 0)
        return false;

    return true;
}

bool_t xMqNetworkInterfaceWriteInput(string_t data, size_t sz)
{
    RsAssert(sz >= mqInitAttr.mq_msgsize);

    if (mq_send(mqIn, data, sz, 0) < 0)
        return false;

    return true;
}

/* Event handler */

static void event_handler(union sigval sv)
{
    struct mq_attr mqattr;
    char *buf;

    if (mq_getattr(mqIn, &mqattr) < 0)
        LOGE(TAG_WIFI, "Failed to get size inbound packet");

    buf = pvRsMemAlloc(mqattr.mq_msgsize);

    if (mq_receive(mqIn, buf, mqattr.mq_msgsize, NULL) < 0)
        LOGE(TAG_WIFI, "Failed to read inbound packet");

    LOGD(TAG_WIFI, "Received %lu bytes for the RINA stack", mqattr.mq_msgsize);

    if (!xNetworkInterfaceInput(buf, mqattr.mq_msgsize, NULL))
        LOGE(TAG_WIFI, "RX processing failed");

    vRsMemFree(buf);
}

/* Public interface */

bool_t xNetworkInterfaceInitialise(const MACAddress_t *pxPhyDev)
{
    return true;
}

bool_t xNetworkInterfaceConnect(void)
{
    struct sigevent se;

    mqIn = mq_open("/testIn", O_RDWR | O_CREAT, 0644, &mqInitAttr);
    if (mqIn < 0) {
        LOGE(TAG_WIFI, "Failed to create inbound queue");
        goto err;
    }
    mqOut = mq_open("/testOut", O_RDWR | O_CREAT, 0644, &mqInitAttr);
    if (mqOut < 0) {
        LOGE(TAG_WIFI, "Failed to create outbound queue");
        goto err;
    }

    se.sigev_notify = SIGEV_THREAD;
    se.sigev_notify_function = event_handler;
    se.sigev_notify_attributes = NULL;
    se.sigev_value.sival_ptr = &mqIn;

    /* Get notified of INCOMING messages. */
    if (mq_notify(mqIn, &se) < 0) {
        LOGE(TAG_WIFI, "Failed to setup notification for incoming messages");
        goto err;
    }

    LOGD(TAG_WIFI, "MQ-NetworkInterface initialized");

    return true;

    err:
    if (mqIn > 0)
        mq_close(mqIn);
    if (mqOut > 0)
        mq_close(mqOut);

    return false;
}

bool_t xNetworkInterfaceDisconnect(void)
{
    int r1 = 0, r2 = 0;

    if (mqIn > 0) {
        r1 = mq_close(mqIn);
        mq_unlink("/testIn");
    }
    if (mqOut > 0) {
        r2 = mq_close(mqOut);
        mq_unlink("/testOut");
    }

    mqIn = 0;
    mqOut = 0;

    return r1 == 0 && r2 == 0;
}

bool_t xNetworkInterfaceOutput(NetworkBufferDescriptor_t *const pxNetworkBuffer,
                               bool_t xReleaseAfterSend)
{
    if (!mq_send(mqOut, (const char *)pxNetworkBuffer->pucEthernetBuffer, pxNetworkBuffer->xDataLength, 0)) {
        LOGD(TAG_WIFI, "Wrote %u bytes to network interface", pxNetworkBuffer->xDataLength);
        return true;
    }

    return false;
}

bool_t xNetworkInterfaceInput(void *buffer, uint16_t len, void *eb)
{
	NetworkBufferDescriptor_t *pxNetworkBuffer;
	RINAStackEvent_t xRxEvent = {eNetworkRxEvent, NULL};
    struct timespec ts = { 0 };

	pxNetworkBuffer = pxGetNetworkBufferWithDescriptor(len, &ts);

	if (pxNetworkBuffer != NULL)
	{
		/* Set the packet size, in case a larger buffer was returned. */
		pxNetworkBuffer->xDataLength = len;

		/* Copy the packet data. */
		memcpy(pxNetworkBuffer->pucEthernetBuffer, buffer, len);
		xRxEvent.pvData = (void *)pxNetworkBuffer;

		if (xSendEventStructToIPCPTask(&xRxEvent, &ts) == false)
		{
			LOGE(TAG_WIFI, "Failed to enqueue packet to network stack %p, len %d", buffer, len);
			vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
			return false;
		}

        return true;
	}

	else
	{
		LOGE(TAG_WIFI, "Failed to get buffer descriptor");
		vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
		return false;
	}
}

void vNetworkNotifyIFDown()
{
}

void vNetworkNotifyIFUp()
{
}
