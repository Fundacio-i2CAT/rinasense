#include <string.h>

#include "freertos_rslist.h"
#include "portability/rslist.h"

void vRsListInit( RsList_t * const pList )
{
    vListInitialise( pList );
}

void vRsListInitItem( RsListItem_t * const pListItem )
{
    vListInitialiseItem( pListItem );
}

void vRsListInsert( RsList_t * const pList, RsListItem_t * const pListItem )
{
    vListInsert( pList, pListItem );
}

void vRsListInsertEnd( RsList_t * const pList, RsListItem_t * const pListItem )
{
    vListInsertEnd( pList, pListItem );
}

void * pRsListGetOwnerOfHeadEntry( RsList_t * const pList )
{
    return listGET_OWNER_OF_HEAD_ENTRY( pList );
}

void vRsListRemoveItem( RsListItem_t * const pListItem )
{
    uxListRemove( pListItem );
}

uint32_t unRsListCurrentListLength( RsList_t * const pList )
{
    return listCURRENT_LIST_LENGTH( pList );
}

bool vRsListIsContainedWithin( RsList_t * const pList, RsListItem_t * const pListItem )
{
    return listIS_CONTAINED_WITHIN( pList, pListItem );
}

void vRsListSetListItemOwner( RsListItem_t * const pList, void * pOwner )
{
}

