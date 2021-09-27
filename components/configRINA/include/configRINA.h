#ifndef CONFIG_RINA_H
#define CONFIG_RINA_H

/************* SHIM WIFI CONFIGURATION ***********/
	#define SHIM_PROCESS_NAME     					"wlan0"
	#define SHIM_PROCESS_INSTANCE      				"1"
	#define SHIM_ENTITY_NAME     					""
	#define SHIM_ENTITY_INSTANCE      				""

	#define SHIM_DAF_PROCESS_NAME     				"wlan0"
	#define SHIM_DAF_PROCESS_INSTANCE      			"1"
	#define SHIM_DAF_ENTITY_NAME     				""
	#define SHIM_DAF_ENTITY_INSTANCE      			""

	#define SHIM_DIF_NAME  							"Irati"

	#define SHIM_INTERFACE							"ESP_WIFI_MODE_STA"

	#define SIZE_SDU_QUEUE							(  200  )

	//Delimiter for Encode name
	#define DELIMITER 								"/"

#endif
