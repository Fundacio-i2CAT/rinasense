#include <sys/ioctl.h>
#include <linux/netlink.h>
#include <linux/if_tun.h>
#include <linux/rtnetlink.h>
#include <net/if.h>

#include <pthread.h>
#include <bits/pthreadtypes.h>
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "common/mac.h"
#include "configSensor.h"
#include "configRINA.h"
#include "linux_rsdefs.h"
#include "portability/port.h"
#include "common/rina_gpha.h"

#include "ARP826_defs.h"
#include "IPCP_api.h"
#include "IPCP_events.h"
#include "BufferManagement.h"
#include "NetworkInterface.h"
#include "ShimIPCP.h"
#include "rina_common_port.h"

struct ReadThreadParams {
    int nTapFD;
    int nPipe;
};

/* The MTU of a TAP device is always 1500. */
#define TAP_MTU    1500

#define CLONE_DEV  "/dev/net/tun"

/* The 2 following functions are part of the original ESP32
 * NetworkInterface API, but not in the header file. This keeps them
 * around but I'm not sure if there is an end goal for them */
void vNetworkNotifyIFDown();

void vNetworkNotifyIFUp();

/* File descriptor to the tap device we create. */
static int nTapFD = -1;

/* File descriptor to use to talk to the TAP read thread. */
static int nTapThreadFD = -1;

/* File descriptor for monitoring the network interface. */
static int nMonitorFD = -1;

/* Params passed to the TAP read thread. */
static struct ReadThreadParams xTapReadTreadParams;

/* Reading thread. */
static pthread_t xReadThread;

static bool_t xIsInterfaceConnected = false;

/* Mutex used to protecte access to nCurrentIffFlags. */
static pthread_mutex_t xIffDataMutex;

/* Current flags of the interface we use, you need to hold the
 * "xIffDataMutex" to consult this variable. */
static int nCurrentIffFlags;

/* MAC address */
const MACAddress_t *pxMacAddr;

/* TEMPORARY: blind pointer to the IPCP instance data so we can pass
 * it to the shim when we receive packets. */
struct ipcpInstance_t *pxSelf;

bool_t prvLinuxGetMac(string_t sTapName, MACAddress_t *pxMac)
{
    int fd = -1;
    string_t sIfaceMacFmt = "/sys/class/net/%s/address";
    stringbuf_t sIfaceMacPath[PATH_MAX];
    stringbuf_t sMac[18];
    string_t psMac;
    int n;

    snprintf(sIfaceMacPath, PATH_MAX, sIfaceMacFmt, sTapName);

    if ((fd = open(sIfaceMacPath, O_RDONLY)) < 0) {
        LOGE(TAG_WIFI, "Unable to open MAC address file: %s", sIfaceMacPath);
        return false;
    }

    if (read(fd, sMac, sizeof(sMac)) < 0) {
        LOGE(TAG_WIFI, "Read error reading MAC address!");
        goto err;
    } else {
        sMac[17] = '\0';
        n = sscanf(sMac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                   &(pxMac->ucBytes[0]), &(pxMac->ucBytes[1]),
                   &(pxMac->ucBytes[2]), &(pxMac->ucBytes[3]),
                   &(pxMac->ucBytes[4]), &(pxMac->ucBytes[5]));
        if (n < 6)
            goto err;
    }

    close(fd);
    return true;

    err:
    if (fd > 0)
        close(fd);

    return false;
}

bool_t prvLinuxTapOpen(string_t sTapName, int *pnFD)
{
    struct ifreq ifr;
    int fd, err;

    /* open the clone device */
    if ((fd = open(CLONE_DEV, O_RDWR)) < 0)
        return fd;

    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */
    strncpy(ifr.ifr_name, sTapName, IFNAMSIZ);

    /* Try to create the device */
    if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
        close(fd);
        return false;
    }

    *pnFD = fd;

    /* this is the special file descriptor that the caller will use to talk
     * with the virtual interface */
    return true;
}

/* Return the flags associated with the device, this also returns a
 * file description which should be passed to prvLinuxTapSetFlags, or
 * closed if not needed. */
