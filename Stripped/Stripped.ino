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
#define MIDI_BOARD_SWITCH_CLEAR MIDI_BOARD_SWITCH_D4

SoftwareSerial* debugSerial;
StopState* stopState;
StopDriver* driver_CH_GT;
StopDriver* driver_SW_PD;
MidiReader* midiReader;

void setup() {
  debugSerial = &SoftwareSerial(DEBUG_RX_PORT, DEBUG_TX_PORT);
  
  debugSerial->begin(19200);
  debugSerial->println("Starting...");

  pinMode(HEART_BEAT_LED, OUTPUT);

  pinMode(MIDI_BOARD_SWITCH_PROG, INPUT_PULLUP);
  pinMode(MIDI_BOARD_SWITCH_RECALL, INPUT_PULLUP);
  pinMode(MIDI_BOARD_SWITCH_CLEAR, INPUT_PULLUP);
  
  stopState = new StopState();
  midiReader = new MidiReader(stopState);

  // 2nd division starts at pin 41 (64 pins if all used - 40 pins allocated) / 2 = 12 upper bits to skip on lower value provided last)
  // same offset for both boards.
  driver_SW_PD = new StopDriver(41, 39, 37, 12);
  driver_CH_GT = new StopDriver(40, 38, 36, 12);  
//  driver_SW_PD = new StopDriver(37, 39, 41, 12);
//  driver_CH_GT = new StopDriver(36, 38, 40, 12);  
}

void loop() {
  unsigned long iGreat = 0;
  unsigned long iChior = 0;
  unsigned long iSwell = 0;
  unsigned long iPedal = 0;

//while (1)
//  midiReader->readMessages();

/*
while(1) {
  driver_CH_GT->testSetAllActive();
  delay(500);
  driver_CH_GT->setAllOff();
  delay(500);

  driver_CH_GT->test_SetAllInactive();
  delay(500);
  driver_CH_GT->setAllOff();
  delay(500);
}
*/

//debugSerial->print("   DEACT ");
//unsigned long iDivStops = 3;
//unsigned long i = driver_CH_GT->calcDeactivation(iDivStops, 0);
//debugSerial->println(i, HEX);
//
//while (1) ;


// TEST:  operate all stops ON, OFF, repeat
while(1) {
  debugSerial->print("PART ON:   ");
  //2048, 1
  driver_CH_GT->send((unsigned long) 0xffffffff, 0,   // CH
                     (unsigned long) 0xffffffff, 0);  // GT
  driver_SW_PD->send((unsigned long) 0xffffffff, 0,   // SW
                     (unsigned long) 0xffffffff, 0);  // PD

  delay(STOP_DRIVE_TIME_MS);

  driver_CH_GT->setAllOff();
  driver_SW_PD->setAllOff();
                       
  delay(1000);

  debugSerial->print("PART OFF:  ");
  driver_CH_GT->send(0, 0,   // CH
                     0, 0);  // GT
  driver_SW_PD->send(0, 0,   // SW
                     0, 0);  // PD
                     
  delay(STOP_DRIVE_TIME_MS);

  driver_CH_GT->setAllOff();
  driver_SW_PD->setAllOff();

  delay(1000);
}



// operate one stop at a time on all divisions at once
//while(1) {
//  int iStop=1;
//  for (int i=0; i<20; i++) 
//  {
//    driver_CH_GT->send(iStop, 0,   // CH
//                       iStop, 0);  // GT
//    iStop=iStop<<1;
//    delay(1000);
//  }
//}

  while (1) {
    midiReader->readMessages();
  
    if (!digitalRead(MIDI_BOARD_SWITCH_PROG))
    {
      debugSerial->println("program!");
      iGreat = stopState->great;
      iChior = stopState->chior;
      iPedal = stopState->pedal;
      iSwell = stopState->swell;
//      debugSerial->print("Great:  ");
//      debugSerial->println(iGreat);
//      debugSerial->print("Chior:  ");
//      debugSerial->println(iChior);
    } else  
    if (!digitalRead(MIDI_BOARD_SWITCH_CLEAR))
    {
      debugSerial->println("clear!");

      driver_CH_GT->send(0, 0,   // CH
                         0, 0);  // GT
      driver_SW_PD->send(0, 0,   // SW
                         0, 0);  // PD
    
      delay(STOP_DRIVE_TIME_MS);
    
      driver_CH_GT->setAllOff();
      driver_SW_PD->setAllOff();
    } else
    if (!digitalRead(MIDI_BOARD_SWITCH_RECALL))
    {
      debugSerial->println("recall!");
//      debugSerial->print("Great:  ");
//      debugSerial->println(iGreat);
//      debugSerial->print("Chior:  ");
//      debugSerial->println(iChior);

      driver_CH_GT->send(iChior, 0,
                         iGreat, 0);
      driver_SW_PD->send(iSwell, 0,
                         iPedal, 0);
    
      delay(STOP_DRIVE_TIME_MS);
    
      driver_CH_GT->setAllOff();
      driver_SW_PD->setAllOff();
    }
  } 

//  driver_CH_GT->send(2, 0,   // CH
//                     2, 0);  // GT
//                     
//  delay(1000);
//  
//  driver_CH_GT->send(3, 0,   // CH
//                     3, 0);  // GT
//
//  delay(1000);
//  
//  driver_CH_GT->send(0, 0,   // CH
//                     0, 0);  // GT
//
//  delay(1000);

  flashHeartbeatLED();
}
