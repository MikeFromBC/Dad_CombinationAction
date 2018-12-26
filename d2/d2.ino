#include <SoftwareSerial.h>

#include "Types.h"
#include "MidiReader.h"

extern SoftwareSerial* debugSerial(DEBUG_RX_PORT, DEBUG_TX_PORT);

MidiReader* midiReader;

void setup() {
  debugSerial = &SoftwareSerial(DEBUG_RX_PORT, DEBUG_TX_PORT);
  
  debugSerial->begin(19200);
  debugSerial->println("Setting up to read MIDI");

  midiReader = &MidiReader();
}

void loop() {
  midiReader->readMessages();
}
