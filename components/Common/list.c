#include <stdint.h>
#include "common/list.h"
#include "common/private/iot_doubly_linked_list.h"

bool_t xRsListIsContainedWithin(RsList_t *const pxList, RsListItem_t *const pxItem)
{
    RsListItem_t *pItem;
    void *pvOwner;

    pItem = pxRsListGetFirst(pxList);

    while (pItem != NULL) {
        if (pItem == pxItem)
            return true;
        pItem = pxRsListGetNext(pItem);
    }

    return false;
}

