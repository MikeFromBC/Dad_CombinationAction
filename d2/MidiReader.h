#ifndef __MIDIREADER_H__
#define __MIDIREADER_H__

#include "StopState.h"

class MidiReader
{
  private:
  public:
    StopState stopState;
    
    MidiReader();
    void readMessages();
};

#endif  __MIDIREADER_H__