bool_t prvLinuxTapGetFlags(string_t sTapName, int *pFD, int *pnFlags)
{
    int err, nFD;
    struct ifreq ifr;

    /* Get the flags that are set */
    nFD = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (nFD < 0 )
        return false;

    /* Put the name of the device we want to peek. */
    strncpy(ifr.ifr_name, sTapName, IFNAMSIZ);

    /* Get the interface flags first. */
    err = ioctl(nFD, SIOCGIFFLAGS, (void*)&ifr);
    if (err < 0) {
        close(nFD);
        return false;
    }

    if (pnFlags != NULL)
        *pnFlags = ifr.ifr_flags;

    if (pFD != NULL)
        *pFD = nFD;
    else
        close(nFD);

    return true;
}

/* This sets the flags on an interface using the FD opened with
 * prvLinuxTapGetFlags. Make sure to close it when you're done. */
bool_t prvLinuxTapSetFlags(string_t sTapName, int nFD, int nFlags)
{
    int err;
    struct ifreq ifr;

    ifr.ifr_flags = nFlags;
    strncpy(ifr.ifr_name, sTapName, IFNAMSIZ);

    err = ioctl(nFD, SIOCSIFFLAGS, (void*) &ifr);
    if (err < 0)
        return err;

    return true;
}

bool_t prvLinuxTapSetUp(string_t sTapName)
{
    int nFD, nFlags;
    bool bStatus;

    if (!prvLinuxTapGetFlags(sTapName, &nFD, &nFlags))
        return false;

    nFlags |= (IFF_UP | IFF_RUNNING);

    bStatus = prvLinuxTapSetFlags(sTapName, nFD, nFlags);
    close(nFD);

    return bStatus;
}

bool_t prvLinuxTapSetDown(string_t sTapName)
{
    int nFD, nFlags;
    bool bStatus;

    if (!prvLinuxTapGetFlags(sTapName, &nFD, &nFlags))
        return false;

    nFlags |= ~(IFF_UP | IFF_RUNNING);

    bStatus = prvLinuxTapSetFlags(sTapName, nFD, nFlags);
    close(nFD);

    return bStatus;
}

void *prvLinuxTapReadThread(void *pvParams)
{
    struct ReadThreadParams *pxParams;
    uint8_t buffer[TAP_MTU + sizeof(EthernetHeader_t)];
    struct pollfd pfds[2];
    int nPoll;

    pxParams = (struct ReadThreadParams *)pvParams;

    /* Prepare the poll descriptors. */
    pfds[0].fd = pxParams->nTapFD;
    pfds[0].events = POLLIN | POLLERR;
    pfds[1].fd = pxParams->nPipe;
    pfds[1].events = POLLIN | POLLERR;

    LOGI(TAG_WIFI, "Read thread on device %s STARTED", LINUX_TAP_DEVICE);

    while (1) {
        nPoll = poll(pfds, 2, 0);

        if (pfds[0].revents & POLLIN) {
            ssize_t count = read(nTapFD, buffer, sizeof(buffer));

            /* Hang ups are not normal here. */
            if (count < 0 || count == 0) break;

            /* Otherwise, this is an actual packet anId send it up the
               stack. */
            else xNetworkInterfaceInput(buffer, count, NULL);
        }
        else if (pfds[1].revents & POLLIN) {
            /* The interface is down, just leave. The management
             * functions will deal with restarting the thread. */
            LOGD(TAG_WIFI, "Read thread found device %s is DOWN", LINUX_TAP_DEVICE);
            return NULL;
        }
    }

    LOGE(TAG_WIFI, "Read thread on device %s EXITED! (This is not normal!)", LINUX_TAP_DEVICE);
    return NULL;
}

bool_t prvLinuxTapCheckFlags(string_t sTapName, int nFlags)
{
    int n;

    pthread_mutex_lock(&xIffDataMutex);

    n = (nCurrentIffFlags & nFlags);

    pthread_mutex_unlock(&xIffDataMutex);

    return n > 0;
}

void prvLinuxTapSaveFlags(int nFlags)
{
    pthread_mutex_lock(&xIffDataMutex);

    nCurrentIffFlags = nFlags;

    pthread_mutex_unlock(&xIffDataMutex);
}

