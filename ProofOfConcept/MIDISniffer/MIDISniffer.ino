/******************************************************************************
MIDI-sniffer.ino
Use SparkFun MIDI Shield as a MIDI data analyzer.

Byron Jacquot, SparkFun Electronics
October 8, 2015
https://github.com/sparkfun/MIDI_Shield/tree/V_1.5/Firmware/MIDI-sniffer

Reads all events arriving over MIDI, and turns them into descriptive text.
If you hold the button on D2, it will switch to display the raw hex values arriving,
which can be useful for viewing incomplete messages and running status.

Resources:

  Requires that the MIDI Sheild be configured to use soft SoftSerial on pins 8 & 9, 
  so that debug text can be printed to the hardware SoftSerial port.

  This code is dependent on the FortySevenEffects MIDI library for Arduino.
  https://github.com/FortySevenEffects/arduino_midi_library
  This was done using version 4.2, hash fb693e724508cb8a473fa0bf1915101134206c34
  This library is now under the MIT license, as well.
  You'll need to install that library into the Arduino IDE before compiling.

  
Development environment specifics:
  It was developed for the Arduino Uno compatible SparkFun RedBoard, with a  SparkFun
  MIDI Shield.
    
  Written, compiled and loaded with Arduino 1.6.5

This code is released under the [MIT License](http://opensource.org/licenses/MIT).

Please review the LICENSE.md file included with this example. If you have any questions 
or concerns with licensing, please contact techsupport@sparkfun.com.

Distributed as-is; no warranty is given.
******************************************************************************/


#include <SoftwareSerial.h>
#include <MsTimer2.h>
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

