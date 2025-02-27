#include "IAEntity.h"
#include "Utils2/TimeManager.h"
#include "ICollisionManager.h"
#include "IGeometry.h"
#include "Utils2/Logger.h"

const float fRotateSpeed = 10.f;
const float g_fArrivedAtDestinationDistance = 115.f;

IAEntity::IAEntity(EEInterface& oInterface):
m_nRecoveryTime( 1000 ),
m_bHitEnemy( false ),
m_eFightState( eNoFight ),
m_eTalkToState(eNoTalkTo),
m_fAngleRemaining( 0.f ),
m_bArriveAtDestination( true ),
m_fDestinationDeltaRadius(g_fArrivedAtDestinationDistance),
m_nCurrentPathPointNumber( 0 ),
m_pCurrentEnemy(NULL),
m_bFaceToTarget(false),
m_pCurrentInterlocutor(nullptr),
m_oTalkToCallback(nullptr, nullptr),
m_fForceDeltaRotation(0.),
m_oTimeManager(static_cast<CTimeManager&>(*oInterface.GetPlugin("TimeManager")))
{
}

void IAEntity::UpdateFightState()
{
	float distance = 0;
	CVector oEnemyPos;
	switch( m_eFightState )
	{
	case eNoFight:
		break;
	case eBeginHitReceived:
		if( GetLife() <= 0 )
			m_eFightState = eNoFight;
		else
			m_eFightState = eReceivingHit;
		break;
	case eBeginGoToEnemy:
		m_pCurrentEnemy->GetPosition( oEnemyPos );
		Goto( oEnemyPos, -1.f );
		m_eFightState = eGoingToEnemy;
		break;
	case eGoingToEnemy:
		m_pCurrentEnemy->GetPosition( oEnemyPos );
		//if( GetDistanceTo2dPoint( oEnemyPos ) > m_fDestinationDeltaRadius)
		if (!m_bArriveAtDestination)
		{
			SetDestination( oEnemyPos );
			Run();
		}
		else
			m_eFightState = eArrivedToEnemy;
		break;
	case eArrivedToEnemy:
		m_eFightState = ePreparingForNextAttack;
		TurnFaceToDestination();
		break;
	case eBeginLaunchAttack:
		MainHit();
		m_eFightState = eLaunchingAttack;
		break;
	case eLaunchingAttack:
		break;
	case eEndLaunchAttack:
		if( m_pCurrentEnemy->GetLife() <= 0 )
			m_eFightState = eEndFight;
		else
			m_eFightState = eBeginPrepareForNextAttack;
		break;
	case eBeginPrepareForNextAttack:
		Guard();
		m_nBeginWaitTimeBeforeNextAttack = m_oTimeManager.GetCurrentTimeInMillisecond();
		m_eFightState = ePreparingForNextAttack;
		break;
	case ePreparingForNextAttack:
		m_pCurrentEnemy->GetPosition( oEnemyPos );
		distance = GetDistanceTo2dPoint(oEnemyPos);
		if(distance > g_fArrivedAtDestinationDistance)
			m_eFightState = eBeginGoToEnemy;
		else
		{
			FaceTo(oEnemyPos);
			m_nCurrentWaitTimeBeforeNextAttack = m_oTimeManager.GetCurrentTimeInMillisecond() - m_nBeginWaitTimeBeforeNextAttack;
			if( m_nCurrentWaitTimeBeforeNextAttack > m_nRecoveryTime )
				m_eFightState = eBeginLaunchAttack;
		}
		break;
	case eEndFight:
		if(m_pCurrentEnemy->GetLife() > 0)
			Stand();
		m_eFightState = eNoFight;
		break;
	default:
		break;
	}
	float fDistanceToEnemy = GetDistanceToEnemy();
	if (fDistanceToEnemy > m_fMinimumFleeDistance)
		Flee();
}

void IAEntity::UpdateTalkToState()
{
	try {
		CVector oInterlocuteurPosition;

		switch (m_eTalkToState)
		{
		case eNoTalkTo:
			break;
		case eBeginGotoInterlocutor:
			m_pCurrentInterlocutor->GetPosition(oInterlocuteurPosition);
			Goto(oInterlocuteurPosition, -1.f);
			m_eTalkToState = eGoingToInterlocutor;
			break;
		case eGoingToInterlocutor:
			m_pCurrentInterlocutor->GetPosition(oInterlocuteurPosition);
			if (GetDistanceTo2dPoint(oInterlocuteurPosition) > 100.f) {
				SetDestination(oInterlocuteurPosition);
				Run();
			}
			else {
				m_eTalkToState = eNoTalkTo;
				m_bArriveAtDestination = true;
				Stand();
				OpenTopicWindow();
			}
			break;
		default:
			break;
		}
	}
	catch (CEException& e)
	{
		m_eTalkToState = eNoTalkTo;
		m_bArriveAtDestination = true;
		throw e;
	}
}

