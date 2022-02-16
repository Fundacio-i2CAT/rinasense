
#ifndef PIDM_H__INCLUDED
#define PIDM_H__INCLUDED

typedef struct xPIDM {
	List_t xAllocatedPorts;
	portId_t xLastAllocated;
}pidm_t;

typedef struct xALLOC_PID {
        ListItem_t xporIdItem;
        portId_t xPid;
}allocPid_t;


pidm_t * pxPidmCreate( void );

portId_t xPidmAllocate(pidm_t *pxInstance);



#endif