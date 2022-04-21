#ifndef CONFIG_SENSOR_H
#define CONFIG_SENSOR_H




/*********** Miscelaneous ***********************/






/*-----------------------------------------------------------*/
/* Utility macros for marking casts as recognized during     */
/* static analysis.                                          */
/*-----------------------------------------------------------*/

	#define offsetof(s,m) (size_t)&(((s *)0)->m

	#define container_of(ptr, type, member)                        \
	({                                                          \
		const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
		(type *)( (char *)__mptr - offsetof(type,member) );        \
	})

 	#define portINLINE    inline

    #define CAST_PTR_TO_TYPE_PTR( TYPE, pointer )                ( vCastPointerTo_ ## TYPE( ( void * ) ( pointer ) ) )
    #define CAST_CONST_PTR_TO_CONST_TYPE_PTR( TYPE, pointer )    ( vCastConstPointerTo_ ## TYPE( ( const void * ) ( pointer ) ) )

/*-----------------------------------------------------------*/
/* Utility macros for declaring cast utility functions in    */
/* order to centralize typecasting for static analysis.      */
/*-----------------------------------------------------------*/
    #define DECL_CAST_PTR_FUNC_FOR_TYPE( TYPE )          TYPE * vCastPointerTo_ ## TYPE( void * pvArgument )
    #define DECL_CAST_CONST_PTR_FUNC_FOR_TYPE( TYPE )    const TYPE * vCastConstPointerTo_ ## TYPE( const void * pvArgument )

	#define ETH_P_RINA      				0xD1F0
	#define ETH_P_ARP						0x4305

/********* Define BLE Parameters ****************/

 	#define SHIM_BLE_MODULE						( 0 )

/*********  Define WiFi Parameters *************/


	#define ESP_MAXIMUM_RETRY  					( 3 )
    /*TAG for Debugging*/
	#define TAG_WIFI							"[NetInterface]"

		//Delimiter for Encode name
	#define DELIMITER 								"/"


/*********  Define BUFFERS Parameters *************/

	#define NUM_NETWORK_BUFFER_DESCRIPTORS 		( 12 )
	#define TAG_NETBUFFER						"[NetBuffer]"

	#define USE_LINKED_RX_MESSAGES				( 0 )
	#define BUFFER_PADDING    					( 0 )
	#define MTU									( 1500 )


/*********   Configure ARP Parameters  ************/

	//Length of ARP cache. Define 2 because it is a Point-to-Point.
	#define ARP_CACHE_ENTRIES 					( 4 )

	#define MAX_ARP_AGE 						( 5 )
	#define MAX_ARP_RETRANSMISSIONS 			( 5 )

	#define MAC_ADDRESS_LENGTH_BYTES 			( 6 )


	/*TAG for Debugging*/
	#define TAG_ARP 							"[ARP]"



/*********   Configure IPCP Parameters  ************/


/* A FreeRTOS queue is used to send events from application tasks to the RINA
 * stack. EVENT_QUEUE_LENGTH sets the maximum number of events that can
 * be queued for processing at any one time.  The event queue must be a minimum of
 * 5 greater than the total number of network buffers. */
    #define EVENT_QUEUE_LENGTH          ( NUM_NETWORK_BUFFER_DESCRIPTORS + 5 )

	/** @brief Maximum time the IPCP task is allowed to remain in the Blocked state.*/
 	#define MAX_IPCP_TASK_SLEEP_TIME    ( pdMS_TO_TICKS( 10000UL ) )

	/** @brief Maximun length of chars for an String_t (Application Name) */
	#define MAX_LENGTH_STRING  					( 255 )

	/** @brief Time delay between repeated attempts to initialise the network hardware. */
 	#define INITIALISATION_RETRY_DELAY    ( pdMS_TO_TICKS( 3000U ) )



	/*TAG for Debugging*/
	#define TAG_IPCPNORMAL 							"[IPCP_NORMAL]"
	#define TAG_IPCPMANAGER 						"[IPCP_MANAGER]"
	#define TAG_IPCPFACTORY							"[IPCP_FACTORY]"


	#define IPCP_TASK_PRIORITY                   ( configMAX_PRIORITIES - 2 )

	#define IPCP_TASK_STACK_SIZE_WORDS           ( configMINIMAL_STACK_SIZE * 5 )



/*********   Configure Shim Parameters  ************/
	/*TAG for Debugging*/
	#define TAG_SHIM 							"[SHIM_WIFI]"

/*********   Configure EFCP PArameters **************/

	#define EFCP_IMAP_ENTRIES     				( 5 )
	#define TAG_EFCP 							"[EFCP]"

	#define MAX_SDU_SIZE						( 1000 )

	#define TAG_RINA 							"[RINA_API]"



	#define FLOW_DEFAULT_RECEIVE_BLOCK_TIME 	portMAX_DELAY
	#define FLOW_DEFAULT_SEND_BLOCK_TIME 		portMAX_DELAY

	#define INSTANCES_IPCP_ENTRIES				( 5 )


	#define TAG_RIB 								"[RIB]"
	#define MANAGEMENT_AE  						"Management"


	#define TAG_ENROLLMENT						"[ENROLLMENT]"

	#define TAG_FA						"[FLOW_ALLOCATOR]"

#endif
