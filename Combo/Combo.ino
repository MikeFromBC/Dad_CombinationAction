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

enum TestMode {tmNormal, tmSequential, tmLowLevelCycle, tmCycleAll, tmCycleLeft, tmCycleRight, tmCustom};

TestMode eTestMode = tmCustom;// tmSequential;//tmNormal;
bool bDebug = false;

String sCommandBuffer;

void setup() {
  debugSerial = &SoftwareSerial(DEBUG_RX_PORT, DEBUG_TX_PORT);
  
  debugSerial->begin(19200);
  debugSerial->println("Starting...");

  sCommandBuffer.reserve(50);
  sCommandBuffer = "";

  pinMode(HEART_BEAT_LED, OUTPUT);

  pinMode(MIDI_BOARD_SWITCH_PROG, INPUT_PULLUP);
  pinMode(MIDI_BOARD_SWITCH_RECALL, INPUT_PULLUP);
  pinMode(MIDI_BOARD_SWITCH_CLEAR, INPUT_PULLUP);
  
  pinMode(30, OUTPUT);
  
  stopState = new StopState();
  midiReader = new MidiReader(stopState);

  // 2nd division starts at pin 41 (64 pins if all used - 40 pins allocated) / 2 = 12 upper bits to skip on lower value provided last)
  // same offset for both boards.
  // the "41" mentioned below is a port #
  driver_SW_PD = new StopDriver(49, 47, 45, 12, 35);
  driver_CH_GT = new StopDriver(48, 46, 44, 12, 0);  
//  driver_SW_PD = new StopDriver(37, 39, 41, 12);
//  driver_CH_GT = new StopDriver(36, 38, 40, 12);  
}


void handleCommands() {
  while (debugSerial->available() > 0) {
    // get incoming byte:
    char c = debugSerial->read();

    // discard unused char
    if (c == 10)
      continue;

    if (c == 13) {
      debugSerial->print("Executing command:  ");
      debugSerial->println(sCommandBuffer);

      if (sCommandBuffer.equalsIgnoreCase("Normal")) {
        eTestMode = tmNormal;
        debugSerial->println("Normal mode selected");
      } else if (sCommandBuffer.equalsIgnoreCase("Custom")) {
        eTestMode = tmCustom;
        debugSerial->println("Custom test mode selected");
      } else if (sCommandBuffer.equalsIgnoreCase("Seq")) {
        eTestMode = tmSequential;
        debugSerial->println("Sequential mode selected");
      } else if (sCommandBuffer.equalsIgnoreCase("LLCycle")) {
        eTestMode = tmLowLevelCycle;
        debugSerial->println("Low Level Cycle mode selected");
      } else if (sCommandBuffer.equalsIgnoreCase("CycleAll")) {
        eTestMode = tmCycleAll;
        debugSerial->println("Cycle All mode selected");
      } else if (sCommandBuffer.equalsIgnoreCase("CycleLeft")) {
        eTestMode = tmCycleLeft;
        debugSerial->println("Cycle Left mode selected");
      } else if (sCommandBuffer.equalsIgnoreCase("CycleRight")) {
        eTestMode = tmCycleRight;
        debugSerial->println("Cycle Right mode selected");
      } else if (sCommandBuffer.equalsIgnoreCase("DebugOn")) {
        bDebug = true;
        debugSerial->println("Debug is now on");
      } else if (sCommandBuffer.equalsIgnoreCase("DebugOff")) {
        bDebug = false;
        debugSerial->println("Debug is now off");
      } else
        debugSerial->println("Unrecognized command!");  

      sCommandBuffer = "";  
    } else
      sCommandBuffer+=c;
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
  int iStop;

  while(1) {
    driver_CH_GT->setAllOff();
    driver_SW_PD->setAllOff();

    handleCommands();

    switch (eTestMode) {
  /*
      case tmNormal:
        normalRun();
        break;  */
  
      case tmSequential:
        iStop=1;
        
        // could use MAX_DIV_STOPS instead but we don't have that many stops
        for (int i=0; i<15; i++) 
        {
          driver_CH_GT->send(iStop, 0,   // CH
                             iStop>>2, 0);  // GT
          driver_SW_PD->send(iStop, 0,   // SW
                             iStop>>2, 0);  // PD

          delay(STOP_DRIVE_TIME_MS);
          
          driver_CH_GT->setAllOff();
          driver_SW_PD->setAllOff();
          
          iStop=iStop<<1;
          delay(1000);
        }
        break;

      case tmCustom:
        // could use MAX_DIV_STOPS instead but we don't have that many stops
        for (int i=0; i<15; i++) 
        {
          debugSerial->println("(begin)");
          
          driver_CH_GT->send(0xffffffff, 0,   // CH
                             0xffffffff, 0);  // GT

          driver_SW_PD->send(0xffffffff, 0,   // SW
                             0xffffffff, 0);  // PD

          delay(STOP_DRIVE_TIME_MS);
          
          driver_CH_GT->setAllOff();
          driver_SW_PD->setAllOff();
          
          delay(1000);

          driver_CH_GT->send(0, 0,   // SW
                             0, 0);  // PD

          driver_SW_PD->send(0, 0,   // SW
                             0, 0);  // PD

          delay(STOP_DRIVE_TIME_MS);
          
          driver_CH_GT->setAllOff();
          driver_SW_PD->setAllOff();
          
          delay(1000);
        }
        break;
        
  /*
      case tmLowLevelCycle:
        driver_CH_GT->testSetAllActive();
        delay(STOP_DRIVE_TIME_MS);
        driver_CH_GT->setAllOff();
        delay(500);
      
        driver_CH_GT->testSetAllInactive();
        delay(STOP_DRIVE_TIME_MS);
        driver_CH_GT->setAllOff();
        delay(500);
  
        break;
        
      case tmCycleAll: 
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
      
      case tmCycleLeft:
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
      
      case tmCycleRight:  
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
        break;  */
    }  
  }
  
  flashHeartbeatLED();
}
