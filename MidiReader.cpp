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

#define STOP_INACTIVE     0
#define STOP_ACTIVE       1
#define INVALIDSTOPSTATE  2


MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);


void showValue(long stops) {
  for (int i=0; i<MAX_STOPS; i++) {
    long iCurrValue = (long) 1 << i;
    if (stops & iCurrValue)
      SoftSerial.print(1);
      else
      SoftSerial.print(0);              
  }

  SoftSerial.println();                
}


void updateStops(long* pStops, byte iCmd, long iStopValue) {
  if (iCmd == MIDI_CMD_ACTIVATE)
    *pStops |= iStopValue;
    else
    *pStops &= ((long) 0xffffffff - iStopValue);
}


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

            byte iStopState;
            byte iCmd = MIDI.getData1();
            
            switch (iCmd) {
              case MIDI_CMD_ACTIVATE:
                iStopState=STOP_ACTIVE;
                break;

              case MIDI_CMD_DEACTIVATE:
                iStopState=STOP_INACTIVE;
                break;

              default: iStopState=INVALIDSTOPSTATE;
            }

            int iDiv = MIDI.getChannel();
            int iStopNum = MIDI.getData2() - 1;
            long iStopValue = (long) 1 << iStopNum;
            
            if (iStopState != INVALIDSTOPSTATE)
              switch (iDiv)
              {
                case SYNDYNE_SWELL_CHANNEL:
                  updateStops(&swell, iCmd, iStopValue);
                  break;
  
                case SYNDYNE_GREAT_CHANNEL:
                  updateStops(&great, iCmd, iStopValue);
                  break;
                    
                case SYNDYNE_PEDAL_CHANNEL:
                  updateStops(&pedal, iCmd, iStopValue);
                  break;
                    
                case SYNDYNE_CHIOR_CHANNEL:
                  updateStops(&chior, iCmd, iStopValue);
                  break;
              }

            SoftSerial.print("Swell:  ");
            showValue(swell);
            SoftSerial.print("Great:  ");
            showValue(great);
            SoftSerial.print("Chior:  ");
            showValue(chior);
            SoftSerial.print("Pedal:  ");
            showValue(pedal);
            SoftSerial.println();              
          }
          break;
      }
    }
  }
}
