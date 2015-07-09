#include <stdlib.h>
#include "queue.h"

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
