#ifndef __MIDIREADER_H__
#define __MIDIREADER_H__

#include "StopState.h"

class MidiReader
{
  private:
    StopState* stopState;
  public:
    MidiReader(StopState* _stopState);
    void readMessages();
};

#endif  __MIDIREADER_H__

