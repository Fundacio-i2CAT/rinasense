/*
 * du.h
 *
 *  Created on: 30 sept. 2021
 *      Author: i2CAT
 */

#ifndef COMPONENTS_RMT_INCLUDE_DU_H_
#define COMPONENTS_RMT_INCLUDE_DU_H_

#include "portability/port.h"
#include "common/rsrc.h"
#include "common/netbuf.h"

#include "pci.h"
#include "rina_common_port.h"

#ifdef __cplusplus
extern "C" {
#endif

/* FIXME: THE PCI SHOULD NOT HAVE A FIXED SIZE */

/* typedef struct { */
/*     netbuf_t *pxNb; */
/* } du_t; */

typedef netbuf_t du_t;

static inline void vDuDestroy(du_t *pxDu)
{
    vNetBufFreeAll(pxDu);
}

/* Split the header from the data */
bool_t xDuDecap(size_t unSz, du_t *pxDu);

/* Prepend a header netbuf */
du_t *xDuEncap(void *pvPci, size_t unSz, du_t *pxDu);

bool_t xDuIsOk(const du_t *pxDu);

#ifdef __cplusplus
}
#endif

#endif /* COMPONENTS_RMT_INCLUDE_DU_H_ */
