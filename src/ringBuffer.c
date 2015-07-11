#include "ringBuffer.h"

#include <stdlib.h>

// ringPos_t's size should be set to a size that allows
// your architecture to read/modify it in an atomic way (one instruction).
typedef uint16_t ringPos_t;

volatile ringPos_t ringHead;
volatile ringPos_t ringTail;
uint8_t ringData[RING_SIZE][MAX_MSG_SIZE];

int RingBufferPut(uint8_t* data, uint8_t dataLen) {
  ringPos_t nextHead = (ringHead + 1) % RING_SIZE;

  if (nextHead == ringTail) {
    return -1;
  }

  memcpy(ringData[ringHead], data, dataLen);

  ringHead = nextHead;

  return 0;
}
 
int RingBufferGet(uint8_t* data, uint8_t dataLen) {
  if (ringHead == ringTail) {
    return -1;
  }

  memcpy(data, ringData[ringTail], dataLen);

  ringTail = (ringTail + 1) % RING_SIZE;

  return 0;
}
