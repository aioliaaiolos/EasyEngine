#pragma once


#include "Interface.h"
#include "IPhysic.h"


class CPhysic : public IPhysic
{
public:

	CPhysic(EEInterface& oInterface);
	float		GetEpsilonError() override;
	void		RestoreGravity() override;
	void		SetZCollisionError(float e);
	float		GetZCollisionError();
	float		GetGravity();
	void		SetGravity(float fGravity);
	string		GetName();

private:
	float	m_fEpsilonError;
	float	m_fZCollisionError;
	float	m_fGravity;
	const float m_fEarthGravity = 1000.f;

	EEInterface& m_oInterface;
};

extern "C" _declspec(dllexport) IPhysic* CreatePhysic(EEInterface& oInterface);