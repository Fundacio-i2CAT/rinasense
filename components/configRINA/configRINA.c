#include <stdio.h>
#include "configRINA.h"

#if 0
void func(void)
{
}

void vTCPStateChange(FreeRTOS_Socket_t *pxSocket,
					 enum eTCP_STATE eTCPState)
{
	FreeRTOS_Socket_t *xParent = NULL;
	BaseType_t bBefore = tcpNOW_CONNECTED((BaseType_t)pxSocket->u.xTCP.ucTCPState); /* Was it connected ? */ //pdFALSE
	BaseType_t bAfter = tcpNOW_CONNECTED((BaseType_t)eTCPState);					/* Is it connected now ? */ //pdTRUE

#if (ipconfigHAS_DEBUG_PRINTF != 0)
	BaseType_t xPreviousState = (BaseType_t)pxSocket->u.xTCP.ucTCPState;
#endif
#if (ipconfigUSE_CALLBACKS == 1)
	FreeRTOS_Socket_t *xConnected = NULL;
#endif

	/* Has the connected status changed? */
	if (bBefore != bAfter)
	{
		/* Is the socket connected now ? */
		if (bAfter != pdFALSE)
		{
			/* if bPassQueued is true, this socket is an orphan until it gets connected. */
			if (pxSocket->u.xTCP.bits.bPassQueued != pdFALSE_UNSIGNED)
			{
				/* Now that it is connected, find it's parent. */
				if (pxSocket->u.xTCP.bits.bReuseSocket != pdFALSE_UNSIGNED)
				{
					xParent = pxSocket;
				}
				else
				{
					xParent = pxSocket->u.xTCP.pxPeerSocket;
					configASSERT(xParent != NULL);
				}

				if (xParent != NULL)
				{
					if (xParent->u.xTCP.pxPeerSocket == NULL)
					{
						xParent->u.xTCP.pxPeerSocket = pxSocket;
					}

					xParent->xEventBits |= (EventBits_t)eSOCKET_ACCEPT;

#if (ipconfigSUPPORT_SELECT_FUNCTION == 1)
					{
						/* Library support FreeRTOS_select().  Receiving a new
						 * connection is being translated as a READ event. */
						if ((xParent->xSelectBits & ((EventBits_t)eSELECT_READ)) != 0U)
						{
							xParent->xEventBits |= ((EventBits_t)eSELECT_READ) << SOCKET_EVENT_BIT_COUNT;
						}
					}
#endif

#if (ipconfigUSE_CALLBACKS == 1)
					{
						if ((ipconfigIS_VALID_PROG_ADDRESS(xParent->u.xTCP.pxHandleConnected)) &&
							(xParent->u.xTCP.bits.bReuseSocket == pdFALSE_UNSIGNED))
						{
							/* The listening socket does not become connected itself, in stead
							 * a child socket is created.
							 * Postpone a call the OnConnect event until the end of this function. */
							xConnected = xParent;
						}
					}
#endif
				}

				/* Don't need to access the parent socket anymore, so the
				 * reference 'pxPeerSocket' may be cleared. */
				pxSocket->u.xTCP.pxPeerSocket = NULL;
				pxSocket->u.xTCP.bits.bPassQueued = pdFALSE_UNSIGNED;

				/* When true, this socket may be returned in a call to accept(). */
				pxSocket->u.xTCP.bits.bPassAccept = pdTRUE_UNSIGNED;
			}
			else
			{
				pxSocket->xEventBits |= (EventBits_t)eSOCKET_CONNECT;

#if (ipconfigSUPPORT_SELECT_FUNCTION == 1)
				{
					if ((pxSocket->xSelectBits & ((EventBits_t)eSELECT_WRITE)) != 0U)
					{
						pxSocket->xEventBits |= ((EventBits_t)eSELECT_WRITE) << SOCKET_EVENT_BIT_COUNT;
					}
				}
#endif
			}
		}
		else /* bAfter == pdFALSE, connection is closed. */
		{
			/* Notify/wake-up the socket-owner by setting a semaphore. */
			pxSocket->xEventBits |= (EventBits_t)eSOCKET_CLOSED;

#if (ipconfigSUPPORT_SELECT_FUNCTION == 1)
			{
				if ((pxSocket->xSelectBits & (EventBits_t)eSELECT_EXCEPT) != 0U)
				{
					pxSocket->xEventBits |= ((EventBits_t)eSELECT_EXCEPT) << SOCKET_EVENT_BIT_COUNT;
				}
			}
#endif
		}

#if (ipconfigUSE_CALLBACKS == 1)
		{
			if ((ipconfigIS_VALID_PROG_ADDRESS(pxSocket->u.xTCP.pxHandleConnected)) && (xConnected == NULL))
			{
				/* The 'connected' state has changed, call the user handler. */
				xConnected = pxSocket;
			}
		}
#endif /* ipconfigUSE_CALLBACKS */

		if (prvTCPSocketIsActive(pxSocket->u.xTCP.ucTCPState) == 0)
		{
			/* Now the socket isn't in an active state anymore so it
			 * won't need further attention of the IP-task.
			 * Setting time-out to zero means that the socket won't get checked during
			 * timer events. */
			pxSocket->u.xTCP.usTimeout = 0U;
		}
	}
	else
	{
		if ((eTCPState == eCLOSED) ||
			(eTCPState == eCLOSE_WAIT))
		{
			/* Socket goes to status eCLOSED because of a RST.
			 * When nobody owns the socket yet, delete it. */
			if ((pxSocket->u.xTCP.bits.bPassQueued != pdFALSE_UNSIGNED) ||
				(pxSocket->u.xTCP.bits.bPassAccept != pdFALSE_UNSIGNED))
			{
				FreeRTOS_debug_printf(("vTCPStateChange: Closing socket\n"));

				if (pxSocket->u.xTCP.bits.bReuseSocket == pdFALSE_UNSIGNED)
				{
					configASSERT(xIsCallingFromIPTask() != pdFALSE);
					vSocketCloseNextTime(pxSocket);
				}
			}
		}
	}

	/* Fill in the new state. */
	pxSocket->u.xTCP.ucTCPState = (uint8_t)eTCPState;

	/* Touch the alive timers because moving to another state. */
	prvTCPTouchSocket(pxSocket);

#if (ipconfigHAS_DEBUG_PRINTF == 1)
	{
		if ((xTCPWindowLoggingLevel >= 0) && (ipconfigTCP_MAY_LOG_PORT(pxSocket->usLocalPort)))
		{
			FreeRTOS_debug_printf(("Socket %u -> %xip:%u State %s->%s\n",
								   pxSocket->usLocalPort,
								   (unsigned)pxSocket->u.xTCP.ulRemoteIP,
								   pxSocket->u.xTCP.usRemotePort,
								   FreeRTOS_GetTCPStateName((UBaseType_t)xPreviousState),
								   FreeRTOS_GetTCPStateName((UBaseType_t)eTCPState)));
		}
	}
#endif /* ipconfigHAS_DEBUG_PRINTF */

#if (ipconfigUSE_CALLBACKS == 1)
	{
		if (xConnected != NULL)
		{
			/* The 'connected' state has changed, call the OnConnect handler of the parent. */
			xConnected->u.xTCP.pxHandleConnected((Socket_t)xConnected, bAfter);
		}
	}
#endif

	if (xParent != NULL)
	{
		vSocketWakeUpUser(xParent);
	}
}
#endif