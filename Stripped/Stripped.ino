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

enum TestMode {tmNormal, tmSequential, tmLowLevelCycle, tmCycleAll, tmCycleLeft, tmCycleRight};

TestMode eTestMode = tmNormal;
bool bDebug = false;

String sCommandBuffer;

void setup() {
  debugSerial = &SoftwareSerial(DEBUG_RX_PORT, DEBUG_TX_PORT);
  
  debugSerial->begin(19200);
  debugSerial->println("Starting...");

  sCommandBuffer = "";

  pinMode(HEART_BEAT_LED, OUTPUT);

  pinMode(MIDI_BOARD_SWITCH_PROG, INPUT_PULLUP);
  pinMode(MIDI_BOARD_SWITCH_RECALL, INPUT_PULLUP);
  pinMode(MIDI_BOARD_SWITCH_CLEAR, INPUT_PULLUP);
  
  stopState = new StopState();
  midiReader = new MidiReader(stopState);

  // 2nd division starts at pin 41 (64 pins if all used - 40 pins allocated) / 2 = 12 upper bits to skip on lower value provided last)
  // same offset for both boards.
  // the "41" mentioned below is a port #
  driver_SW_PD = new StopDriver(41, 39, 37, 12, 35);
  driver_CH_GT = new StopDriver(40, 38, 36, 12, 0);  
//  driver_SW_PD = new StopDriver(37, 39, 41, 12);
//  driver_CH_GT = new StopDriver(36, 38, 40, 12);  
}


void handleCommands() {
  while (Serial.available() > 0) {
    // get incoming byte:
    char c = Serial.read();

    // discard unused char
    if (c == 10)
      continue;

    if (c == 13) {
      Serial.println("Executing command:  " + sCommandBuffer);

      if (sCommandBuffer.equalsIgnoreCase("Normal")) {
        eTestMode = tmNormal;
        Serial.println("Normal mode selected");
      } else if (sCommandBuffer.equalsIgnoreCase("Seq")) {
        eTestMode = tmSequential;
        Serial.println("Sequential mode selected");
      } else if (sCommandBuffer.equalsIgnoreCase("LLCycle")) {
        eTestMode = tmLowLevelCycle;
        Serial.println("Low Level Cycle mode selected");
      } else if (sCommandBuffer.equalsIgnoreCase("CycleAll")) {
        eTestMode = tmCycleAll;
        Serial.println("Cycle All mode selected");
      } else if (sCommandBuffer.equalsIgnoreCase("CycleLeft")) {
        eTestMode = tmCycleLeft;
        Serial.println("Cycle Left mode selected");
      } else if (sCommandBuffer.equalsIgnoreCase("CycleRight")) {
        eTestMode = tmCycleRight;
        Serial.println("Cycle Right mode selected");
      } else if (sCommandBuffer.equalsIgnoreCase("DebugOn")) {
        bDebug = true;
        Serial.println("Debug is now on");
      } else if (sCommandBuffer.equalsIgnoreCase("DebugOff")) {
        bDebug = false;
        Serial.println("Debug is now off");
      } else
        Serial.println("Unrecognized command!");

      Serial.println();

      sCommandBuffer = "";
    } else
      sCommandBuffer += c;
  }
}


void normalRun() {
    unsigned long iGreat = 0;
    unsigned long iChior = 0;
    unsigned long iPedal = 0;
    unsigned long iSwell = 0;

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
}  // normalRun


void loop() {
  // TEST:  operate all stops ON, OFF, repeat
  while(1) {
    handleCommands();
  
    switch (eTestMode) {
      tmNormal:
        normalRun();
        break;
  
      tmSequential:
        int iStop=1;
        for (int i=0; i<20; i++) 
        {
          driver_CH_GT->send(iStop, 0,   // CH
                             iStop, 0);  // GT
          iStop=iStop<<1;
          delay(1000);
        }
        break;
  
      tmLowLevelCycle:
        driver_CH_GT->testSetAllActive();
        delay(STOP_DRIVE_TIME_MS);
        driver_CH_GT->setAllOff();
        delay(500);
      
        driver_CH_GT->testSetAllInactive();
        delay(STOP_DRIVE_TIME_MS);
        driver_CH_GT->setAllOff();
        delay(500);
  
        break;
        
      tmCycleAll: 
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
        break;
      
      tmCycleLeft:
        debugSerial->print("PART ON:   ");
        //2048, 1
        driver_CH_GT->send((unsigned long) 0xffffffff, 0,   // CH
                           (unsigned long) 0xffffffff, 0);  // GT
      
        delay(STOP_DRIVE_TIME_MS);
      
        driver_CH_GT->setAllOff();
                             
        delay(1000);
      
        debugSerial->print("PART OFF:  ");
        driver_CH_GT->send(0, 0,   // CH
                           0, 0);  // GT
                           
        delay(STOP_DRIVE_TIME_MS);
      
        driver_CH_GT->setAllOff();
      
        delay(1000);
        break;
      
      tmCycleRight:  
        debugSerial->print("PART ON:   ");
        //2048, 1
        driver_SW_PD->send((unsigned long) 0xffffffff, 0,   // SW
                           (unsigned long) 0xffffffff, 0);  // PD
      
        delay(STOP_DRIVE_TIME_MS);
      
        driver_SW_PD->setAllOff();
                             
        delay(1000);
      
        debugSerial->print("PART OFF:  ");
        driver_SW_PD->send(0, 0,   // SW
                           0, 0);  // PD
                           
        delay(STOP_DRIVE_TIME_MS);
      
        driver_SW_PD->setAllOff();
      
        delay(1000);
        break;
    }
  }
  
  flashHeartbeatLED();
}
