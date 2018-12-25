#ifndef __STOPSTATE_H__
#define __STOPSTATE_H__

class StopState
{
  private:
    unsigned long stopValue(int iStop);
	void turnOffStop(unsigned long* piDivStops, int iStop);
  public:
	unsigned long swell;
	unsigned long great;
	unsigned long chior;
	unsigned long pedal;
	
    StopState();
    void clear();
	void turnOffSwellStop(int iStop);
	void turnOnSwellStop(int iStop);
	void turnOffGreatStop(int iStop);
	void turnOnGreatStop(int iStop);
	void turnOffChiorStop(int iStop);
	void turnOnChiorStop(int iStop);
	void turnOffPedalStop(int iStop);
	void turnOnPedalStop(int iStop);
};

#endif __STOPSTATE_H__

