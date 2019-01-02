#include <Arduino.h>
#include <SoftwareSerial.h>

#include "StopDriver.h"
#include "Types.h"

extern SoftwareSerial* debugSerial;

#define STOP_DRIVE_TIME_MS  200

StopDriver::StopDriver(int _iStrobePortBit, int _iClockPortBit, int _iDataPortBit, byte _iSkipUpperBits) {
  iStrobePortBit = _iStrobePortBit;
  iClockPortBit = _iClockPortBit;
  iDataPortBit = _iDataPortBit;
  iSkipUpperBits = _iSkipUpperBits;

  pinMode(iStrobePortBit, OUTPUT);
  pinMode(iClockPortBit, OUTPUT);
  pinMode(iDataPortBit, OUTPUT);

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
    delayMicroseconds(2);
    // clock-it-in
    digitalWrite(iClockPortBit, LOW); 
    delayMicroseconds(2);
    digitalWrite(iClockPortBit, HIGH);     
    delayMicroseconds(2);
}


unsigned long StopDriver::calcActivation(unsigned long iDivStops, unsigned long iDivStopState) {
    // don't bother activating stops that don't require activation
    return iDivStops;  // optimize:  & (iDivStops ^ iDivStopState);
}


unsigned long StopDriver::calcDeactivation(unsigned long iDivStops, unsigned long iDivStopState) {
    // don't bother deactivating stops that don't require deactivation
    return calcActivation(iDivStops ^ (unsigned long) 0xffffffff, iDivStopState ^ (unsigned long) 0xffffffff);
}


void StopDriver::sendDataEx(unsigned long iDivStops, unsigned long iDivStopState, byte iSkipUpperBits) {
//  Serial.println(iDivStops);

  // always load MSB...LSB
  const unsigned long MASK = 0x80000000;

debugSerial->print("INPUT ");
debugSerial->print(iDivStops);

  unsigned long iActivate = calcActivation(iDivStops, iDivStopState);
  unsigned long iDeactivate = calcDeactivation(iDivStops, iDivStopState) & 3;

debugSerial->print("   ACT ");
debugSerial->print(iActivate);
debugSerial->print("   DEACT ");
debugSerial->println(iDeactivate);

  if (iActivate & iDeactivate > 0) {
      Serial.println("OH NO!  WE'RE TRYING TO ACTIVATE AND DEACTIVATE AT THE SAME TIME!  Turning off deactivation to avoid serious problems.");
      iDeactivate = 0;
  }

  int iBitsAlreadySent = 0;

  // highest bits go first
  for (int i=MAX_DIV_STOPS - 1; i>=0; i--) 
  {
//    Serial.print(result);
//    Serial.println(iDivStops);

    iBitsAlreadySent++;
    
    if (1) {  //(iBitsAlreadySent > iSkipUpperBits) {
      // possibly deactivate stop; deactivate bit loaded first
      if (iDeactivate & MASK) 
        // de-energise
        digitalWrite(iDataPortBit, HIGH); 
        else
        // energise
        digitalWrite(iDataPortBit, LOW); 
  
      clockOutput();
  
      // possibly activate stop; activate bit loaded last
      if (iActivate & MASK) 
        // energise
        digitalWrite(iDataPortBit, HIGH); 
        else
        // de-energise
        digitalWrite(iDataPortBit, LOW); 
  
      clockOutput();
    }
  
    // always load MSB...LSB
    iActivate=iActivate << 1;
    iDeactivate=iDeactivate << 1;
  }

//  Serial.println();
//  Serial.println("end");
}


void StopDriver::setAllOff() {
  // lock output
  digitalWrite(iStrobePortBit, LOW); 

  for (int i=MAX_DIV_STOPS * 2 - 1; i>=0; i--) 
  {
    // turn off deactivation; deactivate bit loaded first
    digitalWrite(iDataPortBit, LOW); 

    clockOutput();

    // turn off activation; activate bit loaded last
    digitalWrite(iDataPortBit, LOW); 

    clockOutput();
  }

  // unlock output
  digitalWrite(iStrobePortBit, HIGH); 
}


void StopDriver::testSetAllActive() {
  // lock output
  digitalWrite(iStrobePortBit, LOW); 

  for (int i=7 * 12; i>=0; i--) 
  {
    // turn off deactivation; deactivate bit loaded first
    digitalWrite(iDataPortBit, LOW); 

    clockOutput();

    // turn off activation; activate bit loaded last
    digitalWrite(iDataPortBit, HIGH); 

    clockOutput();
  }

  // unlock output
  digitalWrite(iStrobePortBit, HIGH); 
}


void StopDriver::testSetAllInactive() {
  // lock output
  digitalWrite(iStrobePortBit, LOW); 

  for (int i=7 * 12; i>=0; i--) 
  {
    // turn off deactivation; deactivate bit loaded first
    digitalWrite(iDataPortBit, HIGH); 

    clockOutput();

    // turn off activation; activate bit loaded last
    digitalWrite(iDataPortBit, LOW); 

    clockOutput();
  }

  // unlock output
  digitalWrite(iStrobePortBit, HIGH); 
}


void StopDriver::send(unsigned long iDiv1Stops, unsigned long iDiv1State, unsigned long iDiv2Stops, unsigned long iDiv2State) {
  // lock output
  digitalWrite(iStrobePortBit, LOW); 

  sendDataEx(iDiv2Stops, iDiv2State, 0);
  sendDataEx(iDiv1Stops, iDiv1State, iSkipUpperBits);
  
  // unlock output
  digitalWrite(iStrobePortBit, HIGH); 

  delay(STOP_DRIVE_TIME_MS);

  setAllOff();
}


