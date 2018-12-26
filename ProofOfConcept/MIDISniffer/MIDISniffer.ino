#include <SoftwareSerial.h>
#include <MIDI.h>

#define PIN_RAW_INPUT 2

#define PIN_POT_A0 0
#define PIN_POT_A1 1

#define MIDI_CMD_ACTIVATE    73
#define MIDI_CMD_DEACTIVATE  74


static const uint16_t DEBOUNCE_COUNT = 50;

// Need to use soft SoftSerial, so we can report what's happening
// via messages on hard SoftSerial.
SoftwareSerial SoftSerial(50, 51);

/* Args:
   - type of port to use (hard/soft)
   - port object name
   - name for this midi instance
*/
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI);

#define MAX_STOPS 32

#define PEDAL 2
#define GREAT 3
#define SWELL 4
#define CHIOR 5 

#define STOP_INACTIVE     0
#define STOP_ACTIVE       1
#define INVALIDSTOPSTATE  2

long swell;
long great;
long pedal;
long chior;

void setup()
{
  // LED outputs
  SoftSerial.begin(19200);
  SoftSerial.println("Setting up");

  // We want to receive messages on all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);
  
  // We also want to echo the input to the output, 
  // so the sniffer can be dropped inline when things misbehave.
  MIDI.turnThruOn();

  swell=0;
  great=0;
  chior=0;
  pedal=0;

  pinMode(PIN_RAW_INPUT, INPUT_PULLUP);
}


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


void loop()
{
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
        case midi::ControlChange :
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
            
//            SoftSerial.print("iStopNum  " );              
//            SoftSerial.println(iStopNum);              
//            SoftSerial.print("iStopValue  " );              
//            SoftSerial.println(iStopValue);              

            if (iStopState != INVALIDSTOPSTATE)
              switch (iDiv)
              {
                case SWELL:
                  updateStops(&swell, iCmd, iStopValue);
                  break;
  
                case GREAT:
                  updateStops(&great, iCmd, iStopValue);
                  break;
                    
                case PEDAL:
                  updateStops(&pedal, iCmd, iStopValue);
                  break;
                    
                case CHIOR:
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

//Swell:  111111111111111000000000000000
//Great:  111111111000000000000000000000
//Chior:  111111111111000000000000000000
//Pedal:  111111111000000000000000000000

