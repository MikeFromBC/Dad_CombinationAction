#include <SoftwareSerial.h>
#include <MIDI.h>

#include "Types.h"
#include "MidiReader.h"

#define MIDI_CMD_ACTIVATE    73
#define MIDI_CMD_DEACTIVATE  74

#define SYNDYNE_PEDAL_CHANNEL 2
#define SYNDYNE_GREAT_CHANNEL 3
#define SYNDYNE_SWELL_CHANNEL 4
#define SYNDYNE_CHIOR_CHANNEL 5 


MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);


MidiReader::MidiReader() {
  // LED outputs
  SoftSerial.begin(19200);
  SoftSerial.println("Setting up to read MIDI");

  // We want to receive messages on all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);
  
  // We also want to echo the input to the output, 
  // so the sniffer can be dropped inline when things misbehave.
  MIDI.turnThruOn();

  pinMode(PIN_RAW_INPUT, INPUT_PULLUP);
}


void MidiReader::readMessages() {
  if(digitalRead(PIN_RAW_INPUT) == LOW)
  {
    byte input;
    if(SoftSerial.available() != 0)
      input = SoftSerial.read();
  }
  else
  {
    // turn the crank...
    if (MIDI.read())
    {
      switch (MIDI.getType())
      {
        case midi::ControlChange:
          {
//            SoftSerial.print("Controller, chan: ");
//            SoftSerial.print(MIDI.getChannel());
//            SoftSerial.print(" Controller#: ");
//            SoftSerial.print(MIDI.getData1());
//            SoftSerial.print(" Value: ");
//            SoftSerial.println(MIDI.getData2());

            byte iCmd = MIDI.getData1();
			bool bOn = iCmd == MIDI_CMD_ACTIVATE;
			
            int iDiv = MIDI.getChannel();
            int iStopNum = MIDI.getData2() - 1;
            
            if (iStopState != INVALIDSTOPSTATE)
              switch (iDiv)
              {
                case SYNDYNE_SWELL_CHANNEL:
				  stopState.setSwellStop(iStopNum, bOn);
                  break;
  
                case SYNDYNE_GREAT_CHANNEL:
				  stopState.setGreatStop(iStopNum, bOn);
                  break;
                    
                case SYNDYNE_PEDAL_CHANNEL:
				  stopState.setPedalStop(iStopNum, bOn);
                  break;
                    
                case SYNDYNE_CHIOR_CHANNEL:
				  stopState.setChiorStop(iStopNum, bOn);
				  break;
              }

			stopState.debug_ShowAllStopStates("normal"); 
          }
          break;
      }
    }
  }
}
