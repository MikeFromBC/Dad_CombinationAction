#include <SoftwareSerial.h>
#include <Arduino.h>

#include "Utils.h"
#include "Types.h"
#include "MidiReader.h"
#include "StopState.h"

#define NOTE_OFF_MSG_NIBBLE         0x80
#define NOTE_ON_MSG_NIBBLE          0x90
#define AFTERTOUCH_MSG_NIBBLE       0xA0
#define CONTROLLER_MSG_NIBBLE       0xB0
#define PITCH_CHANGE_MSG_NIBBLE     0xC0
#define CHANNEL_PRESSURE_MSG_NIBBLE 0xD0
#define PITCH_BEND_MSG_NIBBLE       0xE0
#define SYSTEM_MSG_NIBBLE           0xF0

#define MIDI_CMD_ACTIVATE    73
#define MIDI_CMD_DEACTIVATE  74

#define SYNDYNE_PEDAL_CHANNEL 1
#define SYNDYNE_GREAT_CHANNEL 2
#define SYNDYNE_SWELL_CHANNEL 3
#define SYNDYNE_CHIOR_CHANNEL 4 

extern SoftwareSerial* debugSerial;

MidiReader::MidiReader(StopState* _stopState) {
  stopState = _stopState;
  
  Serial.begin(31250);

  // clear buffers
  while (Serial.available()) {
    Serial.read();
  }
}


void MidiReader::readMessages() {
  bool bOn;
  byte iCmd;
  int iDiv;
  int iStopNum;
         
  while (Serial.available()) {
    int iBasicCommand = Serial.read();
    int iBasicCommandNibble = iBasicCommand & 0xF0;

debugSerial->print("read: ");
debugSerial->println(iBasicCommand);

    switch (iBasicCommandNibble) {

      case NOTE_OFF_MSG_NIBBLE:
      case NOTE_ON_MSG_NIBBLE:
      case AFTERTOUCH_MSG_NIBBLE:
        // eat
        Serial.read();
        Serial.read();
        break;

      case CONTROLLER_MSG_NIBBLE:
        iDiv = iBasicCommand & 0x0F;

        // data 1
        iCmd = Serial.read();
        bOn = iCmd == MIDI_CMD_ACTIVATE;

        // data 2
        iStopNum = Serial.read() - 1;
         
//        debugSerial->print("Controller, chan: ");
//        debugSerial->print(iDiv);
//        debugSerial->print(" Data 1 (cmd): ");
//        debugSerial->print(iCmd);
//        debugSerial->print(" Data 2 (value): ");
//        debugSerial->println(iStopNum);  

        switch (iDiv)
        {
          case SYNDYNE_SWELL_CHANNEL:
              stopState->setSwellStop(iStopNum, bOn);
              break;

          case SYNDYNE_GREAT_CHANNEL:
              stopState->setGreatStop(iStopNum, bOn);
              break;
                
          case SYNDYNE_PEDAL_CHANNEL:
              stopState->setPedalStop(iStopNum, bOn);
              break;
                
          case SYNDYNE_CHIOR_CHANNEL:
              stopState->setChiorStop(iStopNum, bOn);
              break;
        }  // switch iDiv

        //stopState->debug_ShowAllStopStates("normal");
  
        break;

      case PITCH_CHANGE_MSG_NIBBLE:
      case CHANNEL_PRESSURE_MSG_NIBBLE:
        // no data
        break;

      case PITCH_BEND_MSG_NIBBLE:
        // eat
        Serial.read();
        Serial.read();
        break;

      case SYSTEM_MSG_NIBBLE:
        // ?
        break;
    }  // switch msg type
  }  // while
}

