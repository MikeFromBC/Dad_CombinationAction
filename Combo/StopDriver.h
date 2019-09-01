#ifndef __STOPDRIVER_H__
#define __STOPDRIVER_H__

class StopDriver
{
  private:
    byte m_iStrobePortBit;
    byte m_iClockPortBit;
    byte m_iDataPortBit;
    byte m_iUpperDivBitOffset;
    byte m_iSkipUpperBits;
    unsigned long m_iDiv1CouplerMask;
    unsigned long m_iDiv2CouplerMask;

    void sendDataEx(unsigned long iDivStops, unsigned long iDivStopState, byte iSkipUpperBits, unsigned long iDivCouplerMask, bool bIncludeCouplers);
    void clockOutput();
    // starts at 0
    unsigned long calcCouplerMask(byte iFirstCoupler);
  public:
    StopDriver(byte _iStrobePortBit, 
      byte _iClockPortBit, byte _iDataPortBit, byte _iSkipUpperBits, byte iFirstDiv1Coupler, byte iFirstDiv2Coupler);
    void send(unsigned long iDiv1Stops, unsigned long iDiv1StopState, unsigned long iDiv2Stops, unsigned long iDiv2StopState, bool bIncludeCouplers);

    void setAllOff();
    void testSetAllActive();
    void testSetAllInactive();
};

#endif __STOPDRIVER_H__

