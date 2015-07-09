// ----------------------------------------------------------------------------
#define TESTING
//#define FREERTOS_ON

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

void setCanJoinCallback(void (*callback)(NodeIDType, SecretKeyType, uint16_t nodeAddress));
void exampleCanJoinCallback(NodeIDType nodeID, SecretKeyType key, uint16_t nodeAddress);

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

int main(int argc, char* argv[]) {
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

  setCanJoinCallback(exampleCanJoinCallback);

  while (1) {
    reciveFromCenter();
  }
#endif
}

void setupBSD(void){
    BSP_LED_Init(LED3);
    BSP_LED_Init(LED4);
    BSP_LED_Init(LED5);
    BSP_LED_Init(LED6);
    BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
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

typedef struct MessageQueueElem {
  NodeMessage* msg;
  struct MessageQueueElem* next;
} MessageQueueElem_t;

MessageQueueElem_t* messageQueueFirst;
MessageQueueElem_t* messageQueueLast;

void MessageQueuePush(NodeMessage* msg) {
  MessageQueueElem_t* newElem = (MessageQueueElem_t*)malloc(sizeof(MessageQueueElem_t));
  newElem->msg = msg;
  newElem->next = NULL;

  if (messageQueueFirst == NULL || messageQueueLast == NULL) {
    messageQueueFirst = messageQueueLast = newElem;
    return;
  }

  messageQueueLast->next = newElem;
  messageQueueLast = messageQueueLast->next;
}

NodeMessage* MessageQueuePop() {
  if (messageQueueFirst == NULL && messageQueueLast == NULL) {
    return NULL;
  }

  MessageQueueElem_t* oldFirst = messageQueueFirst;
  messageQueueFirst = oldFirst->next;

  if (messageQueueFirst == NULL) {
    messageQueueLast = NULL;
  }

  NodeMessage* msg = oldFirst->msg;
  free(oldFirst);
  return msg;
}

void (*canJoinCallback)(NodeIDType, SecretKeyType, uint16_t) = 0;

void setCanJoinCallback(void (*callback)(NodeIDType, SecretKeyType, uint16_t)) {
  canJoinCallback = callback;
}

void canJoin(NodeIDType nodeID) {
  NodeMessage* msg = (NodeMessage*)malloc(sizeof(NodeMessage));
  msg->code = CANJOIN;
  msg->Tpack.canJoinPacket.nodeID = nodeID;
  msg->length = CANJOIN_DIM;

  MessageQueuePush(msg);
}

void join(NodeIDType nodeID) {
  NodeMessage* msg = (NodeMessage*)malloc(sizeof(NodeMessage));
  msg->code = JOIN;
  msg->Tpack.joinPacket.nodeID = nodeID;
  msg->length = JOIN_DIM;

  MessageQueuePush(msg);
}

void exampleCanJoinCallback(NodeIDType nodeID, SecretKeyType key, uint16_t nodeAddress) {
  BSP_LED_Toggle(LED4);
  join(nodeID);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == KEY_BUTTON_PIN) {
    NodeIDType nodeID;
    nodeID.id0 = 0;
    nodeID.id1 = 0;
    canJoin(nodeID);
  }
}

