#ifndef __STOPDRIVER_H__
#define __STOPDRIVER_H__

class StopDriver
{
  private:
    int m_iStrobePortBit;
    int m_iClockPortBit;
    int m_iDataPortBit;
    int m_iUpperDivBitOffset;
    byte m_iSkipUpperBits;

    void sendDataEx(unsigned long iDivStops, unsigned long iDivStopState, byte iSkipUpperBits);
    void clockOutput();
  public:
    StopDriver(int _iStrobePortBit, int _iClockPortBit, int _iDataPortBit, byte _iSkipUpperBits);
    void send(unsigned long iDiv1Stops, unsigned long iDiv1StopState, unsigned long iDiv2Stops, unsigned long iDiv2StopState);

    void setAllOff();
    void testSetAllActive();
    void testSetAllInactive();
};

#endif __STOPDRIVER_H__

