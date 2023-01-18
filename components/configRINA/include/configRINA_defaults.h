/* This *needs* to be set to the size of ethernet packets for now. */
#define MTU (1500)

/* Network configuration for the normal IPC process */
#define CFG_NORMAL_PROCESS_NAME "ue1.mobile.DIF"
#define CFG_NORMAL_PROCESS_INSTANCE "1"
#define CFG_NORMAL_ENTITY_NAME ""
#define CFG_NORMAL_ENTITY_INSTANCE ""

#define CFG_NORMAL_DIF_NAME "mobile.DIF"

#define CFG_MANAGEMENT_AE "Management"

/* Network configuration for the ethernet shim */
#define CFG_SHIM_PROCESS_NAME "wlan0.ue"
#define CFG_SHIM_PROCESS_INSTANCE "1"
#define CFG_SHIM_ENTITY_NAME ""
#define CFG_SHIM_ENTITY_INSTANCE ""

#define CFG_SHIM_DIF_NAME "Irati"

#define CFG_SHIM_INTERFACE "ESP_WIFI_MODE_STA"

/* Network configuration for the local IPC process. */
#define CFG_LOCAL_ADDRESS (1)
#define CFG_LOCAL_ADDRESS_AP_NAME "ue1.mobile.DIF"
#define CFG_LOCAL_ADDRESS_AP_INSTANCE "1"
#define CFG_LOCAL_ADDRESS_AE_NAME ""
#define CFG_LOCAL_ADDRESS_AE_INSTANCE ""

/* Network configuration for the REMOTE IPC process we're going to
 * address with the code here. This has obviously no business being
 * hardcoded in the long run but in some instance, while this is in
 * development, it's obviously easier to fall back to using this than
 * write generic code. */
#define CFG_REMOTE_ADDRESS (3)
#define CFG_REMOTE_ADDRESS_AP_NAME "rs1.mobile.DIF"
#define CFG_REMOTE_ADDRESS_AP_INSTANCE "1"
#define CFG_REMOTE_ADDRESS_AE_NAME ""
#define CFG_REMOTE_ADDRESS_AE_INSTANCE ""

/* QoS cube configuration. Nothing but what is configured here is
 * supported for now. */
#define CFG_QoS_CUBE_NAME "unreliable"
#define CFG_QoS_CUBE_ID (3)
#define CFG_QoS_CUBE_PARTIAL_DELIVERY false
#define CFG_QoS_CUBE_ORDERED_DELIVERY true

/* Name of the network device that we'll use. If LINUX_TAP_CREATE is
 * set to true then this will be created, otherwise we'll assume it
 * was created using the procedure available in the documentation */
#define CFG_LINUX_TAP_DEVICE       "rina00"

/* Build switches */

/* Decides if the TAP NetworkInterface will create the tap device
 * itself. This is probably not a good idea to set this to true for
 * now because the creation code was only lightly tested. */
#undef CFG_LINUX_TAP_CREATE

/* Decides if the TAP NetworkInterface will put UP, or DOWN, the
 * virtual device. It is preferable to leave that to false too but the
 * code is there in case you see any reason for RINA to deal with
 * this. */
#undef CFG_LINUX_TAP_MANAGE

/* Ideally we should be using readv/writev to write netbufs to the
 * network interface but we've had issue with the code doing that so
 * this was left as an option. */
#undef CFG_LINUX_TAP_USE_IOVEC

/* Set this if you want the netbuf structs to be allocated on a memory
 * pool or directly on heap memory. Memory pools make memory problems
 * a bit more difficult to diagnose. */
#undef CFG_NETBUF_USES_POOLS

/* Enable RSRC tracing. This is useful at times but kind of verbose in
 * most case. Defining this enables debugging of RSRC. */
#undef CFG_RSRC_TRACE

/* Enable netbuf tracing. This is kind of verbose too. */
#undef CFG_NETBUF_TRACE

/* Delimiter for encoding RINA names as string */
#define CFG_DELIMITER '/'

/* Number of possible elements in the ARP cache. */
#define CFG_ARP_CACHE_ENTRIES (4)

/* Number of pending flow requests. */
#define CFG_FLOWS_REQUEST (10)

/* Number of EFCP entries we can deal with at once. */
#define CFG_EFCP_IMAP_ENTRIES (5)

/* Maximum size of an SDU */
#define CFG_MAX_SDU_SIZE (1000)

/* This is the default stack size */
#ifndef configMINIMAL_STACK_SIZE
#define configMINIMAL_STACK_SIZE (768)
#endif

/* This is the size of the stack size for the main IPCP process. */
#define CFG_IPCP_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE * 14)

/* Time delay between repeated attempts to initialise the network hardware. */
#define CFG_INITIALISATION_RETRY_DELAY_SEC 3