static void reciveFromCenter( ) {
  NodeMessage message;
  NetPackage netmessage;

#ifdef TESTING
  BSP_LED_Toggle(LED3);
#endif

  while (1) {
    NodeMessage* toSend = MessageQueuePop();
    if (!toSend) {
      break;
    }

    while (VCP_write((uint8_t*)toSend, toSend->length) != toSend->length);

    free(toSend);
  }

  if (VCP_read(&opcode, 1) != 1);

  switch(opcode) {
    case CONFIGSENSOR:
      //invia configurazione al sensore: CC -> nodo centrale -> nodo sensore
      while (VCP_read((uint8_t*)&message, CONFSENS_DIM) != CONFSENS_DIM);

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


      break;

    case CANJOINREPLY:
      //risposta alla join:CC -> nodo centrale
      while (VCP_read((uint8_t*)&message, CANJOINREPLY_DIM) != CANJOINREPLY_DIM);
      netmessage.code = message.code;
      //i primi 64bit di una chiave tutti uguale a zero...IMPOSSIBILE
		//ALL'ARRIVO DELLA REPLY SE I PRIMI 64BIT DELLA KEY SONO UGUALI A 0 VUOL DIRE CHE E' FALLITO
		//IL JOIN, DI CONSEGUENZA IL NODO NON E' PRESENTE NELLA LISTA, VICEVERSA SE E' PRESENTE UNA KEY, ATTRAVERSO
		//QUELLA SI EFFETTUA LA JOIN RISPONDENDO TRAMITE UNA REPLY
#ifdef TESTING
      canJoinCallback(message.Tpack.canJoinReplyPacket.nodeID, message.Tpack.canJoinReplyPacket.secretKey,
                      message.Tpack.canJoinReplyPacket.nodeAddress);
#endif

        if(message.Tpack.canJoinReplyPacket.secretKey.sk0 == 0) {
          //Risposta alla join -NEGATIVA
        } else {
          //Risposta alla join - POSITIVA
        }


      break;

    case READDATA: {
      //invia richiesta di lettura: CC -> nodo centrale -> nodo sensore
      while (VCP_read((uint8_t*)&message, READDATA_DIM) != READDATA_DIM);
      netmessage.code = READDATA;
      netmessage.payload.id = message.Tpack.readDataPacket.sensorID;

#ifdef TESTING
      NodeMessage* reply = (NodeMessage*)malloc(sizeof(NodeMessage));
      reply->code = DATA;
      reply->Tpack.dataPacket.nodeAddress = message.Tpack.readDataPacket.nodeAddress;
      reply->Tpack.dataPacket.sensorID = 0;
      reply->Tpack.dataPacket.timestamp = 666;
      reply->Tpack.dataPacket.value = 7;
      reply->Tpack.dataPacket.alarm = 0;
      reply->length = DATA_DIM;

      MessageQueuePush(reply);
#endif
      /*netmessage.code = message.code;
       * netmessage.payload.id = message.Tpack.readDataPacket.sensorID;*/

      /*AGGIUNTA SICUREZZA*/

      /*INVIO RETE */
      //SEND_MESSAGE(message->readDataPacket->nodeAddress,netmessage,...);
      break;
    }
  }
}


//callback di livello rete almeno così mi hanno detto x test si può usare uart
static void reciveFromNet(){
	NodeMessage message;
	NetPackage netmessage;

	//ricezione dato dalla net
	//

	switch(netmessage.code){
		case JOIN:
			message.code = JOIN;
			//message.Tpack.canJoinPacket.nodeID = FORNITO DAL PACCHETTO LIVELLO SICUREZZA
			//LA LOGICA DI JOIN NON VIENE IMPLEMENTATA DIRETTAMENTE QUI MA BENSI' NELLA REPLY
			//ALL'ARRIVO DELLA REPLY SE I PRIMI 64BIT DELLA KEY SONO UGUALI A 0 VUOL DIRE CHE E' FALLITO
			//IL JOIN, DI CONSEGUENZA IL NODO NON E' PRESENTE NELLA LISTA, VICEVERSA SE E' PRESENTE UNA KEY, ATTRAVERSO
			//QUELLA SI EFFETTUA LA JOIN RISPONDENDO TRAMITE UNA REPLY
			while (VCP_write((uint8_t*)&message, sizeof(CanJoinPacketType)) != sizeof(CanJoinPacketType));
		break;
		case DATA:
			//controllo ******SICUREZZA******
			message.code = DATA;
			message.Tpack.dataPacket.alarm = netmessage.payload.alarm;
			message.Tpack.dataPacket.sensorID = netmessage.payload.id;
			message.Tpack.dataPacket.value = netmessage.payload.val;
			//message.Tpack.dataPacket.nodeAddress = FORNITO DAL PACCHETTO LIVELLO SICUREZZA
			while (VCP_write((uint8_t*)&message, sizeof(DataPacketType)) != sizeof(DataPacketType));
	}
}
#endif

/*-----------------------------------------------------------*/

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
