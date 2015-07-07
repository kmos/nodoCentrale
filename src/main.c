//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include "stm32f407xx.h"
#include "stm32f4xx_hal.h"

#include <stdio.h>
#include "diag/Trace.h"
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

#include "stm32f4_discovery.h"

#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if_template.h"
#include "usbd_desc.h"

USBD_HandleTypeDef USBD_Device;


/* Hardware and starter kit includes. */


// ----------------------------------------------------------------------------
//
// Standalone STM32F4 empty sample (trace via NONE).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the NONE output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

static xQueueHandle xSwitchQueue;
static void vLedTask( void *pvParameters );
static void vSwitchTask( void *pvParameters );
static void vPrint( void *pvParameters );


int
main(int argc, char* argv[])
{
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
       USBD_Init(&USBD_Device, &VCP_Desc, 0);
       	USBD_RegisterClass(&USBD_Device, &USBD_CDC);
       	USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_Template_fops);
       	USBD_Start(&USBD_Device);
       	HAL_Delay(4000);
       BSP_LED_Init(LED3);
       BSP_LED_Init(LED4);
       BSP_LED_Init(LED5);
       BSP_LED_Init(LED6);
       BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);
   xTaskCreate( vLedTask, /* Pointer to the function that implements the task. */
   	            "vLedTask",/* Text name for the task. For debugging only. */
   	            200,/* Stack depth in words. */
   	            NULL,/* We are not using the task parameter. */
   	            tskIDLE_PRIORITY+1,  /* Task Priority */
   	            NULL /* We are not going to use the task handle. */
   	            );
   xTaskCreate( vSwitchTask, "vSwitchTask", 200, NULL, tskIDLE_PRIORITY, NULL );
   static char printTaskNameA[] = "Task A\0";
   static char printTaskNameB[] = "Task B\0";
   xTaskCreate( vPrint, "vPrint_A", 200, (void*)&printTaskNameA, tskIDLE_PRIORITY, NULL );
   xTaskCreate( vPrint, "vPrint_B", 200, (void*)&printTaskNameB, tskIDLE_PRIORITY, NULL );

   /* Create the Queue for communication between the tasks */
   	xSwitchQueue = xQueueCreate( 5, sizeof(uint8_t) );

   	vTaskStartScheduler();
   	for( ;; );

}

static void vLedTask( void *pvParameters )
{
    uint8_t state;

    for( ;; )
    {
        /* Wait until an element is received from the queue */
        if (xQueueReceive(xSwitchQueue, &state, portMAX_DELAY))
        {
            // On button UP, toggle the LED
            if ( state == 1 )
                        	BSP_LED_Toggle(LED3);
        }
    }
}

static void vSwitchTask( void *pvParameters )
{
    uint8_t newstate,state = 0;

    for( ;; )
    {
        if ( BSP_PB_GetState(BUTTON_KEY) != state )
        {
            // debounce and read again
            vTaskDelay( 10 );

            newstate = BSP_PB_GetState(BUTTON_KEY)==1;
            if ( newstate != state )
            {
                state = newstate;
                //Coda, Item, TicksToWait
                if(newstate == 1) xQueueSend(xSwitchQueue, &newstate, 0);
                printf("Button Pressed!\n\r");
            }
        }
        // read again soon
        vTaskDelay( 20 );
    }
}

void vPrint( void *pvParameters ){
	char* taskName = (char*)(pvParameters);
	for(;;){
		printf("%s prints this\n\r", taskName );
		vTaskDelay(2000);
	}
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
/*-----------------------------------------------------------*/

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
