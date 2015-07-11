#ifndef INCLUDE_RINGBUFFER_H_
#define INCLUDE_RINGBUFFER_H_

#include <stdint.h>

#define RING_SIZE 64
#define MAX_MSG_SIZE 64

int RingBufferPut(uint8_t* data, uint8_t dataLen);
int RingBufferGet(uint8_t* data, uint8_t dataLen);

#endif
