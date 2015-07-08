// ----------------------------------------------------------------------------



#include "stm32f407xx.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include "diag/Trace.h"
#include "stm32f4_discovery.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if_template.h"
#include "usbd_desc.h"
#include "applayer.h"

#ifdef FREERTOS_ON
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
/* tasks */
static xQueueHandle xSwitchQueue;
static void vRxCenterTxNetTask( void *pvParameters);
static void vRxNetTxCenterTask( void *pvParameters);
#else
/* functions */
static void reciveFromCenter();
static void reciveFromNet();
#endif

//usb handler
USBD_HandleTypeDef USBD_Device;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"


//Setup Hardware
void setupUSB(void);
void setupBSD(void);

//var
uint8_t opcode;

int
main(int argc, char* argv[])
{
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    setupUSB();
    HAL_Delay(4000);
    setupBSD();

#ifdef FREERTOS_ON
/***************Inizio parte con freeRTOS**************/

    xTaskCreate( vRxCenterTxNetTask, /* Pointer to the function that implements the task. */
   	            "RicezioneCentroControllo",/* Text name for the task. For debugging only. */
   	            200,/* Stack depth in words. */
   	            NULL,/* We are not using the task parameter. */
   	            tskIDLE_PRIORITY+1,  /* Task Priority */
   	            NULL /* We are not going to use the task handle. */
   	            );
    xTaskCreate( vRxNetTxCenterTask, "RicezioneNetwork", 200, NULL, tskIDLE_PRIORITY, NULL );
   /* Create the Queue for communication between the tasks */
   //	xSwitchQueue = xQueueCreate( 5, sizeof(uint8_t) );

   	vTaskStartScheduler();
   	for( ;; );
#else
/***************Inizio parte senza freeRTOS***********/
   	while(1){

   		reciveFromCenter();
//*

   	}


#endif
}


void setupBSD(void){
	BSP_LED_Init(LED3);
    BSP_LED_Init(LED4);
    BSP_LED_Init(LED5);
    BSP_LED_Init(LED6);
    //BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);
}

void setupUSB(){
	USBD_Init(&USBD_Device, &VCP_Desc, 0);
	USBD_RegisterClass(&USBD_Device, &USBD_CDC);
	USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_Template_fops);
	USBD_Start(&USBD_Device);
}




#ifdef FREERTOS_ON
static void vRxCenterTxNetTask( void *pvParameters){

	for( ;; ){
		if(VCP_read((uint8_t*)&lunghezza, 4)!=0){

			VCP_write((uint8_t*)&lunghezza, 4);
	//	receiving_buffer = (uint8_t*) malloc(length[0]);
	//	if(VCP_read(receiving_buffer,length[0])!=0){
	//		VCP_write(receiving_buffer, length[0]);
	//	}
		//VCP_write(receiving_buffer, 5);
		}
	}
}

static void vRxNetTxCenterTask( void *pvParameters){

}

void vApplicationMallocFailedHook( void )
{
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{

}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	taskDISABLE_INTERRUPTS();
	for( ;; );
}
#else



static void reciveFromCenter(){

	NodeMessage message;
	NetPackage netmessage;

	if(VCP_read(&opcode, 1)!=0){
		switch(opcode){
		case CONFIGSENSOR:
			//invia configurazione al sensore: CC -> nodo centrale -> nodo sensore
			if(VCP_read(&message, CONFSENSDIM)!=0){
				netmessage.code = message.code;
				netmessage.payload.id = message.Tpack.configSensor.sensorID;
				netmessage.payload.period=message.Tpack.configSensor.period;
				netmessage.payload.lt=message.Tpack.configSensor.lowThreshold;
				netmessage.payload.ht=message.Tpack.configSensor.highThreshold;
				netmessage.payload.priority=message.Tpack.configSensor.priority;
				netmessage.payload.alarm = message.Tpack.configSensor.alarm;
			/*AGGIUNTA SICUREZZA */

			/*INVIO RETE */
			//SEND_MESSAGE
			}
			break;
		case REPLYJOIN:
			//risposta alla join:CC -> nodo centrale
			if(VCP_read(&message, JOINREPLYDIM)!=0){
				netmessage.code = message.code;
				//i primi 64bit di una chiave tutti uguale a zero...IMPOSSIBILE
				if(message.Tpack.canJoinReplyPacket.secretKey.sk0 == 0){
					//Risposta alla join -NEGATIVA
				}
				else{
					//Risposta alla join - POSITIVA
				}
			}
			break;
		case READDATA:
			//invia richiesta di lettura: CC -> nodo centrale -> nodo sensore
			if(VCP_read((uint8_t*)&message, 4)!=0){
				netmessage.code = message.code;
				netmessage.payload.id = message.Tpack.readDataPacket.sensorID;
				/*AGGIUNTA SICUREZZA*/

				/*INVIO RETE */
				//SEND_MESSAGE(message->readDataPacket->nodeAddress,netmessage,...);
			}
			break;
		};

	}
}

//callback di livello rete almeno così mi hanno detto x test si può usare uart
static void reciveFromNet(){
	struct nodeMessage message;
	struct netPackage netmessage;

	//ricezione dato dalla net
	//

}
#endif

/*-----------------------------------------------------------*/

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
