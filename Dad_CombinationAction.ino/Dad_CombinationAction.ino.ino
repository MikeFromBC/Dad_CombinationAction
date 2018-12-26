#include "MidiReader.h"

MidiReader reader;

void setup()
{
}


void loop()
{
  reader.readMessages();
}

//Swell:  111111111111111000000000000000
//Great:  111111111000000000000000000000
//Chior:  111111111111000000000000000000
//Pedal:  111111111000000000000000000000

