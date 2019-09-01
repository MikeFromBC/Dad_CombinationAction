#include <Arduino.h>
#include <SoftwareSerial.h>

#include "StopDriver.h"
#include "Types.h"
#include "Utils.h"

extern SoftwareSerial* debugSerial;

StopDriver::StopDriver(byte _iStrobePortBit,
      byte _iClockPortBit, 
      byte _iDataPortBit, 
      byte _iSkipUpperBits, 
      byte iFirstDiv1Coupler, 
      byte iFirstDiv2Coupler) {
  m_iStrobePortBit = _iStrobePortBit;
  m_iClockPortBit = _iClockPortBit;
  m_iDataPortBit = _iDataPortBit;
  m_iSkipUpperBits = _iSkipUpperBits;
  m_iDiv1CouplerMask = calcCouplerMask(iFirstDiv1Coupler);
  m_iDiv2CouplerMask = calcCouplerMask(iFirstDiv2Coupler);

//  debugSerial->print("c1 mask ");
//  debug_ShowValue(m_iDiv1CouplerMask);
//  debugSerial->print("c2 mask ");
//  debug_ShowValue(m_iDiv2CouplerMask);

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

unsigned long StopDriver::calcCouplerMask(byte iFirstCoupler) {
  unsigned long iValue = 0;
  
  for (int iBit = iFirstCoupler; iBit < MAX_DIV_STOPS; iBit++) {
    iValue += stopValue(iBit);  
  }

  return iValue;
}

void StopDriver::clockOutput()
{
    delayMicroseconds(2);
    digitalWrite(m_iClockPortBit, LOW); 
    // probably not needed; works without this delay. 
    // this is provided for the one having the 6' long cable.
    delayMicroseconds(2);
    // clock it in
    digitalWrite(m_iClockPortBit, HIGH);     
}


void StopDriver::sendDataEx(unsigned long iDivStops, unsigned long iDivStopState, byte iSkipUpperBits, 
  unsigned long iDivCouplerMask, bool bIncludeCouplers) {
//  Serial.println(iDivStops);

  // always load MSB...LSB
  const unsigned long OUTPUT_MASK = 0x80000000;

/*  debugSerial->println();
  debugSerial->print("test    ");
  debug_ShowValue(1);
  debugSerial->print("state   ");
  debug_ShowValue(iDivStopState);
  debugSerial->print("coupler ");
  debug_ShowValue(iDivCouplerMask);
  debugSerial->print("request ");
  debug_ShowValue(iDivStops); */

  // if couplers to be excluded, tweak to leave 
  if (!bIncludeCouplers) {
    // strip off commanded couplers and instead, substitute current state
    iDivStops = (iDivStops & (0xffffffffffffffff - iDivCouplerMask)) | (iDivStopState & iDivCouplerMask);
                       
//    debugSerial->print("drive   ");
//    debug_ShowValue(iDivStops);
  }

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


void StopDriver::send(unsigned long iDiv1Stops, unsigned long iDiv1State, unsigned long iDiv2Stops, unsigned long iDiv2State, bool bIncludeCouplers) {
  // lock output
  digitalWrite(m_iStrobePortBit, LOW); 

  //debugSerial->println("first ");
  // "bottom" division is farther away from the input and needs to be put in first
  sendDataEx(iDiv2Stops, iDiv2State, 0, m_iDiv2CouplerMask, bIncludeCouplers);
  //debugSerial->println("second ");
  sendDataEx(iDiv1Stops, iDiv1State, m_iSkipUpperBits, m_iDiv1CouplerMask, bIncludeCouplers);
 
  // unlock output
  digitalWrite(m_iStrobePortBit, HIGH); 
}