void prvLinuxTapParseNewLinkMsg(struct nlmsghdr *pxNewLinkMsg)
{
    struct ifinfomsg *ifi;
    bool_t isNowUp, isNowDown;
    stringbuf_t cIfNam[IFNAMSIZ];

    ifi = NLMSG_DATA(pxNewLinkMsg);

    RsAssert(if_indextoname(ifi->ifi_index, cIfNam) != 0);

    /* We obviously only care about the interface we were
       configured to work with. */
    if (strcmp(cIfNam, LINUX_TAP_DEVICE) == 0) {
        pthread_mutex_lock(&xIffDataMutex);

        /* Check if we have a status transition */
        isNowUp = ((ifi->ifi_flags & IFF_UP) && !(nCurrentIffFlags & IFF_UP));
        isNowDown = (!(ifi->ifi_flags & IFF_UP) && (nCurrentIffFlags & IFF_UP));

        /* Save the new flags. */
        nCurrentIffFlags = ifi->ifi_flags;

        pthread_mutex_unlock(&xIffDataMutex);

        if (isNowUp)
            vNetworkNotifyIFUp();
        if (isNowDown)
            vNetworkNotifyIFDown();
    }
}

void *prvLinuxTapMonitorThread(void *pNothing)
{
    struct nlmsghdr *nlh;
    int nMonitorFD;
    char buf[4096 * 2];
    struct msghdr msg;
    struct sockaddr_nl xLocalAddr;
    struct iovec xIov;
    struct nlmsghdr *pxMsg;
    ssize_t nRecv = 0;

    msg.msg_name = &xLocalAddr;
    msg.msg_namelen = sizeof(struct sockaddr_nl);
    msg.msg_iov = &xIov;
    msg.msg_iovlen = 1;

    bzero(&xLocalAddr, sizeof(struct sockaddr_nl));
    xLocalAddr.nl_family = AF_NETLINK;
    xLocalAddr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE;

    nMonitorFD = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

    if (nMonitorFD < 0) {
        LOGE(TAG_WIFI, "Failed to open netlink socket");
        return NULL;
    }

    if (bind(nMonitorFD, (struct sockaddr *)&xLocalAddr, sizeof(xLocalAddr)) < 0) {
        LOGE(TAG_WIFI, "Failed to bind() to netlink socket");
        close(nMonitorFD);
        return NULL;
    }

    while (1) {
        /* Receiving netlink socket data */
        if (nRecv == 0)
            nRecv = recv(nMonitorFD, buf, sizeof(buf), 0);

        if (nRecv < 0) {
            LOGE(TAG_WIFI, "Read error on netlink socket (errno: %d)", errno);
            goto err;
        }

        pxMsg = (struct nlmsghdr *)buf;

        for (; NLMSG_OK(pxMsg, nRecv); pxMsg = NLMSG_NEXT(pxMsg, nRecv)) {

            /* This means we are done reading netlink messages */
            if (pxMsg->nlmsg_type == NLMSG_DONE)
                break;

            /* We need to parse this specific message. */
            if (pxMsg->nlmsg_type == RTM_NEWLINK)
                prvLinuxTapParseNewLinkMsg(pxMsg);
        }
    }

    err:
    LOGE(TAG_WIFI, "Interface monitoring thread EXITED! (This is not a normal situation)");
    return NULL;
}

bool_t prvLinuxTapStartReadThread(int *pnTapFD)
{
    pthread_attr_t xAttr;
    bool_t bStatus = true;
    int nPipe[2];

    /* Setup a pipe so we can communicate with the read thread. */
    if (pipe(nPipe) < 0)
        return false;

    /* Make the pipe and the TAP FD non-blocking so that we can use
       poll(). */
    if (fcntl(nTapFD, F_SETFD, O_NONBLOCK) < 0 ||
        fcntl(nPipe[0], F_SETFD, O_NONBLOCK) < 0) {
        LOGE(TAG_WIFI, "Error setting up FDs for the thread");

        close(nPipe[0]);
        close(nPipe[1]);
        return false;
    }

    nTapThreadFD = nPipe[1];

    xTapReadTreadParams.nPipe = nPipe[0];
    xTapReadTreadParams.nTapFD = nTapFD;

    pthread_attr_init(&xAttr);

    if (pthread_attr_setdetachstate(&xAttr, PTHREAD_CREATE_DETACHED) != 0)
        bStatus = false;

    if (pthread_create(&xReadThread, &xAttr, prvLinuxTapReadThread, &xTapReadTreadParams) != 0)
        bStatus = false;

    pthread_attr_destroy(&xAttr);

    return bStatus;
}

