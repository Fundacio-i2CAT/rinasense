#ifndef CONFIG_SENSOR_H
#define CONFIG_SENSOR_H


/*********  Define WiFi Parameters *************/

	#define ESP_WIFI_SSID      					"WS02"
	#define ESP_WIFI_PASS      					"Esdla2025"
	#define ESP_MAXIMUM_RETRY  					( 3 )
    /*TAG for Debugging*/
	#define TAG_WIFI							"NetInterface"


/*********  Define BUFFERS Parameters *************/

	#define NUM_NETWORK_BUFFER_DESCRIPTORS 		( 6 )


/*********   Configure ARP Parameters  ************/

	//Length of ARP cache. Define 2 because it is a Point-to-Point.
	#define ARP_CACHE_ENTRIES 					( 2 )

	#define MAX_ARP_AGE 						( 5 )
	#define MAX_ARP_RETRANSMISSIONS 			( 5 )

	#define MAC_ADDRESS_LENGTH_BYTES 			( 6 )

	/*TAG for Debugging*/
	#define TAG_ARP 							"ARP"



/*********   Configure IPCP Parameters  ************/

	#define IPCP_ADDRESS_LENGTH_BYTES 			( 4 )

	/** @brief Maximum time the IPCP task is allowed to remain in the Blocked state.*/
 	#define MAX_IPCP_TASK_SLEEP_TIME    ( pdMS_TO_TICKS( 10000UL ) )

	/** @brief Maximun length of chars for an String_t (Application Name) */
	#define MAX_LENGTH_STRING  					( 255 )

	/*TAG for Debugging*/
	#define TAG_ICP 							"IPCP"


#endif
