#include "FighterEntity.h"
#include "IGeometry.h"
#include "ICollisionManager.h"

IFighterEntity::IFighterEntity():
m_bHitEnemy( false ),
m_nLife(1000)
{
}


void IFighterEntity::OnHit( IFighterEntity* pAgressor, string sHitBoneName )
{

	m_sCurrentHitBoneName = sHitBoneName;
	pAgressor->GetCurrentAnimation()->AddCallback([this](IAnimation::TEvent e)
	{
		bool bEndAnimation = false;
		if (e == IAnimation::eAfterUpdate && !m_bHitEnemy)
		{
			IFighterEntity* pEnemy = GetFirstEnemy();
			while (pEnemy)
			{
				if (IsHitIntersectEnemySphere(pEnemy))
				{
					if (IsHitIntersectEnemyBox(pEnemy))
					{
						pEnemy->ReceiveHit(this);
						pEnemy->OnReceiveHit(this);
						m_bHitEnemy = true;
					}
				}
				pEnemy = GetNextEnemy();
			}
		}
		else if (e == IAnimation::eBeginRewind)
		{
			GetCurrentAnimation()->RemoveAllCallback();
			m_bHitEnemy = false;
			OnEndHitAnimation();
		}
	});
}

void IFighterEntity::MainHit()
{
	if (GetLife() > 0) {
		PlayHitAnimation();
		OnHit(this, GetAttackBoneName());
	}
}

void IFighterEntity::SecondaryHit()
{
	if (GetLife() > 0) {
		PlaySecondaryHitAnimation();
		OnHit(this, GetAttackBoneName());
	}
}

void IFighterEntity::OnEndHitAnimation()
{
	if (m_nLife > 0)
		Stand();
}

void IFighterEntity::OnReceiveHit( IFighterEntity* pEnemy )
{
	GetCurrentAnimation()->AddCallback([this](IAnimation::TEvent e)
	{
		switch (e)
		{
		case IAnimation::eBeginRewind:
			GetCurrentAnimation()->RemoveAllCallback();
			if (GetLife() > 0)
				Stand();
			break;
		}
	});
}

void IFighterEntity::ReceiveHit(IFighterEntity* pEnemy)
{
	IncreaseLife(-100);
	if(GetLife() > 0)	
		PlayReceiveHit();
}


int IFighterEntity::GetLife()
{
	return m_nLife;
}

void IFighterEntity::SetLife(int nLife)
{
	m_nLife = nLife;
	if (m_nLife <= 0)
		Die();
}

void IFighterEntity::IncreaseLife(int nLife)
{
	m_nLife += nLife;
	if (m_nLife <= 0)
		Die();
}


bool IFighterEntity::IsHitIntersectEnemySphere( IFighterEntity* pEnemy )
{
	ISphere* pBoneSphere = GetBoneSphere( m_sCurrentHitBoneName );
	CVector oEnemyWorldPosition;
	pEnemy->GetPosition( oEnemyWorldPosition );
	float fBoneDistance = ( pBoneSphere->GetCenter() - oEnemyWorldPosition ).Norm();
	return fBoneDistance < ( pBoneSphere->GetRadius() + pEnemy->GetBoundingSphereRadius());
}

bool IFighterEntity::IsHitIntersectEnemyBox( IFighterEntity* pEnemy )
{
	IBox* pEnemyBox = pEnemy->GetBoundingBox();
	pEnemyBox->SetTM( pEnemy->GetWorldTM() );
	ISphere* pBoneSphere = GetBoneSphere( m_sCurrentHitBoneName );
	return GetCollisionManager().IsIntersection( *pEnemyBox, *pBoneSphere );
}