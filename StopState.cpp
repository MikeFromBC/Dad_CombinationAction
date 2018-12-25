#include "StopState.h"

StopState::StopState() {
	clear();
}

void StopState::clear() {
  swell = 0;
  great = 0;
  chior = 0;
  pedal = 0;
}


void turnOffStop(unsigned long* piDivStops, int iStop) {
	*piDivStops &= ((long) 0xffffffff - iStopValue);
	
	
	    *pStops |= iStopValue;
    else
    *pStops &= 

}


// first stop is 0
unsigned long StopState::stopValue(int iStop) {
  return (long) 1 << iStop;
}
