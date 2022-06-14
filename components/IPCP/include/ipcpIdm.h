
#ifndef IPCPIDM_H__INCLUDED
#define IPCPIDM_H__INCLUDED

typedef struct xIPCIDM
{
        List_t xAllocatedIpcpIds;
        ipcProcessId_t xLastAllocated;
} ipcpIdm_t;

typedef struct xALLOC_IPCPID
{
        ListItem_t xIpcpIdItem;
        ipcProcessId_t xIpcpId;
} allocIpcpId_t;

ipcpIdm_t *pxIpcpIdmCreate(void);

ipcProcessId_t xIpcpIdmAllocate(ipcpIdm_t *pxInstance);

#endif