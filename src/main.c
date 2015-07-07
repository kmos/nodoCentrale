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
/* functions */
static xQueueHandle xSwitchQueue;
static void vRxCenterTxNetTask( void *pvParameters);
static void vRxNetTxCenterTask( void *pvParameters);
#endif



USBD_HandleTypeDef USBD_Device;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

//Setup Hardware
void setupUSB(void);
void setupBSD(void);

//var
uint32_t lunghezza;

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
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
#endif

/*-----------------------------------------------------------*/

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
