#include "FightSystem.h"
#include "IEntity.h"
#include "IGeometry.h"
#include "ICollisionManager.h"
#include "IAEntity.h"


CFightSystem::CFightSystem( IEntityManager& oEntityManager ):
m_oEntityManager( oEntityManager )
{
}

void CFightSystem::Update()
{
	IAEntity* pFighter = m_oEntityManager.GetFirstIAEntity();
	while( pFighter )
	{
		pFighter->UpdateFightState();
		pFighter = m_oEntityManager.GetNextIAEntity();
	}
}

void CFightSystem::OnHit( IAEntity* pAgressor, string sHitBoneName )
{
	pAgressor->m_sCurrentHitBoneName = sHitBoneName;
	pAgressor->GetCurrentAnimation()->AddCallback([this, pAgressor](IAnimation::TEvent e)
	{
		bool bEndAnimation = false;
		bool bHitEnemy = false;
		if (e == IAnimation::eAfterUpdate && !pAgressor->m_bHitEnemy)
		{
			IAEntity* pEnemy = m_oEntityManager.GetFirstIAEntity();
			while (pEnemy)
			{
				if (pAgressor == pEnemy)
				{
					pEnemy = m_oEntityManager.GetNextIAEntity();
					continue;
				}

				ISphere* pBoneSphere = pAgressor->GetBoneSphere(pAgressor->m_sCurrentHitBoneName);
				CVector oEnemyWorldPosition;
				pEnemy->GetPosition(oEnemyWorldPosition);
				float fFootDistance = (pBoneSphere->GetCenter() - oEnemyWorldPosition).Norm();
				if (fFootDistance < pBoneSphere->GetRadius() / 2.f + pEnemy->GetBoundingSphereRadius() / 2.f)
				{
					IMesh* pMesh = pEnemy->GetMesh();
					string sAnimationName;
					pEnemy->GetCurrentAnimation()->GetName(sAnimationName);
					IBox* pEnemyBox = pMesh->GetAnimationBBox(sAnimationName);
					pEnemyBox->SetTM(pEnemy->GetWorldTM());
					if (pAgressor->GetCollisionManager().IsIntersection(*pEnemyBox, *pBoneSphere))
					{
						pEnemy->ReceiveHit(pAgressor);
						pAgressor->m_bHitEnemy = true;
					}
				}
				pEnemy = m_oEntityManager.GetNextIAEntity();
			}
		}
		else if (e == IAnimation::eBeginRewind)
		{
			pAgressor->GetCurrentAnimation()->RemoveAllCallback();
			if (pAgressor->m_eFightState == IAEntity::eLaunchingAttack)
				pAgressor->m_eFightState = IAEntity::eEndLaunchAttack;
			pAgressor->Stand();
			pAgressor->m_bHitEnemy = false;
		}
	});
}

void CFightSystem::OnReceiveHit( IAEntity* pAssaulted, IAEntity* pAgressor )
{
	pAssaulted->m_pCurrentEnemy = pAgressor;
	if( pAssaulted->m_eFightState == IAEntity::eNoFight )
		pAssaulted->m_eFightState = IAEntity::eBeginHitReceived;
	else if( pAssaulted->m_eFightState == IAEntity::eLaunchingAttack )
		pAssaulted->m_eFightState = IAEntity::eEndLaunchAttack;
	int nCallbackIndex = pAssaulted->GetCurrentAnimation()->AddCallback([this, pAssaulted, nCallbackIndex](IAnimation::TEvent e)
	{
		switch (e)
		{
		case IAnimation::eBeginRewind:
			pAssaulted->GetCurrentAnimation()->RemoveCallback(nCallbackIndex);
			if (pAssaulted->m_eFightState == IAEntity::eReceivingHit)
				pAssaulted->m_eFightState = IAEntity::eBeginPrepareForNextAttack;
			else
				pAssaulted->Stand();
			break;
		}
	});
}