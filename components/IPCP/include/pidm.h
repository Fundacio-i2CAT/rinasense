/**
 *
 *        Port Id Manager:
 *       Managing the PortIds allocated in the whole stack. PortIds in IPCP Normal
 *       and Shim Wifi.
 *
 **/

#ifndef PIDM_H__INCLUDED
#define PIDM_H__INCLUDED

typedef struct xPIDM
{
        /*List of Allocated Ports*/
        List_t xAllocatedPorts;

        /*The last Allocated Port*/
        portId_t xLastAllocated;

} pidm_t;

typedef struct xALLOC_PID
{
        /*List Item to register the Pid into the xAllocatedPorts List*/
        ListItem_t xPortIdItem;

        /*Port Id allocated*/
        portId_t xPid;

} allocPid_t;

pidm_t *pxPidmCreate(void);

portId_t xPidmAllocate(pidm_t *pxInstance);

#endif