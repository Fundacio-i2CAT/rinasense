#ifndef _SHIMIPCP_PRIVATE_ETH_H_INCLUDED
#define _SHIMIPCP_PRIVATE_ETH_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

netbuf_t *pxEthAllocHeader(rsrcPoolP_t xEthPool, const gha_t *pxSha, const gha_t *pxTha);

#ifdef __cplusplus
}
#endif

#endif /* _SHIMIPCP_PRIVATE_ETH_H_INCLUDED */
