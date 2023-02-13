#include "EEPlugin.h"


class CTimeManager : CPlugin
{
public:

	CTimeManager(CPlugin* pParent, const std::string& sName);
	
	void				Update();
	unsigned long		GetCurrentTimeInMillisecond();
	int					GetTimeElapsedSinceLastUpdate();
	void				PauseTime(bool bPause);
	void				PauseTimeDelay(int nDelayInMilliseconds);
	string				GetName() override;
	bool				IsTimePaused();


private:
	unsigned long		m_nCurrentTime;
	unsigned long		m_nTimeElapsedSinceLastUpdate;
	unsigned long		m_nLastUpdateTime;
	bool				m_bPause = false;
	string				m_sName;
	bool				m_bDelayedPause = false;

	int					m_nTimeWhenDelayedPause = 0;
	int					m_nTimedPauseValue = 0;
	
};