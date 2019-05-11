#include <Arduino.h>
#include <SoftwareSerial.h>

#include "StopDriver.h"
#include "Types.h"

extern SoftwareSerial* debugSerial;

StopDriver::StopDriver(int _iStrobePortBit, int _iClockPortBit, int _iDataPortBit, byte _iSkipUpperBits) {
  m_iStrobePortBit = _iStrobePortBit;
  m_iClockPortBit = _iClockPortBit;
  m_iDataPortBit = _iDataPortBit;
  m_iSkipUpperBits = _iSkipUpperBits;

  pinMode(m_iStrobePortBit, OUTPUT);
  pinMode(m_iClockPortBit, OUTPUT);
  pinMode(m_iDataPortBit, OUTPUT);

  // test when optimization is active
/*
  if (calcActivation(1, 1)!=0) debugSerial->println("test failed for ACTIVATION SKIP");
  if (calcActivation(1, 0)!=1) debugSerial->println("test failed for NEW ACTIVATION");
  if (calcActivation(0, 1)!=0) debugSerial->println("test failed for DEACTIVATION 1");
  if (calcActivation(0, 0)!=0) debugSerial->println("test failed for DEACTIVATION 2");
  
  if (calcDeactivation(0, 0)!=0) debugSerial->println("test failed for DEACTIVATION SKIP");
  if (calcDeactivation(0, 1)!=1) debugSerial->println("test failed for NEW DEACTIVATION");
  if (calcDeactivation(1, 0)!=0) debugSerial->println("test failed for DEACTIVATION 3");
  if (calcDeactivation(1, 1)!=0) debugSerial->println("test failed for DEACTIVATION 4");

  debugSerial->println("test completed!");
  */
}


void StopDriver::clockOutput()
{
    digitalWrite(m_iClockPortBit, LOW); 
    // probably not needed; works without this delay. 
    // this is provided for the one having the 6' long cable.
    delayMicroseconds(1);
    // clock-it-in
    digitalWrite(m_iClockPortBit, HIGH);     
}


void StopDriver::sendDataEx(unsigned long iDivStops, unsigned long iDivStopState, byte iSkipUpperBits) {
//  Serial.println(iDivStops);

  // always load MSB...LSB
  const unsigned long OUTPUT_MASK = 0x80000000;

  unsigned long iActivate = iDivStops;
  unsigned long iDeactivate = iDivStops ^ 0xffffffff;

  int iBitsAlreadyShiftedOut = 0;

  // highest bits go first
  for (int i=MAX_DIV_STOPS - 1; i>=0; i--) 
  {
    if (iBitsAlreadyShiftedOut >= m_iSkipUpperBits) {
      // possibly deactivate stop; deactivate bit loaded first
      if (iDeactivate & OUTPUT_MASK) 
        // de-energise
        digitalWrite(m_iDataPortBit, HIGH); 
        else
        // energise
        digitalWrite(m_iDataPortBit, LOW); 
  
      clockOutput();
  
      // possibly activate stop; activate bit loaded last
      if (iActivate & OUTPUT_MASK) 
        // energise
        digitalWrite(m_iDataPortBit, HIGH); 
        else 
        // de-energise
        digitalWrite(m_iDataPortBit, LOW); 
      
      clockOutput();
    }
   
    // always load MSB...LSB
    iActivate=iActivate << 1;
    iDeactivate=iDeactivate << 1;
  
    iBitsAlreadyShiftedOut++;
  }

  //debugSerial->println();
//  Serial.println("end");
}


void StopDriver::setAllOff() {
  // lock output
  digitalWrite(m_iStrobePortBit, LOW); 

  for (int i=MAX_DIV_STOPS * 2 - 1; i>=0; i--) 
  {
    // turn off deactivation; deactivate bit loaded first
    digitalWrite(m_iDataPortBit, LOW); 

    clockOutput();

    // turn off activation; activate bit loaded last
    digitalWrite(m_iDataPortBit, LOW); 

    clockOutput();
  }

  // unlock output
  digitalWrite(m_iStrobePortBit, HIGH); 
}


void StopDriver::testSetAllActive() {
  // lock output
  digitalWrite(m_iStrobePortBit, LOW); 

  for (int i=7 * 12; i>=0; i--) 
  {
    // turn off deactivation; deactivate bit loaded first
    digitalWrite(m_iDataPortBit, LOW); 

    clockOutput();

    // turn off activation; activate bit loaded last
    digitalWrite(m_iDataPortBit, HIGH); 

    clockOutput();
  }

  // unlock output
  digitalWrite(m_iStrobePortBit, HIGH); 
}


void StopDriver::testSetAllInactive() {
  // lock output
  digitalWrite(m_iStrobePortBit, LOW); 

  for (int i=7 * 12; i>=0; i--) 
  {
    // turn off deactivation; deactivate bit loaded first
    digitalWrite(m_iDataPortBit, HIGH); 

    clockOutput();

    // turn off activation; activate bit loaded last
    digitalWrite(m_iDataPortBit, LOW); 

    clockOutput();
  }

  // unlock output
  digitalWrite(m_iStrobePortBit, HIGH); 
}


void StopDriver::send(unsigned long iDiv1Stops, unsigned long iDiv1State, unsigned long iDiv2Stops, unsigned long iDiv2State) {
  // lock output
  digitalWrite(m_iStrobePortBit, LOW); 

  sendDataEx(iDiv2Stops, iDiv2State, 0);
  sendDataEx(iDiv1Stops, iDiv1State, m_iSkipUpperBits);
  
  // unlock output
  digitalWrite(m_iStrobePortBit, HIGH); 
}