void IAEntity::OnReceiveHit( IFighterEntity* pAgressor )
{
	if (m_bFightingIAEnabled) {
		SetFightMode(true);
		m_pCurrentEnemy = pAgressor;
		m_eFightState = IAEntity::eBeginHitReceived;
		GetCurrentAnimation()->AddCallback([this](IAnimation::TEvent e)
		{
			switch (e)
			{
			case IAnimation::eBeginRewind:
				GetCurrentAnimation()->RemoveAllCallback();
				if (m_eFightState == IAEntity::eReceivingHit)
					m_eFightState = IAEntity::eBeginPrepareForNextAttack;
				else if (GetLife() > 0)
					Stand();
				break;
			}
		});
	}
}

void IAEntity::OnEndHitAnimation()
{
	if( m_eFightState == IAEntity::eLaunchingAttack )
		m_eFightState = IAEntity::eEndLaunchAttack;
}

void IAEntity::FaceTo(const CVector& target)
{
	m_oDestination = CVector(target.m_x, 0.f, target.m_z);
	m_fAngleRemaining = GetDestinationAngleRemaining();
	m_bFaceToTarget = false;
}

void IAEntity::UpdateFaceTo()
{
	if (!m_bFaceToTarget)
	{
		if (m_fAngleRemaining > 1)
		{
			if (m_fAngleRemaining > fRotateSpeed || m_fAngleRemaining < -fRotateSpeed)
			{
				float fDelta = m_fAngleRemaining > 0 ? -fRotateSpeed : fRotateSpeed;
				m_fAngleRemaining = m_fAngleRemaining + fDelta;
				Turn(-fDelta);
			}
			else
				m_fAngleRemaining = GetDestinationAngleRemaining();
		}
		else {
			Turn(GetDestinationAngleRemaining());
			m_bFaceToTarget = true;
		}
	}
}

void IAEntity::Goto( const CVector& oDestination, float fSpeed )
{
	m_vCurrentPath.clear();
	CVector oPos;
	GetPosition( oPos );
	
	try {
		ComputePathFind2D(oPos, oDestination, m_vCurrentPath);
	}
	catch (CEException& e) {
		m_vCurrentPath.push_back(oDestination);
		CLogger::Log() << e.what();
	}

	if (!m_vCurrentPath.empty()) {
		if (m_vCurrentPath.size() > 1)
			m_fDestinationDeltaRadius = g_fArrivedAtDestinationDistance;
		else
			m_fDestinationDeltaRadius = g_fArrivedAtDestinationDistance;
		m_nCurrentPathPointNumber = 0;
		m_oDestination = m_vCurrentPath[m_nCurrentPathPointNumber];
		m_fAngleRemaining = GetDestinationAngleRemaining();

		//LookAt( m_fAngleRemaining );
		Run();
		m_bArriveAtDestination = false;
	}
}

float IAEntity::GetDistanceToEnemy()
{
	CVector oEnemyPos;
	m_pCurrentEnemy->GetPosition(oEnemyPos);
	float fDistanceToEnemy = GetDistanceTo2dPoint(oEnemyPos);
	return fDistanceToEnemy;
}

void IAEntity::UpdateFlee()
{	
	float fDistanceToEnemy = GetDistanceToEnemy();
	if (fDistanceToEnemy < m_fMinimumFleeDistance)
		Attack(m_pCurrentEnemy);
	else {
		if (abs(m_fAngleRemaining) > abs(fRotateSpeed))
		{
			const float epsilon = 1.0;
			if (abs(m_fAngleRemaining) > epsilon) {
				float fDelta = m_fAngleRemaining > 0 ? -fRotateSpeed : fRotateSpeed;
				if (abs(m_fAngleRemaining) > abs(fDelta)) {
					Turn(fDelta);
					m_fAngleRemaining = m_fAngleRemaining + fDelta;
				}
				else if (abs(m_fAngleRemaining) > epsilon) {
					Turn(-m_fAngleRemaining);
					m_fAngleRemaining = 0;
				}
			}
		}
		else {
			CVector oEnemyPos;
			m_pCurrentEnemy->GetPosition(oEnemyPos);
			m_vCurrentPath.clear();
			SetDestination(oEnemyPos);
			m_fAngleRemaining = GetFleeAngleRemaining();
		}
	}
}

