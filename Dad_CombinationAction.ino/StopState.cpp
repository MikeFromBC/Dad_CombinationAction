#include "StopState.h"
#include "Types.h"

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
	SoftSerial.println(sCaller);
	debug_ShowStopState("great", great);
	debug_ShowStopState("swell", swell);
	debug_ShowStopState("chior", chior);
	debug_ShowStopState("pedal", pedal);
    SoftSerial.println();              
}


void StopState::debug_ShowStopState(char* sDiv, unsigned long iStops) {
	SoftSerial.print(sDiv);
	SoftSerial.print("  ");
	debug_ShowValue(iStops);
}


void StopState::toggleStop(unsigned long* piDivStops, int iStop) {
  unsigned long iStopValue = stopValue(iStop);
  *piDivStops ^= iStopValue;
}


void StopState::setStop(unsigned long* piDivStops, int iStop, bool bOn) {
  unsigned long iStopValue = stopValue(iStop);
  
  if (bOn)
    *piDivStops |= iStopValue;
    else
	*piDivStops &= ((unsigned long) 0xffffffff - iStopValue);
}


// first stop is 0
unsigned long StopState::stopValue(int iStop) {
  return (long) 1 << iStop;
}


void StopState::setSwellStop(int iStop, bool bOn) {
	setStop(&swell, iStop, bOn);
}


void StopState::setGreatStop(int iStop, bool bOn)
{
	setStop(&great, iStop, bOn);
}


void StopState::setChiorStop(int iStop, bool bOn) {
	setStop(&chior, iStop, bOn);
}


void StopState::setPedalStop(int iStop, bool bOn) {
	setStop(&pedal, iStop, bOn);
}


void StopState::toggleSwellStop(int iStop) {
	toggleStop(&swell, iStop);
}


void StopState::toggleGreatStop(int iStop) {
	toggleStop(&great, iStop);
}


void StopState::toggleChiorStop(int iStop) {
	toggleStop(&chior, iStop);
}


void StopState::togglePedalStop(int iStop) {
	toggleStop(&pedal, iStop);
}

