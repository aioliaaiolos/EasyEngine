#include "TimeManager.h"
#include <windows.h>
#include <winbase.h>


CTimeManager::CTimeManager(CPlugin* pParent, const std::string& sName) :
CPlugin(pParent, sName),
m_nCurrentTime( 0 ),
m_nTimeElapsedSinceLastUpdate( 0 ),
m_sName(sName)
{
	m_sName = "TimeManager";
	m_nCurrentTime = GetTickCount();
}

void CTimeManager::Update()
{
	if (!m_bPause) {
		int nCurrentTime = GetTickCount();
		m_nTimeElapsedSinceLastUpdate = nCurrentTime - m_nLastUpdateTime;
		if (m_nTimeElapsedSinceLastUpdate > 100) {
			nCurrentTime = GetTickCount() - 100;
			m_nTimeElapsedSinceLastUpdate = 100;
		}
		m_nLastUpdateTime = nCurrentTime;
	}
	if (m_bDelayedPause) {
		int nCurrentTime = GetTickCount();
		if (nCurrentTime - m_nTimeWhenDelayedPause > m_nTimedPauseValue) {
			m_bPause = true;
			m_bDelayedPause = false;
		}
	}
}

unsigned long CTimeManager::GetCurrentTimeInMillisecond()
{
	if(!m_bPause)
		m_nCurrentTime = GetTickCount();
	return m_nCurrentTime;
}

int CTimeManager::GetTimeElapsedSinceLastUpdate()
{
	return (int)m_nTimeElapsedSinceLastUpdate;
}

void CTimeManager::PauseTime(bool bPause)
{
	m_bPause = bPause;
}

void CTimeManager::PauseTimeDelay(int nDelayInMilliseconds)
{
	if (!m_bDelayedPause) {
		m_bDelayedPause = true;
		m_bPause = false;
		m_nTimedPauseValue = nDelayInMilliseconds;
		m_nTimeWhenDelayedPause = GetTickCount();
	}
}

string CTimeManager::GetName()
{
	return m_sName;
}

bool CTimeManager::IsTimePaused()
{
	return m_bPause;
}