/* Not currently used. */
#define CFG_FLOW_DEFAULT_RECEIVE_BLOCK_TIME (100)
#define CFG_FLOW_DEFAULT_SEND_BLOCK_TIME (100)

/* MEMORY POOLS */

/* Memory pool config for ethernet headers for ARP packets */
#define CFG_ARP_ETH_POOL_INIT_ALLOC (1)
#define CFG_ARP_ETH_POOL_INCR_ALLOC (1)
#define CFG_ARP_ETH_POOL_MAX_RES (0)

/* Memory pool for ARP packets */
#define CFG_ARP_POOL_MAX_RES (0)

/* Memory pool for the ethernet shim */
#define CFG_SHIM_ETH_POOL_INIT_ALLOC (5)
#define CFG_SHIM_ETH_POOL_INCR_ALLOC (1)
#define CFG_SHIM_ETH_POOL_MAX_RES (0)

/* Memory pool for PCI headers in the ethernet shim */
#define CFG_SHIM_PCI_POOL_INIT_ALLOC (1)
#define CFG_SHIM_PCI_POOL_INCR_ALLOC (1)
#define CFG_SHIM_PCI_POOL_MAX_RES (0)

/* Default memory pool configuration for netbufs */
#define CFG_NETBUF_POOL_INIT_ALLOC (0)
#define CFG_NETBUF_POOL_INCR_ALLOC (1)
#define CFG_NETBUF_POOL_MAX_RES (0)

/* Memory pool for per-thread error information */
#define CFG_ERR_POOL_INIT_ALLOC (1)
#define CFG_ERR_POOL_INCR_ALLOC (1)
#define CFG_ERR_POOL_MAX_RES (0)

/* Memory pool for flow SerDes component */
#define CFG_SD_FLOW_POOL_INIT_ALLOC (1)
#define CFG_SD_FLOW_POOL_INCR_ALLOC (1)
#define CFG_SD_FLOW_POOL_MAX_RES (0)

/* Memory pool for enrollment SerDes component, for encoding */
#define CFG_SD_ENROLLMENT_ENC_POOL_INIT_ALLOC (1)
#define CFG_SD_ENROLLMENT_ENC_POOL_INCR_ALLOC (1)
#define CFG_SD_ENROLLMENT_ENC_POOL_MAX_RES (0)

/* Memory pool for enrollment SerDes component, for decoding */
#define CFG_SD_ENROLLMENT_DEC_POOL_INIT_ALLOC (1)
#define CFG_SD_ENROLLMENT_DEC_POOL_INCR_ALLOC (1)
#define CFG_SD_ENROLLMENT_DEC_POOL_MAX_RES (0)

/* Memory pool for SerDes of CDAP neighbor objects, for encoding */
#define CFG_SD_NEIGHBOR_ENC_POOL_INIT_ALLOC (1)
#define CFG_SD_NEIGHBOR_ENC_POOL_INCR_ALLOC (1)
#define CFG_SD_NEIGHBOR_ENC_POOL_MAX_RES (0)

/* Memory pool for SerDes of CDAP message objects, for encoding */
#define CFG_SD_MESSAGE_ENC_POOL_INIT_ALLOC (1)
#define CFG_SD_MESSAGE_ENC_POOL_INCR_ALLOC (1)
#define CFG_SD_MESSAGE_ENC_POOL_MAX_RES (0)

/* Memory pool for SerDes of CDAP message objects, for decoding */
#define CFG_SD_MESSAGE_DEC_POOL_INIT_ALLOC (1)
#define CFG_SD_MESSAGE_DEC_POOL_INCR_ALLOC (1)
#define CFG_SD_MESSAGE_DEC_POOL_MAX_RES (0)

/* Memory pool for SerDes of CDAP neighbor objects, for decoding */
#define CFG_SD_NEIGHBOR_DEC_POOL_MAX_RES (0)

/* Those 2 constants are used by the ARP cache refresh process but
 * that is untested. */
#define CFG_MAX_ARP_AGE (5)
#define CFG_MAX_ARP_RETRANSMISSIONS (5)

/* Maximum time the IPCP task is allowed to remain in the Blocked state.*/
#define CFG_MAX_IPCP_TASK_SLEEP_TIME_US 10000000UL;

/* Maximum number of IPCP process registered. This is not formally
 * used because we can't dynamically add IPCPs yet. */
#define CFG_INSTANCES_IPCP_ENTRIES (5)

/* Keeping this around for the future but for the moment only those
 * values are supported in the code. */
#define CFG_DTP_POLICY_SET_NAME "default"
#define CFG_DTP_POLICY_SET_VERSION "0"

#define CFG_DTP_INITIAL_A_TIMER (300)
#define CFG_DTP_DTCP_PRESENT false

