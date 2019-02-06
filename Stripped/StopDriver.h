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
    byte m_iSemaphorePortBit;

    unsigned long calcActivation(unsigned long iDivStops, unsigned long iDivStopState);
    unsigned long calcDeactivation(unsigned long iDivStops, unsigned long iDivStopState);

    void sendDataEx(unsigned long iDivStops, unsigned long iDivStopState, byte iSkipUpperBits);
    void clockOutput();
  public:
    StopDriver(int _iStrobePortBit, int _iClockPortBit, int _iDataPortBit, byte _iSkipUpperBits, byte _iSemaphorePortBit);
    void send(unsigned long iDiv1Stops, unsigned long iDiv1StopState, unsigned long iDiv2Stops, unsigned long iDiv2StopState);

    void setSemaphoreValue(bool bOn);

    void setAllOff();
    void testSetAllActive();
    void testSetAllInactive();
};

#endif __STOPDRIVER_H__

