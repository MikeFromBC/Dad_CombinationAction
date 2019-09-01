#include <SoftwareSerial.h>

//#include "Types.h"
#include "StopState.h"
#include "Utils.h"

extern SoftwareSerial* debugSerial;

StopState::StopState() {
	clear();
}

void StopState::clear() {
  swell = 0;
  great = 0;
  chior = 0;
  pedal = 0;
}


void StopState::debug_ShowAllStopStates(char* sCaller) {
	debugSerial->println(sCaller);
	debug_ShowStopState("great", great);
	debug_ShowStopState("swell", swell);
	debug_ShowStopState("chior", chior);
	debug_ShowStopState("pedal", pedal);
  debugSerial->println();              
}


void StopState::debug_ShowStopState(char* sDiv, unsigned long iStops) {
	debugSerial->print(sDiv);
	debugSerial->print("  ");
	debug_ShowValue(iStops);
}


void StopState::setStop(unsigned long* piDivStops, int iStopNum, bool bOn) {
  unsigned long iStopValue = stopValue(iStopNum);
  
  if (bOn)
    *piDivStops |= iStopValue;
    else
  	*piDivStops &= ((unsigned long) 0xffffffff - iStopValue);
}


void StopState::setSwellStop(int iStopNum, bool bOn) {
	setStop(&swell, iStopNum, bOn);
}


void StopState::setGreatStop(int iStopNum, bool bOn)
{
	setStop(&great, iStopNum, bOn);
}


void StopState::setChiorStop(int iStopNum, bool bOn) {
	setStop(&chior, iStopNum, bOn);
}


void StopState::setPedalStop(int iStopNum, bool bOn) {
	setStop(&pedal, iStopNum, bOn);
}

