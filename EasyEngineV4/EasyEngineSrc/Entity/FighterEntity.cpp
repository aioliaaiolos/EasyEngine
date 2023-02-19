#include "FighterEntity.h"
#include "IGeometry.h"
#include "ICollisionManager.h"

IFighterEntity::IFighterEntity():
m_bHitEnemy( false ),
m_nLife(1000)
{
}

void IFighterEntity::OnHit()
{
	m_bHitEnemy = false;
	GetCurrentAnimation()->AddCallback([this](IAnimation::TEvent e)
	{
		bool bEndAnimation = false;
		if (e == IAnimation::eAfterUpdate && !m_bHitEnemy)
		{
			IFighterEntity* pEnemy = GetFirstEnemy();
			while (pEnemy)
			{
				if (IsHitIntersectEnemySphere(pEnemy))
				{
					if (m_oHitEnemySphereCallback)
						m_oHitEnemySphereCallback(pEnemy);
					if (IsHitIntersectEnemyBox(pEnemy))
					{
						if (m_oHitEnemyBoxCallback)
							m_oHitEnemyBoxCallback(pEnemy);
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
			OnEndHitAnimation();
		}
	});
}

void IFighterEntity::MainHit()
{
	if (GetFightMode() && GetLife() > 0) {
		PlayHitAnimation();
		OnHit();
	}
}

void IFighterEntity::SecondaryHit()
{
	if (GetLife() > 0) {
		PlaySecondaryHitAnimation();
		OnHit();
	}
}

void IFighterEntity::OnEndHitAnimation()
{

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
	IncreaseLife(-pEnemy->GetHitDamage());
	if(GetLife() > 0)	
		ReceiveHit();
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
	CVector oEnemyWorldPosition;
	pEnemy->GetPosition( oEnemyWorldPosition );	
	CVector oWeaponCenter;
	GetAttackGeometry()->GetCenter(oWeaponCenter);
	oWeaponCenter = GetWeaponTM() * oWeaponCenter;
	float fBoneDistance = (oWeaponCenter - oEnemyWorldPosition ).Norm();
	return fBoneDistance < (GetAttackGeometry()->GetRadius() + pEnemy->GetBoundingSphereRadius());
}

bool IFighterEntity::IsHitIntersectEnemyBox( IFighterEntity* pEnemy )
{
	IBox* pBox = static_cast<IBox*>(pEnemy->GetBoundingGeometry());
	pBox->SetTM(pEnemy->GetWorldTM());	
	GetAttackGeometry()->SetTM(GetWeaponTM());
	return pBox->IsIntersect(*GetAttackGeometry());
}

void IFighterEntity::SetHitEnemySphereCallback(THitEnemyCallback callback)
{
	m_oHitEnemySphereCallback = callback;
}

void IFighterEntity::SetHitEnemyBoxCallback(THitEnemyCallback callback)
{
	m_oHitEnemyBoxCallback = callback;
}