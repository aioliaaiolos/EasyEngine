#pragma once

#include "Utils\istorage.h"

class CLogger
{
public:	
	static CAsciiFileStorage& Log();


private:
	CLogger();
	CLogger::~CLogger();

	static CLogger&	 Instance();

	CAsciiFileStorage m_oFile;
	string m_sLogFileName;
	bool m_bEnabled;

	static CLogger*	s_pInstance;
};