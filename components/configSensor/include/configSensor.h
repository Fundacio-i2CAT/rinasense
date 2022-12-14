#ifndef CONFIG_SENSOR_H
#define CONFIG_SENSOR_H

/* Delimiter for Encode name */
#define DELIMITER '/'

/*********  Define BUFFERS Parameters *************/

#define MTU (1500)

/*********   Configure ARP Parameters  ************/

// Length of ARP cache. Define 2 because it is a Point-to-Point.
#define ARP_CACHE_ENTRIES (4)

#define MAX_ARP_AGE (5)
#define MAX_ARP_RETRANSMISSIONS (5)

/*********   Configure IPCP Parameters  ************/

/** @brief Maximum time the IPCP task is allowed to remain in the Blocked state.*/
#define MAX_IPCP_TASK_SLEEP_TIME_US 10000000UL;

/** @brief Time delay between repeated attempts to initialise the network hardware. */
#define INITIALISATION_RETRY_DELAY_SEC 3

#define configMINIMAL_STACK_SIZE (768)

#define IPCP_TASK_PRIORITY (configMAX_PRIORITIES - 2)

/* For ESP32, the "word" is 8 bytes */
#define IPCP_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE * 14)

/*********   Configure EFCP PArameters **************/

#define EFCP_IMAP_ENTRIES (5)

#define MAX_SDU_SIZE (1000)

#define FLOW_DEFAULT_RECEIVE_BLOCK_TIME (100)
#define FLOW_DEFAULT_SEND_BLOCK_TIME (100)

#define INSTANCES_IPCP_ENTRIES (5)

#define MANAGEMENT_AE "Management"

#define FLOWS_REQUEST (10)

#endif
