#ifndef CONFIG_RINA_H
#define CONFIG_RINA_H

/************* SHIM WIFI CONFIGURATION ***********/
	#define SHIM_WIFI_MODULE					( 1 ) //Zero if not shim WiFi modules is required.

	#define SHIM_PROCESS_NAME     					"wlan0.ue"
	#define SHIM_PROCESS_INSTANCE      				"1"
	#define SHIM_ENTITY_NAME     					""
	#define SHIM_ENTITY_INSTANCE      				""

	#define SHIM_DIF_NAME  							"Irati"

	#define SHIM_INTERFACE							"ESP_WIFI_MODE_STA"

	#define SIZE_SDU_QUEUE							(  200  )

	/************ SHIM DIF CONFIGURATION **************/
	#define ESP_WIFI_SSID      					"irati"//"WS02"
	#define ESP_WIFI_PASS      					"irati2017"//"Esdla2025"

/*********** NORMAL CONFIGURATION ****************/

	#define NORMAL_PROCESS_NAME     					"ue1.mobile"
	#define NORMAL_PROCESS_INSTANCE      				"1"
	#define NORMAL_ENTITY_NAME     						""
	#define NORMAL_ENTITY_INSTANCE      				""


	#define NORMAL_DIF_NAME  							"mobile.DIF"

	/*********** NORMAL IPCP CONFIGURATION ****************/
	/**** Known IPCProcess Address *****/
	#define LOCAL_ADDRESS							( 1 )
	#define LOCAL_ADDRESS_AP_INSTANCE				"1"
	#define LOCAL_ADDRESS_AP_NAME					"ue1.mobile"

	#define REMOTE_ADDRESS							( 3 )
	#define REMOTE_ADDRESS_AP_INSTANCE				"1"
	#define REMOTE_ADDRESS_AP_NAME					"ar1.mobile"



#endif
