
#ifndef CEPIDM_H__INCLUDED
#define CEPIDM_H__INCLUDED

typedef struct xCEPIDM {
	List_t xAllocatedCepIds;
	cepId_t xLastAllocated;
}cepIdm_t;

typedef struct xALLOC_CEPID {
        ListItem_t xCepIdItem;
        cepId_t xCepId;
}allocCepId_t;


cepIdm_t * pxCepIdmCreate( void );

cepId_t xCepIdmAllocate(cepIdm_t *pxInstance);



#endif