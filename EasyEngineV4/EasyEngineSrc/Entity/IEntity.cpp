#include "IEntity.h"
#include "Utils2/TimeManager.h"
#include "IPhysic.h"


CBody::CBody(IPhysic& oPhysic) :
m_fWeight( 0 ),
m_oPhysic(oPhysic)
{
}

void CBody::Update()
{
	unsigned long nTimeElapsedSinceLastUpdate = CTimeManager::Instance()->GetTimeElapsedSinceLastUpdate();
	if( m_fWeight > 0 )
		m_oSpeed.m_y -= (float)( nTimeElapsedSinceLastUpdate ) * m_oPhysic.GetGravity() / 1000.f;
	else
		m_oSpeed.m_y = 0.f;
}


