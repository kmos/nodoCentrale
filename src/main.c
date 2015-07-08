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

//

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
	//struct NodeMessage message* = malloc(sizeof(struct NodeMessage));

	struct NodeMessage *message;
	message = (NodeMessage *)malloc(sizeof(NodeMessage));

	if(VCP_read(&message, DIMPACK)!=0){
		switch(message->code){
		case READDATA:
			//invia richiesta di lettura
			break;
		case CONFIGSENSOR:
			//invia configurazione al sensore
			break;
		case REPLYJOIN:
			//risposta alla join

			break;
		};

	}

}

//callback di livello rete
static void reciveFromNet(){

}
#endif

/*-----------------------------------------------------------*/

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
