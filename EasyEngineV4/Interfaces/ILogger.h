#ifndef ILOGGER_H
#define ILOGGER_H

#include "EEPlugin.h"

class ILogger : public CPlugin
{
public:
	ILogger() : CPlugin(nullptr, "") {}
	//virtual void Log() = 0;
	virtual CAsciiFileStorage& Log() = 0;
};

#endif // ILOGGER_H