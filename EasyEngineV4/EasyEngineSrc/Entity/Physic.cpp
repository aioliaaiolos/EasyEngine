#include "Physic.h"



CPhysic::CPhysic(EEInterface& oInterface) :
	IPhysic(oInterface),
	m_fGravity(0.1),
	m_fZCollisionError(10.f),
	m_oInterface(oInterface)
{
	m_fEpsilonError = 0.0001f * m_fGravity * m_fGravity / 1000.f;
}

string CPhysic::GetName()
{
	return "Physic";
}

void CPhysic::SetGravity(float fGravity)
{
	m_fGravity = fGravity;
}

void CPhysic::SetZCollisionError(float e)
{
	m_fZCollisionError = e;
}

float CPhysic::GetZCollisionError()
{
	return m_fZCollisionError;
}

float CPhysic::GetGravity()
{
	return m_fGravity;
}

float CPhysic::GetEpsilonError() 
{ 
	return m_fEpsilonError; 
}

void CPhysic::RestoreGravity()
{
	m_fGravity = m_fEarthGravity;
}

extern "C" _declspec(dllexport) IPhysic* CreatePhysic(EEInterface& oInterface)
{
	return new CPhysic(oInterface);
}