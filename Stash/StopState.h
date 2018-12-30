#ifndef __STOPSTATE_H__
#define __STOPSTATE_H__

class StopState
{
  private:
    unsigned long stopValue(int iStop);
	void toggleStop(unsigned long* piDivStops, int iStop);
	void setStop(unsigned long* piDivStops, int iStop, bool bOn);
  public:
	unsigned long swell;
	unsigned long great;
	unsigned long chior;
	unsigned long pedal;
	
    StopState();
    void clear();

	void debug_ShowAllStopStates(char* sCaller);
	void debug_ShowStopState(char* sDiv, unsigned long iStops);
	
	void setSwellStop(int iStop, bool bOn);
	void setGreatStop(int iStop, bool bOn);
	void setChiorStop(int iStop, bool bOn);
	void setPedalStop(int iStop, bool bOn);

	void toggleSwellStop(int iStop);
	void toggleGreatStop(int iStop);
	void toggleChiorStop(int iStop);
	void togglePedalStop(int iStop);
};

#endif __STOPSTATE_H__