void IAEntity::UpdateGoto()
{
	if( !m_bArriveAtDestination )
	{
		float fDistance = GetDistanceTo2dPoint( m_oDestination );
		if( fDistance > 200 )
		{			
			if( m_fAngleRemaining > fRotateSpeed || m_fAngleRemaining < -fRotateSpeed )
			{
				float fDelta = m_fAngleRemaining > 0 ? -fRotateSpeed : fRotateSpeed;
				m_fAngleRemaining = m_fAngleRemaining + fDelta;
				Turn( -fDelta );
			}
			else
				m_fAngleRemaining = GetDestinationAngleRemaining();
		}
		else
			Turn( GetDestinationAngleRemaining() );
		
		if( fDistance < m_fDestinationDeltaRadius )
		{
			if( m_nCurrentPathPointNumber >= m_vCurrentPath.size() - 1 )
			{
				m_bArriveAtDestination = true;
				if(!GetFightMode())
					Stand();
				m_vCurrentPath.clear();
			}
			else
			{
				m_nCurrentPathPointNumber++;
				m_oDestination = m_vCurrentPath[ m_nCurrentPathPointNumber ];
			}
		}
	}
}

void IAEntity::Update()
{
	if (m_eFightState == eNoFight && m_pCurrentEnemy) {
		float fDistanceToEnemy = GetDistanceToEnemy();
		if (fDistanceToEnemy < m_fDangerZone) {
			Flee();
		}
	}
	else if (m_eFightState == eFlee)
		UpdateFlee();
	else if ( (m_eFightState != eNoFight) && (GetLife() > 0) ) {
		UpdateFaceTo();
		UpdateGoto();
		UpdateFightState();
	}
	else if (m_eTalkToState != eNoTalkTo) {
		UpdateGoto();
		UpdateTalkToState();
	}
	else {
		UpdateGoto();
	}
}

void IAEntity::SetDestination( const CVector& oDestination )
{
	m_oDestination = oDestination;
	m_bArriveAtDestination = false;
	if( m_vCurrentPath.size() == 0 )
		m_vCurrentPath.push_back(m_oDestination);
}

float IAEntity::GetDestinationAngleRemaining()
{
	CVector v( 0, 0, 1, 1 ), oThisPosition, oTempPosition( m_oDestination.m_x, 0, m_oDestination.m_z );
	CVector oBefore = GetWorldTM().GetRotation() * v;
	GetPosition( oThisPosition );
	oThisPosition = CVector( oThisPosition.m_x, 0, oThisPosition.m_z );
	CVector oDirection = oTempPosition - oThisPosition;
	if (oDirection == CVector(0, 0, 0)) {
		return 0.f;
	}
	return m_fForceDeltaRotation + GetAngleBetween2Vectors(oBefore, oDirection);
}

float IAEntity::GetFleeAngleRemaining()
{
	return 180.f - GetDestinationAngleRemaining();
}

float IAEntity::GetAngleBetween2Vectors(CVector& v1, CVector& v2)
{
	float n = (v1.Norm() * v2.Norm());
	float cosAlpha = 0;
	if (n != 0)
		cosAlpha = (v1 * v2) / n;
	if (cosAlpha > 1.f) cosAlpha = 1.f;
	else if (cosAlpha < -1.f) cosAlpha = -1.f;
	float alpha = acosf(cosAlpha) * 180.f / 3.1415927f;
	CVector up = (v1 ^ v2) / n;
	if (up.m_y < 0)
		alpha = -alpha;
	return alpha;
}

bool IAEntity::IsArrivedAtDestination()
{
	return m_bArriveAtDestination;
}

void IAEntity::OnCollision( IAEntity* pEntity )
{
	IAEntity* pHuman = static_cast< IAEntity* >( pEntity );
	pHuman->m_bArriveAtDestination = true;
	pHuman->Stand();
}

void IAEntity::TurnFaceToDestination()
{ 
	Turn( GetDestinationAngleRemaining() ); 
}

void IAEntity::Attack(IFighterEntityInterface* pEntity)
{
	IFighterEntity* pEnemy = dynamic_cast<IFighterEntity*>(pEntity);
	if (pEnemy)
		Attack(pEnemy);
}

void IAEntity::TalkTo(IFighterEntityInterface* pEntity, TalkToCallback callback)
{
	if (pEntity->GetLife() > 0) {
		m_pCurrentInterlocutor = pEntity;
		m_eTalkToState = eBeginGotoInterlocutor;
		m_oTalkToCallback.first = callback;
		m_oTalkToCallback.second = pEntity;
	}
}

void IAEntity::Attack(IFighterEntity* pEntity)
{
	SetFightMode(true);
	m_pCurrentEnemy = pEntity;
	m_eFightState = eBeginGoToEnemy;
}

void IAEntity::Flee()
{
	m_eFightState = eFlee;
	Run();
}
