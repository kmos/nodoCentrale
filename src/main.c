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
#include "ringBuffer.h"

// USB handle
USBD_HandleTypeDef USBD_Device;

void setupBSP(void){
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

void onCanJoinReply(NodeIDType id, SecretKeyType key, uint16_t address);

void exampleOnCanJoinReply(NodeIDType nodeID, SecretKeyType key, uint16_t nodeAddress) {
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
    NodeMessage toSend;
    if (RingBufferGet((uint8_t*)&toSend, sizeof(NodeMessage)) != 0) {
      break;
    }

    while (VCP_write((uint8_t*)&toSend, toSend.length) != toSend.length);
  }

  uint8_t opcode;
  if (VCP_read(&opcode, 1) != 1) {
    return;
  }

  switch(opcode) {
    case CONFIGSENSOR: {
      ConfigSensorType configSensorPacket;
      while (VCP_read((uint8_t*)&configSensorPacket, CONFSENS_DIM) != CONFSENS_DIM);

      sendConfigSensor(&configSensorPacket);

      break;
    }

    case CANJOINREPLY: {
      CanJoinReplyPacketType canJoinReplyPacket;
      while (VCP_read((uint8_t*)(&canJoinReplyPacket), CANJOINREPLY_DIM) != CANJOINREPLY_DIM);

      callCanJoinCallback(canJoinReplyPacket.nodeID, canJoinReplyPacket.secretKey, canJoinReplyPacket.nodeAddress);

      break;
    }

    case READDATA: {
      ReadDataPacketType readDataPacket;
      while (VCP_read((uint8_t*)&readDataPacket, READDATA_DIM) != READDATA_DIM);

      sendReadData(&readDataPacket);

      break;
    }
  }
}

int main(int argc, char* argv[]) {
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
  setupUSB();
  HAL_Delay(4000);
  setupBSP();

  setCanJoinCallback(exampleOnCanJoinReply);

  while (1) {
    receiveFromCenter();
  }
}
