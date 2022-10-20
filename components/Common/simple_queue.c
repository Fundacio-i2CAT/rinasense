#include "common/simple_queue.h"
#include "common/list.h"
#include "common/rsrc.h"
#include "linux_rsmem.h"

struct queueItem {
    RsListItem_t xListItem;
    void *pxItem;
};

bool_t xSimpleQueueInit(const char *sQueueName, RsSimpleQueue_t *pxQueue)
{
    vRsListInit(&pxQueue->xQueueList);
    pxQueue->xPool = pxRsrcNewPool(sQueueName, sizeof(struct queueItem), 5, 5, 0);

    if (!pxQueue->xPool)
        return false;

    return true;
}

void vSimpleQueueFini(RsSimpleQueue_t *pxQueue)
{

}

bool_t xSimpleQueueSendToBack(RsSimpleQueue_t *pxQueue, void *pxItem)
{
    struct queueItem *pxQueueItem;

    pxQueueItem = pxRsrcAlloc(pxQueue->xPool, __FUNCTION__);
    if (!pxQueueItem)
        return false;

    pxQueueItem->pxItem = pxItem;
    vRsListInitItem(&pxQueueItem->xListItem, pxQueueItem);
    vRsListInsertEnd(&pxQueue->xQueueList, &pxQueueItem->xListItem);

    return true;
}

void *pvSimpleQueueReceive(RsSimpleQueue_t *pxQueue)
{
    RsListItem_t *pxListItem;
    struct queueItem *pxQueueItem;
    void *pxItem;

    pxListItem = pxRsListGetFirst(&pxQueue->xQueueList);

    if (!pxListItem)
        return NULL;

    pxQueueItem = (struct queueItem *)pxListItem->pvOwner;
    pxItem = pxQueueItem->pxItem;

    vRsListRemove(pxListItem);

    vRsrcFree(pxQueueItem);

    return pxItem;
}
