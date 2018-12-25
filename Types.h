#ifndef __TYPES_H__
#define __TYPES_H__

#include <SoftwareSerial.h>

#define MAX_DIV_STOPS 32

#define DEBUG_RX_PORT 50
#define DEBUG_TX_PORT 51

SoftwareSerial SoftSerial(DEBUG_RX_PORT, DEBUG_TX_PORT);

#endif // __TYPES_H__
