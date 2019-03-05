#include <SoftwareSerial.h>
#include <MIDI.h>

#include "Utils.h"
#include "Types.h"
#include "MidiReader.h"
#include "StopState.h"

#define MIDI_CMD_ACTIVATE    73
#define MIDI_CMD_DEACTIVATE  74

#define SYNDYNE_PEDAL_CHANNEL 2
#define SYNDYNE_GREAT_CHANNEL 3
#define SYNDYNE_SWELL_CHANNEL 4
#define SYNDYNE_CHIOR_CHANNEL 5 

extern SoftwareSerial* debugSerial;

MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);


MidiReader::MidiReader(StopState* _stopState) {
  stopState = _stopState;
  
  // We want to receive messages on all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);
  
  // We also want to echo the input to the output, 
  // so the sniffer can be dropped inline when things misbehave.
  MIDI.turnThruOn();
}


void MidiReader::readMessages() {
  if (MIDI.read())
  {
    switch (MIDI.getType())
    {
      case midi::ControlChange:
        {  /*
          debugSerial->print("Controller, chan: ");
          debugSerial->print(MIDI.getChannel());
          debugSerial->print(" Controller#: ");
          debugSerial->print(MIDI.getData1());
          debugSerial->print(" Value: ");
          debugSerial->println(MIDI.getData2());  */

          byte iCmd = MIDI.getData1();
          bool bOn = iCmd == MIDI_CMD_ACTIVATE;
    
          int iDiv = MIDI.getChannel();
          int iStopNum = MIDI.getData2() - 1;
   
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

          stopState->debug_ShowAllStopStates("normal");
        }  // case
        break;  
    }  // switch msg type 
  }  // if
}

