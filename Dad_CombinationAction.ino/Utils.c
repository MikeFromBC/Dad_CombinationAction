#include "Utils.h"
#include "Types.h"

void debug_ShowValue(unsigned long stops) {
  for (int i=0; i<MAX_STOPS; i++) {
    unsigned long iCurrValue = (unsigned long) 1 << i;
	
    if (stops & iCurrValue)
      SoftSerial.print(1);
      else
      SoftSerial.print(0);              
  }

  SoftSerial.println();                
}


