#pragma once

#include "Utils\istorage.h"
#include "ILogger.h"

class CLogger : public ILogger
{
public:		
	CLogger(EEInterface& oInterface);
	CLogger::~CLogger();
	CAsciiFileStorage& Log();
	string GetName() override;

private:
	

	CAsciiFileStorage m_oFile;
	string m_sLogFileName;
	bool m_bEnabled;
	EEInterface& m_oInterface;
	
};

extern "C" _declspec(dllexport) ILogger* CreateLogger(EEInterface& oInterface);