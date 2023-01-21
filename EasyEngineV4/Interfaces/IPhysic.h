#pragma once


#include "EEPlugin.h"

class IPhysic : public CPlugin
{
public:
	//virtual ~IPhysic() = 0 {}

	IPhysic(EEInterface& oInterface) : CPlugin(NULL, "") {}
	virtual float		GetEpsilonError() = 0;
	virtual float		GetGravity() = 0;
	virtual void		SetGravity(float fGravity) = 0;
	virtual void		RestoreGravity() = 0;
	virtual void		SetZCollisionError(float e) = 0;
};