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

#define MaxStops 30
typedef byte TMem[MaxStops];

#define PEDAL 2
#define GREAT 3
#define SWELL 4
#define CHIOR 5 

#define STOP_INACTIVE     0
#define STOP_ACTIVE       1
#define INVALIDSTOPSTATE  2

TMem swell;
TMem great;
TMem pedal;
TMem chior;

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

  for (int i=0; i<MaxStops; i++)
  {
    swell[i]=0;
    great[i]=0;
    chior[i]=0;
    pedal[i]=0;
  }

  pinMode(PIN_RAW_INPUT, INPUT_PULLUP);
}


void loop()
{
  static uint8_t  ticks = 0;
  static uint8_t  old_ticks = 0;

  // put your main code here, to run repeatedly:

  if(digitalRead(PIN_RAW_INPUT) == LOW)
  {
    // If you hold button D2 on the shield, we'll print
    // the raw hex values from the MIDI input.
    //
    // This can be useful if you need to troubleshoot issues with
    // running status

    byte input;
    if(SoftSerial.available() != 0)
    {
      input = SoftSerial.read();
    
      if(input & 0x80)
      {
        SoftSerial.println();
      }
      SoftSerial.print(input, HEX);
      SoftSerial.print(' ');
    }
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
            
          SoftSerial.println(iCmd);              

            switch (iCmd) {
              case MIDI_CMD_ACTIVATE:
          SoftSerial.println("act");              
                iStopState=STOP_ACTIVE;
                break;

              case MIDI_CMD_DEACTIVATE:
          SoftSerial.println("deact");              
                iStopState=STOP_INACTIVE;
                break;

              default: iStopState=INVALIDSTOPSTATE;
            }

            int iDiv = MIDI.getChannel();
            int iStopNum = MIDI.getData2() - 1;
            
            if (iStopState != INVALIDSTOPSTATE)
              switch (iDiv)
              {
                case SWELL:
                  swell[iStopNum] = iStopState;
                  break;
  
                case GREAT:
                  great[iStopNum] = iStopState;
                  break;
                    
                case PEDAL:
                  pedal[iStopNum] = iStopState;
                  break;
                    
                case CHIOR:
                  chior[iStopNum] = iStopState;
                  break;
              }

            SoftSerial.print("Swell:  ");
            for (int i=0; i<MaxStops; i++) {
              SoftSerial.print(swell[i]);              
            }
            SoftSerial.println();              
            SoftSerial.print("Great:  ");
            for (int i=0; i<MaxStops; i++) {
              SoftSerial.print(great[i]);              
            }
            SoftSerial.println();              
            SoftSerial.print("Chior:  ");
            for (int i=0; i<MaxStops; i++) {
              SoftSerial.print(chior[i]);              
            }
            SoftSerial.println();              
            SoftSerial.print("Pedal:  ");
            for (int i=0; i<MaxStops; i++) {
              SoftSerial.print(pedal[i]);              
            }
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

