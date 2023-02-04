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
#include "Utils2/Logger.h"

CNPCEntity::CNPCEntity(EEInterface& oInterface, string sFileName, string sID):
CCharacter(oInterface, sFileName, sID),
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
	return CCharacter::GetLife(); 
}

void CNPCEntity::SetLife( int nLife )
{ 
	CCharacter::SetLife( nLife ); 
}

void CNPCEntity::IncreaseLife( int nLife )
{ 
	CCharacter::IncreaseLife( nLife ); 
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
	CCharacter::Run( true ); 
}

void CNPCEntity::MoveToGuard()
{
	CCharacter::MoveToGuard();
}

void CNPCEntity::Guard()
{
	CCharacter::Guard();
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
	CCharacter::ReceiveHit( pEnemy );
}

void CNPCEntity::Update()
{
	CCharacter::Update();
	IAEntity::Update();	
}

IAnimation* CNPCEntity::GetCurrentAnimation()
{
	return CCharacter::GetCurrentAnimation();
}

CMatrix& CNPCEntity::GetWorldTM()
{
	return CCharacter::GetWorldTM();
}

IFighterEntity* CNPCEntity::GetFirstEnemy()
{
	return CCharacter::GetFirstEnemy();
}

IFighterEntity* CNPCEntity::GetNextEnemy()
{
	return CCharacter::GetNextEnemy();
}

void CNPCEntity::GetPosition( CVector& v )
{
	CCharacter::GetPosition( v );
}

void CNPCEntity::ReceiveHit( IFighterEntity* pEnemy )
{
	CCharacter::ReceiveHit(pEnemy);
}

void CNPCEntity::Stand()
{
	CCharacter::Stand();
}

void CNPCEntity::Die()
{
	CCharacter::Die();
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

void CNPCEntity::ComputePathFind2D( const CVector& oOrigin, const CVector& oDestination, vector<CVector>& vPoints)
{
	ComputePathFind2DAStar(oOrigin, oDestination, vPoints);
}

void CNPCEntity::OnTopicWindowClosed(IGUIWindow* pWindow, IBaseObject* pThisEntity)
{
	CNPCEntity* pThisNPCEntity = dynamic_cast<CNPCEntity*>(pThisEntity);
	pThisNPCEntity->m_oTalkToCallback.first(pThisNPCEntity, pThisNPCEntity->m_oTalkToCallback.second);
	pThisNPCEntity->m_oGUIManager.GetTopicsWindow()->SetCloseWindowCallback(nullptr, nullptr);
}

void CNPCEntity::OpenTopicWindow()
{
	m_oGUIManager.GetTopicsWindow()->SetCloseWindowCallback(OnTopicWindowClosed, this);
	m_oGUIManager.GetTopicsWindow()->SetSpeakerId(GetIDStr());
	m_oGUIManager.AddWindow(m_oGUIManager.GetTopicsWindow());
}

void CNPCEntity::UpdateGoto()
{
	IAEntity::UpdateGoto();
	if (IsArrivedAtDestination() && m_pBackupBoundingGeometry) {
		m_pBoundingGeometry = m_pBackupBoundingGeometry;
	}
	if (!IsArrivedAtDestination() && m_bCollideOnObstacle && (m_fAngleRemaining != 0.f)) {
		if (m_fAngleRemaining > 0)
			m_fForceDeltaRotation++;
		else if (m_fAngleRemaining < 0)
			m_fForceDeltaRotation--;
	}
	else {
		if(m_fForceDeltaRotation > 0)
			m_fForceDeltaRotation--;
		else if(m_fForceDeltaRotation < 0)
			m_fForceDeltaRotation++;
	}
}

void CNPCEntity::ComputePathFind2DAStar(const CVector& oOrigin, const CVector& oDestination, vector<CVector>& vPoints)
{
	int originx, originy, destinationx, destinationy;
	CEntity* pParent = dynamic_cast<CEntity*>(GetParent());

	// convert gobal to local coordinates
	CMatrix oParentInv;
	pParent->GetWorldMatrix().GetInverse(oParentInv);
	CVector oLocalOrigin = oParentInv * oOrigin;
	CVector oLocalDestination = oParentInv * oDestination;
	m_pCollisionMap->GetCellCoordFromPosition(oLocalOrigin.m_x, oLocalOrigin.m_z, originx, originy);
	m_pCollisionMap->GetCellCoordFromPosition(oLocalDestination.m_x, oLocalDestination.m_z, destinationx, destinationy);
	
	IGrid* pGrid = pParent->GetCollisionGrid();
	if (!pGrid) {
		string sSceneName = m_pScene->GetIDStr();
		throw CEException(string("Error : no collision map found for scene '") + sSceneName);
	}
	pGrid->SetDepart(originx, originy);
	pGrid->SetDestination(destinationx, destinationy);
	bool bPathFound = false;
	try {
		bPathFound = m_oPathFinder.FindPath(pGrid);
	}
	catch (CEException& e) {
		CLogger::Log() << "Error : ComputePathFind2DAStar(" << oOrigin << "), " << oDestination << ") failed";
	}
	if (bPathFound) {
		vector<IGrid::ICell*> path;
		pGrid->GetPath(path);

		path.erase(path.begin());
		for (vector<IGrid::ICell*>::iterator it = path.begin(); it != path.end(); it++) {
			IGrid::ICell* pCell = (*it);
			int r, c;
			float x, y;
			pCell->GetCoordinates(r, c);
			m_pCollisionMap->GetPositionFromCellCoord(r, c, x, y);
			vPoints.push_back(CVector(x, 0, y));
		}
		if (!vPoints.empty())
			vPoints.pop_back();
	}
	vPoints.push_back(oLocalDestination);
	pGrid->ResetAllExceptObstacles();

	// convert local path to world
	for (CVector& point : vPoints) {
		point = pParent->GetWorldMatrix() * point;
	}
}

INode* CNPCEntity::GetParent()
{
	return CNode::GetParent();
}