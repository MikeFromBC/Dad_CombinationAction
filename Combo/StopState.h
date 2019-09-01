#ifndef __STOPSTATE_H__
#define __STOPSTATE_H__

class StopState
{
  private:
	  void setStop(unsigned long* piDivStops, int iStopNum, bool bOn);
  public:
    unsigned long swell;
    unsigned long great;
    unsigned long chior;
    unsigned long pedal;

    StopState();
    void clear();
  
  	void debug_ShowAllStopStates(char* sCaller);
  	void debug_ShowStopState(char* sDiv, unsigned long iStops);
  	
  	void setSwellStop(int iStopNum, bool bOn);
  	void setGreatStop(int iStopNum, bool bOn);
  	void setChiorStop(int iStopNum, bool bOn);
  	void setPedalStop(int iStopNum, bool bOn);
  };

#endif __STOPSTATE_H__

