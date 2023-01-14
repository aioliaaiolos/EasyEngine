
#include "Logger.h"


CLogger* CLogger::s_pInstance = nullptr;

CLogger::CLogger() :
	m_bEnabled(false)
{
	s_pInstance = nullptr;
	m_sLogFileName = "log.txt";
	m_oFile.OpenFile(m_sLogFileName, IFileStorage::TOpenMode::eWrite);
}

CLogger::~CLogger()
{
	m_oFile.CloseFile();
}

CAsciiFileStorage& CLogger::Log()
{
	return Instance().m_oFile;
}

CLogger& CLogger::Instance()
{
	if (!s_pInstance)
		s_pInstance = new CLogger;
	return *s_pInstance;
}