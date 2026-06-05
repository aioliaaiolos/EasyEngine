#include "Logger.h"



CLogger::CLogger(EEInterface& oInterface) :
	m_bEnabled(false),
	m_oInterface(oInterface)
{
	m_sLogFileName = "log.txt";
	m_oFile.OpenFile(m_sLogFileName, IFileStorage::TOpenMode::eWrite);
}

CLogger::~CLogger()
{
	m_oFile.CloseFile();
}

CAsciiFileStorage& CLogger::Log()
{
	return m_oFile;
}

string CLogger::GetName()
{
	return "Logger";
}

extern "C" _declspec(dllexport) ILogger* CreateLogger(EEInterface& oInterface)
{
	return new CLogger(oInterface);
}