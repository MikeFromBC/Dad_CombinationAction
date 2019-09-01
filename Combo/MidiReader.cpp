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

#define SYNDYNE_FIRST_GREAT_COUPLER_STOP 16  /* in this firmware, stops start at 0 so 3rd 8-conductor connector begins with position 16 */
#define SYNDYNE_FIRST_PEDAL_COUPLER_STOP 16  /* in this firmware, stops start at 0 so 3rd 8-conductor connector begins with position 16 */
#define SYNDYNE_FIRST_CHIOR_COUPLER_STOP 16  /* in this firmware, stops start at 0 so 3rd 8-conductor connector begins with position 16 */

#define SYNDYNE_CONNECTOR_CONDUCTORS 8   /* stop input board incoming connectors */

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

    // leave these here; without it, MIDI messages are sometimes incomplete
    debugSerial->print(millis());
    debugSerial->print(":  read: ");
    debugSerial->println(iBasicCommand, HEX);

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
////        debugSerial->print(" Data1 (cmd): ");
////        debugSerial->print(iCmd);
//        debugSerial->print(" Data2 (value): ");
//        debugSerial->println(iStopNum + 1);  // show in industry standard 

        switch (iDiv)
        {
          case SYNDYNE_SWELL_CHANNEL:
              stopState->setSwellStop(iStopNum, bOn);
              break;

          case SYNDYNE_GREAT_CHANNEL:
              // compensate:  make couplers contiguous with stops; they're not coming
              //              from Syndyne that way because of the 8 pin connectors not
              //              facilitating contiguous use of inputs. 
              if (iStopNum >= SYNDYNE_FIRST_GREAT_COUPLER_STOP)
                iStopNum-=SYNDYNE_CONNECTOR_CONDUCTORS - 1;  // only 1 of 8 was used on the previous syndyne connector
                
              stopState->setGreatStop(iStopNum, bOn);
              break;
                
          case SYNDYNE_PEDAL_CHANNEL:
              // compensate:  make couplers contiguous with stops; they're not coming
              //              from Syndyne that way because of the 8 pin connectors not
              //              facilitating contiguous use of inputs. 
              if (iStopNum >= SYNDYNE_FIRST_PEDAL_COUPLER_STOP)
                iStopNum-=SYNDYNE_CONNECTOR_CONDUCTORS - 1;  // only 1 of 8 was used on the previous syndyne connector
                
              stopState->setPedalStop(iStopNum, bOn);
              break;
                
          case SYNDYNE_CHIOR_CHANNEL:
              // compensate:  make couplers contiguous with stops; they're not coming
              //              from Syndyne that way because of the 8 pin connectors not
              //              facilitating contiguous use of inputs. 
              if (iStopNum >= SYNDYNE_FIRST_CHIOR_COUPLER_STOP)
                iStopNum-=SYNDYNE_CONNECTOR_CONDUCTORS - 4;    // only 4 of 8 was used on the previous syndyne connector
                
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