bool_t prvLinuxTapStartMonitorThread()
{
    pthread_attr_t xAttr;
    bool_t bStatus = false;

    pthread_mutex_init(&xIffDataMutex, NULL);

    pthread_attr_init(&xAttr);

    if (pthread_attr_setdetachstate(&xAttr, PTHREAD_CREATE_DETACHED) != 0)
        bStatus = false;

    if (pthread_create(&xReadThread, &xAttr, prvLinuxTapMonitorThread, NULL) != 0)
        bStatus = false;

    pthread_attr_destroy(&xAttr);

    return bStatus;
}

static inline void prvGenRandomEth(uint8_t *pAddr)
{
    for(int i = 0; i < 6; i++)
        pAddr[i] = (uint8_t)(rand() % CHAR_MAX);

    pAddr[0] &= 0xfe;	/* clear multicast bit */
    pAddr[0] |= 0x02;	/* set local assignment bit (IEEE802) */
}

/* Public interface */

bool_t xNetworkInterfaceInitialise(struct ipcpInstance_t *pxS, MACAddress_t *pxPhyDev)
{
    pxSelf = pxS;

#if LINUX_TAP_CREATE == true
    /* If we're in create mode, return an error if the TAP device
     * we're asked to create already exists. */

    if (prvLinuxTapGetFlags(LINUX_TAP_DEVICE, NULL, NULL)) {
        LOGE(TAG_WIFI, "Device %s already exists but we were asked to create it", LINUX_TAP_DEVICE);
        return false;
    }
#endif

    /* Otherwise, just open it. */
    if (!prvLinuxTapOpen(LINUX_TAP_DEVICE, &nTapFD)) {
        LOGE(TAG_WIFI, "Failed to open TAP device %s", LINUX_TAP_DEVICE);
        return false;
    }

    /* Capture the flags on the device before starting the monitoring
     * thread. */
    prvLinuxTapGetFlags(LINUX_TAP_DEVICE, NULL, &nCurrentIffFlags);

    /* Generate a random ethernet address for the outgoing packets. */
    prvGenRandomEth(pxPhyDev->ucBytes);
    pxMacAddr = pxPhyDev;

    /* Start the thread to monitor the interface. This has to be
     * always running. */
    prvLinuxTapStartMonitorThread();

    return true;
}

bool_t xNetworkInterfaceConnect(void)
{
    /* Put UP the device if management is enabled. */
#if LINUX_TAP_MANAGE == true
    if (!prvLinuxTapUp(TAP_DEVICE)) {
        LOGE(TAG_WIFI, "Failed to bring device %s UP", LINUX_TAP_DEVICE);
        return false;
    }
    else LOGI(TAG_WIFI, "Device %s is now UP", LINUX_TAP_DEVICE);
#endif

    /* Start the read thread only if the device is up. */
    if (prvLinuxTapCheckFlags(LINUX_TAP_DEVICE, IFF_UP)) {

        /* Spin up the thread that will do the blocking read for packets on
         * the TAP device. */
        if (!prvLinuxTapStartReadThread(&nTapFD)) {
            LOGE(TAG_WIFI, "Failed to spin up read thread on device %s", LINUX_TAP_DEVICE);
            return false;
        }
        else LOGI(TAG_WIFI, "Thread reading on device %s is running", LINUX_TAP_DEVICE);
    }
    else
        LOGW(TAG_WIFI, "Device %s is DOWN, not starting read thread.", LINUX_TAP_DEVICE);

    xIsInterfaceConnected = true;

    return true;
}

bool_t xNetworkInterfaceDisconnect(void)
{
    /* Take down and dispose of the tap device if we were configured
     * to do so automatically. */
#if LINUX_TAP_MANAGE == true
    if (!prvLinuxTapDown(TAP_DEVICE)) {
        LOGE(TAG_WIFI, "Failed to take device %s DOWN", LINUX_TAP_DEVICE);
        return false;
    }
    else LOGI(TAG_WIFI, "Device %s is now DOWN", LINUX_TAP_DEVICE);
#endif

    xIsInterfaceConnected = false;

    return true;
}

