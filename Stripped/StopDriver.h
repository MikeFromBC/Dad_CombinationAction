#ifndef __STOPDRIVER_H__
#define __STOPDRIVER_H__

class StopDriver
{
  private:
    int iStrobePortBit;
    int iClockPortBit;
    int iDataPortBit;
    int iUpperDivBitOffset;
    byte iSkipUpperBits;

    unsigned long calcActivation(unsigned long iDivStops, unsigned long iDivStopState);
    unsigned long calcDeactivation(unsigned long iDivStops, unsigned long iDivStopState);

    void sendDataEx(unsigned long iDivStops, unsigned long iDivStopState, byte iSkipUpperBits);
    void clockOutput();
    void setAllOff();
  public:
    StopDriver(int _iStrobePortBit, int _iClockPortBit, int _iDataPortBit, byte _iSkipUpperBits);
    void send(unsigned long iDiv1Stops, unsigned long iDiv1StopState, unsigned long iDiv2Stops, unsigned long iDiv2StopState);
};

#endif __STOPDRIVER_H__

