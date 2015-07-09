#ifndef INCLUDE_QUEUE_H_
#define INCLUDE_QUEUE_H_

#include "applayer.h"

typedef struct MessageQueueElem {
  NodeMessage* msg;
  struct MessageQueueElem* next;
} MessageQueueElem_t;

void MessageQueuePush(NodeMessage* msg);
NodeMessage* MessageQueuePop();

#endif
