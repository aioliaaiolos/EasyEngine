#include "IEntity.h"
#include "Utils2/TimeManager.h"
#include "IPhysic.h"


CBody::CBody(EEInterface& oInterface) :
m_fWeight( 0 ),
m_oPhysic(static_cast<IPhysic&>(*oInterface.GetPlugin("Physic"))),
m_oTimeManager(static_cast<CTimeManager&>(*oInterface.GetPlugin("TimeManager")))
{
}

void CBody::Update()
{
	unsigned long nTimeElapsedSinceLastUpdate = m_oTimeManager.GetTimeElapsedSinceLastUpdate();
	if( m_fWeight > 0 )
		m_oSpeed.m_y -= (float)( nTimeElapsedSinceLastUpdate ) * m_oPhysic.GetGravity() / 1000.f;
	else
		m_oSpeed.m_y = 0.f;
}


