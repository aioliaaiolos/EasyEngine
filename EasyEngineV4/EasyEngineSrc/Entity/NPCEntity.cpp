#include "Interface.h"
#include "NPCEntity.h"
#include "IGeometry.h"
#include "LineEntity.h"
#include "ICollisionManager.h"
#include "IGUIManager.h"
#include "CylinderEntity.h"
#include "SphereEntity.h"
#include "Scene.h"
#include "EntityManager.h"

CNPCEntity::CNPCEntity(EEInterface& oInterface, string sFileName, string sID):
CMobileEntity(oInterface, sFileName, sID),
m_oPathFinder(static_cast<IPathFinder&>(*oInterface.GetPlugin("PathFinder"))),
m_oGUIManager(static_cast<IGUIManager&>(*oInterface.GetPlugin("GUIManager"))),
m_pGotoBox(nullptr),
m_pBackupBoundingGeometry(nullptr)
{
	m_sTypeName = "NPC";
	if( !m_pfnCollisionCallback )
		m_pfnCollisionCallback = OnCollision;
	
	m_pGotoBox = dynamic_cast<IBox*>(m_pBoundingGeometry->Duplicate());
	m_pGotoBox->SetX(m_pGotoBox->GetDimension().m_x - m_fBBoxReduction);
	m_pGotoBox->SetMinPoint(CVector(m_pGotoBox->GetMinPoint().m_x + m_fBBoxReduction / 2.f, m_pGotoBox->GetMinPoint().m_y, m_pGotoBox->GetMinPoint().m_z));
}

int CNPCEntity::GetLife()
{ 
	return CMobileEntity::GetLife(); 
}

void CNPCEntity::SetLife( int nLife )
{ 
	CMobileEntity::SetLife( nLife ); 
}

void CNPCEntity::IncreaseLife( int nLife )
{ 
	CMobileEntity::IncreaseLife( nLife ); 
}

float CNPCEntity::GetDistanceTo2dPoint( const CVector& oPosition )
{
	CVector oThisPosition;
	GetWorldPosition( oThisPosition );
	CVector o2DThisPosition = CVector( oThisPosition.m_x, 0, oThisPosition.m_z );
	CVector o2DPosition = CVector( oPosition.m_x, 0, oPosition.m_z );
	return ( o2DPosition - o2DThisPosition ).Norm();
}

void CNPCEntity::Run()
{ 
	CMobileEntity::Run( true ); 
}

void CNPCEntity::MoveToGuard()
{
	CMobileEntity::MoveToGuard();
}

void CNPCEntity::Guard()
{
	CMobileEntity::Guard();
}

void CNPCEntity::LookAt( float alpha )
{
	float fMustEyeRotH = 0.f;
	float fMustNeckRotH = 0.f;
	//	const float fEyeRotMult = 3.6f;
	const float fEyeRotMult = 4.3f;
	if( ( alpha < m_fMaxEyeRotationH * fEyeRotMult ) && ( alpha > -m_fMaxEyeRotationH * fEyeRotMult  ) )
	{
		fMustEyeRotH = alpha / fEyeRotMult - m_fEyesRotH;
		m_fEyesRotH += fMustEyeRotH;
	}
	else
	{
		fMustEyeRotH = m_fMaxEyeRotationH - m_fEyesRotH;
		if( alpha < 0 )
			fMustEyeRotH = -fMustEyeRotH;
		m_fEyesRotH += fMustEyeRotH;
		alpha -= m_fEyesRotH * fEyeRotMult;
		if( alpha < m_fMaxNeckRotationH - m_fNeckRotH )
			fMustNeckRotH = alpha - m_fNeckRotH;
		else
			fMustNeckRotH = m_fMaxNeckRotationH - m_fNeckRotH;
	}
	m_fNeckRotH += fMustNeckRotH;
	TurnEyesH( fMustEyeRotH );
	TurnNeckH( fMustNeckRotH );
}

void CNPCEntity::ReceiveHit( IAEntity* pEnemy )
{
	CMobileEntity::ReceiveHit( pEnemy );
}

void CNPCEntity::Update()
{
	CMobileEntity::Update();
	IAEntity::Update();	
}

IAnimation* CNPCEntity::GetCurrentAnimation()
{
	return CMobileEntity::GetCurrentAnimation();
}

CMatrix& CNPCEntity::GetWorldTM()
{
	return CMobileEntity::GetWorldTM();
}

IFighterEntity* CNPCEntity::GetFirstEnemy()
{
	return CMobileEntity::GetFirstEnemy();
}

IFighterEntity* CNPCEntity::GetNextEnemy()
{
	return CMobileEntity::GetNextEnemy();
}

void CNPCEntity::GetPosition( CVector& v )
{
	CMobileEntity::GetPosition( v );
}

void CNPCEntity::ReceiveHit( IFighterEntity* pEnemy )
{
	CMobileEntity::ReceiveHit(pEnemy);
}

void CNPCEntity::Stand()
{
	CMobileEntity::Stand();
}

void CNPCEntity::Die()
{
	CMobileEntity::Die();
}

void CNPCEntity::Turn( float fAngle )
{
	Yaw( fAngle );
}

