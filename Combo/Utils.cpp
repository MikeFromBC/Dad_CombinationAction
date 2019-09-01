#include <SoftwareSerial.h>

// provide access to Arduino "defines"
#include <Arduino.h>
  
#include "Utils.h"
#include "Types.h"

extern SoftwareSerial* debugSerial;

void debug_ShowValue(unsigned long stops) {
  for (int i=0; i<MAX_DIV_STOPS; i++) {
    unsigned long iCurrValue = (unsigned long) 1 << i;
	
    if (stops & iCurrValue)
      debugSerial->print(1);
      else
      debugSerial->print(0);              
  }

  debugSerial->println();                
}


// first stop is 0
unsigned long stopValue(int iStopNum) {
  return (unsigned long) 1 << iStopNum;
}


