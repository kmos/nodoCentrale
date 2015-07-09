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