void CNPCEntity::OnCollision( CEntity* pThis, vector<INode*> entities)
{
	CNPCEntity* pNPC = static_cast< CNPCEntity* >(pThis);
	IAEntity::OnCollision( pNPC );
}

void CNPCEntity::Goto( const CVector& oPosition, float fSpeed )
{
	IAEntity::Goto( oPosition, fSpeed );
	m_pBackupBoundingGeometry = m_pBoundingGeometry;
	m_pBoundingGeometry = m_pGotoBox;
}

IBox* CNPCEntity::GetFirstCollideBox()
{
	IEntity* pEntity = m_pEntityManager->GetFirstCollideEntity();
	if( pEntity == this )
		pEntity = m_pEntityManager->GetNextCollideEntity();
	if( pEntity )
	{
		IMesh* pMesh = static_cast< IMesh* >( pEntity->GetRessource() );
		string sAnimationName;
		if( pEntity->GetCurrentAnimation() )
		{
			pEntity->GetCurrentAnimation()->GetName( sAnimationName );
			return pMesh->GetAnimationBBox( sAnimationName );
		}
		else
		{
			IBox* pBox = m_oGeometryManager.CreateBox( *pMesh->GetBBox() );
			pBox->SetTM( pEntity->GetWorldMatrix() );
			return pBox;
		}
	}
	return NULL;
}

IBox* CNPCEntity::GetNextCollideBox()
{
	IEntity* pEntity = m_pEntityManager->GetNextCollideEntity();
	IBox* pBoxRet = NULL;
	if( pEntity )
	{
		if( pEntity == this )
			pEntity = m_pEntityManager->GetNextCollideEntity();
		IMesh* pMesh = static_cast< IMesh* >( pEntity->GetRessource() );
		if( pEntity->GetCurrentAnimation() )
		{
			string sAnimationName;
			pEntity->GetCurrentAnimation()->GetName( sAnimationName );
			pBoxRet = pMesh->GetAnimationBBox( sAnimationName );
		}
		else
		{
			IBox* pBox = m_oGeometryManager.CreateBox( *pMesh->GetBBox() );
			pBox->SetTM( pEntity->GetWorldMatrix() );
			pBoxRet = pBox;
		}
	}
	return pBoxRet;
}

void CNPCEntity::ComputePathFind2D( const CVector2D& oOrigin, const CVector2D& oDestination, vector< CVector2D >& vPoints)
{
	ComputePathFind2DAStar(oOrigin, oDestination, vPoints, m_pScene->GetCellSize());
}

void CNPCEntity::OnTopicWindowClosed(IGUIWindow* pWindow, IObject* pThisEntity)
{
	CNPCEntity* pThisNPCEntity = dynamic_cast<CNPCEntity*>(pThisEntity);
	pThisNPCEntity->m_oTalkToCallback.first(pThisNPCEntity, pThisNPCEntity->m_oTalkToCallback.second);
	pThisNPCEntity->m_oGUIManager.GetTopicsWindow()->SetCloseWindowCallback(nullptr, nullptr);
}

void CNPCEntity::OpenTopicWindow()
{
	string sId;
	GetEntityName(sId);
	m_oGUIManager.GetTopicsWindow()->SetCloseWindowCallback(OnTopicWindowClosed, this);
	m_oGUIManager.GetTopicsWindow()->SetSpeakerId(sId);
	m_oGUIManager.AddWindow(m_oGUIManager.GetTopicsWindow());
}

void CNPCEntity::UpdateGoto()
{
	IAEntity::UpdateGoto();
	if (IsArrivedAtDestination() && m_pBackupBoundingGeometry) {
		m_pBoundingGeometry = m_pBackupBoundingGeometry;
	}
}

void CNPCEntity::ComputePathFind2DAStar(const CVector2D& oOrigin, const CVector2D& oDestination, vector< CVector2D >& vPoints, int nCellSize)
{
	int originx, originy, destinationx, destinationy;
	m_pCollisionMap->GetCellCoordFromPosition(oOrigin.m_x, oOrigin.m_y, originx, originy);
	m_pCollisionMap->GetCellCoordFromPosition(oDestination.m_x, oDestination.m_y, destinationx, destinationy);
	CEntity* pParent = dynamic_cast<CEntity*>(GetParent());
	IGrid* pGrid = pParent->GetCollisionGrid();
	if (!pGrid) {
		string sSceneName;
		m_pScene->GetEntityName(sSceneName);
		throw CEException(string("Error : no collision map found for scene '") + sSceneName);
	}
	pGrid->SetDepart(originx, originy);
	pGrid->SetDestination(destinationx, destinationy);
	if (m_oPathFinder.FindPath(pGrid)) {
		vector<IGrid::ICell*> path;
		pGrid->GetPath(path);

		path.erase(path.begin());
		for (vector<IGrid::ICell*>::iterator it = path.begin(); it != path.end(); it++) {
			IGrid::ICell* pCell = (*it);
			int r, c;
			float x, y;
			pCell->GetCoordinates(r, c);
			m_pCollisionMap->GetPositionFromCellCoord(r, c, x, y);
			vPoints.push_back(CVector2D(x, y));
		}
		if (!vPoints.empty())
			vPoints.pop_back();
	}
	vPoints.push_back(oDestination);
	pGrid->ResetAllExceptObstacles();
}