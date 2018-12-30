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

void flashHeartbeatLED() {  
  digitalWrite(HEART_BEAT_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);                       // wait for a second
  digitalWrite(HEART_BEAT_LED, LOW);    // turn the LED off by making the voltage LOW
  delay(100);                       // wait for a second
}
