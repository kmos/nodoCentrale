#include <stdlib.h>

#include "queue.h"
#include "applayer.h"

void (*canJoinCallback)(NodeIDType, SecretKeyType, uint16_t) = NULL;

void setCanJoinCallback(void (*callback)(NodeIDType, SecretKeyType, uint16_t)) {
  canJoinCallback = callback;
}

void callCanJoinCallback(NodeIDType nodeID, SecretKeyType secretKey, uint16_t nodeAddress) {
  canJoinCallback(nodeID, secretKey, nodeAddress);
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

// XXX: Ricezione di un pacchetto validato dal livello sicurezza.
void securityLevelCallback(uint8_t* data, uint16_t address) {
  NetMessage* payload = (NetMessage*)data;

  NodeMessage* msg = (NodeMessage*)malloc(sizeof(NodeMessage));
  msg->code = DATA;
  msg->Tpack.dataPacket.nodeAddress = address;
  msg->Tpack.dataPacket.sensorID = payload->sensorID;
  msg->Tpack.dataPacket.timestamp = 0; /* Da dove prendiamo il timestamp? */
  msg->Tpack.dataPacket.value = payload->value;
  msg->Tpack.dataPacket.alarm = payload->alarm;
  msg->length = DATA_DIM;

  MessageQueuePush(msg);
}

void sendConfigSensor(ConfigSensorType* packet) {
  NetMessage* payload = (NetMessage*)malloc(sizeof(NetMessage));
  payload->opCode = CONFIGSENSOR;
  payload->sensorID = packet->sensorID;
  payload->alarm = packet->alarm;
  payload->highThreshold = packet->highThreshold;
  payload->lowThreshold = packet->lowThreshold;
  payload->period = packet->period;
  payload->priority = packet->priority;

  // XXX: Invio di un pachetto tramite il livello sicurezza.
  //securityLevelSend(packet->nodeAddress, payload);
}

void sendReadData(ReadDataPacketType* packet) {
  NetMessage* payload = (NetMessage*)malloc(sizeof(NetMessage));
  payload->opCode = READDATA;
  payload->sensorID = packet->sensorID;
  payload->period = 0;

  // XXX: Invio di un pachetto tramite il livello sicurezza.
  //securityLevelSend(packet->nodeAddress, payload);

  // XXX: Rimuovere questa simulazione di ricezione pacchetto dalla rete!
  NetMessage* fakePayload = (NetMessage*)malloc(sizeof(NetMessage));
  fakePayload->sensorID = 0;
  fakePayload->value = 7;
  fakePayload->alarm = 0;
  securityLevelCallback((uint8_t*)fakePayload, 1);
  free(fakePayload);
}
