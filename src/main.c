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

void receiveFromCenter();

void setCanJoinCallback(void (*callback)(NodeIDType, SecretKeyType, uint16_t nodeAddress));
void exampleCanJoinCallback(NodeIDType nodeID, SecretKeyType key, uint16_t nodeAddress);

void onCanJoinReply(NodeIDType id, SecretKeyType key, uint16_t address);

//usb handler
USBD_HandleTypeDef USBD_Device;

//Setup Hardware
void setupUSB(void);
void setupBSD(void);

int main(int argc, char* argv[]) {
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
  setupUSB();
  HAL_Delay(4000);
  setupBSD();

  setCanJoinCallback(exampleCanJoinCallback);

  while (1) {
    receiveFromCenter();
  }
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

typedef struct MessageQueueElem {
  NodeMessage* msg;
  struct MessageQueueElem* next;
} MessageQueueElem_t;

MessageQueueElem_t* messageQueueFirst = NULL;
MessageQueueElem_t* messageQueueLast = NULL;

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
    BSP_LED_Toggle(LED5);
    NodeIDType nodeID;
    memset(&nodeID, 0, sizeof(NodeIDType));
    canJoin(nodeID);
  }
}

void receiveFromCenter() {
  BSP_LED_Toggle(LED3);

  while (1) {
    NodeMessage* toSend = MessageQueuePop();
    if (!toSend) {
      break;
    }

    while (VCP_write((uint8_t*)toSend, toSend->length) != toSend->length);

    free(toSend);
  }

  uint8_t opcode;
  if (VCP_read(&opcode, 1) != 1) {
    return;
  }

  NodeMessage message;

  switch(opcode) {
    case CONFIGSENSOR:
      while (VCP_read((uint8_t*)&message, CONFSENS_DIM) != CONFSENS_DIM);

      // XXX: Invia configurazione al nodo sensore.

      break;

    case CANJOINREPLY: {
      CanJoinReplyPacketType canJoinReplyPacket;
      while (VCP_read((uint8_t*)(&canJoinReplyPacket), CANJOINREPLY_DIM) != CANJOINREPLY_DIM);

      canJoinCallback(canJoinReplyPacket.nodeID, canJoinReplyPacket.secretKey, canJoinReplyPacket.nodeAddress);

      break;
    }

    case READDATA: {
      ReadDataPacketType readDataPacket;
      while (VCP_read((uint8_t*)&readDataPacket, READDATA_DIM) != READDATA_DIM);

      NodeMessage* reply = (NodeMessage*)malloc(sizeof(NodeMessage));
      reply->code = DATA;
      reply->Tpack.dataPacket.nodeAddress = readDataPacket.nodeAddress;
      reply->Tpack.dataPacket.sensorID = 0;
      reply->Tpack.dataPacket.timestamp = 666;
      reply->Tpack.dataPacket.value = 7;
      reply->Tpack.dataPacket.alarm = 0;
      reply->length = DATA_DIM;

      MessageQueuePush(reply);

      // XXX: Invio comando al nodo sensore.

      break;
    }
  }
}
