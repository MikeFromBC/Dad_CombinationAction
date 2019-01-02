#include <SoftwareSerial.h>

#include "Types.h"
#include "Utils.h"
#include "MidiReader.h"
#include "StopDriver.h"

#define MIDI_BOARD_SWITCH_D2  2
#define MIDI_BOARD_SWITCH_D3  3
#define MIDI_BOARD_SWITCH_D4  4

#define MIDI_BOARD_SWITCH_PROG MIDI_BOARD_SWITCH_D2  
#define MIDI_BOARD_SWITCH_RECALL MIDI_BOARD_SWITCH_D3

SoftwareSerial* debugSerial;
StopState* stopState;
StopDriver* driver_GT_CH;
StopDriver* driver_SW_PD;
MidiReader* midiReader;

void setup() {
  debugSerial = &SoftwareSerial(DEBUG_RX_PORT, DEBUG_TX_PORT);
  
  debugSerial->begin(19200);
  debugSerial->println("Starting...");

  pinMode(HEART_BEAT_LED, OUTPUT);

  pinMode(MIDI_BOARD_SWITCH_PROG, INPUT_PULLUP);
  pinMode(MIDI_BOARD_SWITCH_RECALL, INPUT_PULLUP);
  
  stopState = new StopState();
  midiReader = new MidiReader(stopState);

  // 2nd division starts at pin 41 (64 pins if all used - 40 pins allocated) / 2 = 12 upper bits to skip on lower value provided last)
  // same offset for both boards.
  driver_GT_CH = new StopDriver(32, 33, 34, 12);  
  //driver_SW_PD = new StopDriver(20);
}

void loop() {
  unsigned long iGreat = 0;
  unsigned long iChior = 0;

//while (1)
//  midiReader->readMessages();

/*
  while (1) {
    midiReader->readMessages();
  
    if (digitalRead(MIDI_BOARD_SWITCH_PROG))
    {
      iGreat = stopState->great;
      iChior = stopState->chior;
    } else  
    if (digitalRead(MIDI_BOARD_SWITCH_RECALL))
    {
      driver_GT_CH->send(iGreat, iChior);
    }
  } */ 

/*
while(1) {
  driver_GT_CH->testSetAllActive();
  delay(500);
  driver_GT_CH->setAllOff();
  delay(500);

  driver_GT_CH->test_SetAllInactive();
  delay(500);
  driver_GT_CH->setAllOff();
  delay(500);
}
*/

//debugSerial->print("   DEACT ");
//unsigned long iDivStops = 3;
//unsigned long i = driver_GT_CH->calcDeactivation(iDivStops, 0);
//debugSerial->println(i, HEX);
//
//while (1) ;



while(1) {
  debugSerial->print("PART ON:   ");
  //2048, 1
  driver_GT_CH->send((unsigned long) 0xffffffff, 0,   // GT
                     (unsigned long) 0xffffffff, 0);  // CH
                     
  delay(1000);

  debugSerial->print("PART OFF:  ");
  driver_GT_CH->send(0, 0,   // GT
                     0, 0);  // CH
                     
  delay(1000);
}

while(1) {
  int iStop=1;
  for (int i=0; i<20; i++) 
  {
    driver_GT_CH->send(iStop, 0,   // GT
                       iStop, 0);  // CH
    iStop=iStop<<1;
    delay(1000);
  }
}

//  driver_GT_CH->send(2, 0,   // GT
//                     2, 0);  // CH
//                     
//  delay(1000);
//  
//  driver_GT_CH->send(3, 0,   // GT
//                     3, 0);  // CH
//
//  delay(1000);
//  
//  driver_GT_CH->send(0, 0,   // GT
//                     0, 0);  // CH
//
//  delay(1000);

  flashHeartbeatLED();
}