bool_t xNetworkInterfaceOutput(NetworkBufferDescriptor_t *const pxNetworkBuffer,
                               bool_t xReleaseAfterSend)
{
    /* Write the packet on the device. */
    ssize_t nWriteSz;
    size_t unWriteSzLeft = pxNetworkBuffer->xDataLength;
    size_t unLastWriteSz = 0;

    if (!prvLinuxTapCheckFlags(LINUX_TAP_DEVICE, IFF_UP)) {
        LOGE(TAG_WIFI, "Writing %zu bytes on interface %s failed, interface is DOWN",
             pxNetworkBuffer->xDataLength, LINUX_TAP_DEVICE);
        return false;
    }

    do {
        nWriteSz = write(nTapFD, pxNetworkBuffer->pucEthernetBuffer + unLastWriteSz, unWriteSzLeft);

        if (nWriteSz < 0) {
            LOGD(TAG_WIFI, "Write error (errno: %d)", errno);
            return false;
        }

        unWriteSzLeft -= nWriteSz;
        unLastWriteSz = nWriteSz;
    } while (unWriteSzLeft > 0);

    LOGD(TAG_WIFI, "Wrote %zu bytes to the network", pxNetworkBuffer->xDataLength);

    if (xReleaseAfterSend)
        vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);

    return true;
}

/* Called by the tap read thread to advertises that an ethernet
 * frame has arrived. */
bool_t xNetworkInterfaceInput(void *buffer, uint16_t len, void *eb)
{
	NetworkBufferDescriptor_t *pxNetworkBuffer;
    EthernetHeader_t *pxEthernetHeader;
	RINAStackEvent_t xRxEvent = {
        .eEventType = eNetworkRxEvent,
        .xData.PV = NULL
    };

	if (eConsiderFrameForProcessing(buffer) != eProcessBuffer)
		return true;

    pxNetworkBuffer = pxGetNetworkBufferWithDescriptor(len, 250 * 1000);

	if (pxNetworkBuffer != NULL) {
		/* Set the packet size, in case a larger buffer was returned. */
		pxNetworkBuffer->xEthernetDataLength = len;

        /* We need to look into the frame data. */
        pxEthernetHeader = (EthernetHeader_t *)buffer;

        /* If it's a broadcasted packet, replace the target address in
         * the frame by ours. The rest of the code should not have to
         * deal with this. */
        if (xIsBroadcastMac(&pxEthernetHeader->xDestinationAddress)) {
            LOGD(TAG_WIFI, "Handling broadcasted ethernet packet.");
            memcpy(&pxEthernetHeader->xDestinationAddress, pxMacAddr->ucBytes, sizeof(MACAddress_t));

        } else {
            /* Make sure this is actually meant from us. */
            if (memcmp(&pxEthernetHeader->xDestinationAddress, pxMacAddr->ucBytes, sizeof(MACAddress_t)) != 0) {
#ifndef NDEBUG
                {
                    stringbuf_t ucMac[MAC2STR_MIN_BUFSZ];
                    mac2str(&pxEthernetHeader->xDestinationAddress, ucMac, MAC2STR_MIN_BUFSZ);
                    LOGW(TAG_WIFI, "Dropping packet with destination %s, not for us", ucMac);
                }
#else
                LOGW(TAG_WIFI, "Dropping ethernet packet not destined for us")
#endif
                return false;
            }
        }

		/* Copy the packet data. */
		memcpy(pxNetworkBuffer->pucEthernetBuffer, buffer, len);
        vShimHandleEthernetPacket(pxSelf, pxNetworkBuffer);

		return true;
	}
	else {
        LOGE(TAG_WIFI, "Failed to get buffer descriptor");
        vReleaseNetworkBufferAndDescriptor(pxNetworkBuffer);
        return false;
    }
}

void vNetworkNotifyIFDown()
{
    char n = 99; /* Random crap to wake up the read thread. */

    LOGW(TAG_WIFI, "Interface %s is now DOWN", LINUX_TAP_DEVICE);

    if (xIsInterfaceConnected)
        write(nTapThreadFD, &n, 1);
}

void vNetworkNotifyIFUp()
{
    LOGW(TAG_WIFI, "Interface %s is now UP", LINUX_TAP_DEVICE);

    if (xIsInterfaceConnected)
        prvLinuxTapStartReadThread(&nTapFD);
}
