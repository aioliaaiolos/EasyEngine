#include "LinkedCamera.h"
#include "MobileEntity.h"
#include "EntityManager.h"

CLinkedCamera::CLinkedCamera(EEInterface& oInterface, float fFov):
CCamera(oInterface, fFov)
{
	m_sEntityID = "Linked camera";
	m_sTypeName = "LinkedCamera";
	m_sName = "Linked camera";

	m_pNearNode = new CNode;
	m_pFarNode = new CNode;
	m_pNearSphere = m_pEntityManager->CreateSphere(10);
	m_pFarSphere = m_pEntityManager->CreateSphere(10);
}

CLinkedCamera::~CLinkedCamera()
{
}

void CLinkedCamera::Move( float fOffsetYaw, float fOffsetPitch, float fOffsetRoll, float fAvanceOffet, float fLeftOffset, float fUpOffset )
{
	throw 1;
}

void CLinkedCamera::SetEntityName(string sName)
{
	m_sEntityID = sName;
}

void CLinkedCamera::GetEntityName(string& sName)
{
	sName = m_sEntityID;
}

void CLinkedCamera::Zoom(int value)
{
	float z = value * 20;
	LocalTranslate(0, 0, z);
}

IBone* CLinkedCamera::GetHeadNode(CCharacter* pPerso)
{
	IBone* pHeadNode = pPerso->GetSkeletonRoot()->GetChildBoneByName("Tete");
	if (!pHeadNode)
		pHeadNode = pPerso->GetSkeletonRoot()->GetChildBoneByName("mixamorig:HeadTop_End");
	return pHeadNode;
}

void CLinkedCamera::Link(INode* pNode)
{
	CCharacter* pPerso = dynamic_cast<CCharacter*>(pNode);
	if (pPerso) {
		static bool linkToHeadNode = false;
		if (linkToHeadNode) {
			m_pHeadNode = GetHeadNode(pPerso);
			CNode::Link(m_pHeadNode);
			CMatrix headTM;
			m_pHeadNode->GetLocalMatrix(headTM);

			Yaw(180.f);
			LocalTranslate(0.f, 16.f, -40.f);

			m_pNearNode->Link(pPerso);
			m_pFarNode->Link(pPerso);

			m_pNearNode->SetLocalMatrix(headTM);
			m_pFarNode->SetLocalMatrix(headTM);

			m_pNearSphere->Link(m_pNearNode);
			m_pNearSphere->SetLocalMatrix(headTM);

			m_pFarSphere->Link(m_pFarNode);
			m_pFarSphere->SetLocalMatrix(headTM);
		}
		else {
			CNode::Link(pPerso);
			m_pHeadNode = pPerso->GetSkeletonRoot()->GetChildBoneByName("Tete");
			if (!m_pHeadNode)
				m_pHeadNode = pPerso->GetSkeletonRoot()->GetChildBoneByName("mixamorig:HeadTop_End");
			CMatrix heandTM;
			m_pHeadNode->GetWorldMatrix(heandTM);
			SetWorldMatrix(heandTM);
			Yaw(-180.f);
			Pitch(10.f);
			LocalTranslate(0.f, 16.f, -40.f);
		}
	}
}

void CLinkedCamera::Update() 
{
	if (!m_bFreeze)
	{
		float lastHeight = m_oWorldMatrix.m_13;
		CNode::Update();
		float nextHeight = m_oWorldMatrix.m_13;
		float ascendSpeed = nextHeight - lastHeight;
		float maxAscendSpeed = ascendSpeed / 80.f;
		if (nextHeight - lastHeight > maxAscendSpeed)
			m_oWorldMatrix.m_13 = lastHeight + maxAscendSpeed;
	}
	if (m_bDisplayViewCone) {
		DisplayViewCone();
	}
}

void CLinkedCamera::DisplayViewCone()
{
	m_oRenderer.CullFace(0);
	CMatrix backupModelMatrix;
	m_oRenderer.GetModelMatrix(backupModelMatrix);

	double dHeight = 100.;
	/*
	CMatrix headMatrix, headMatrixTransformed;
	m_pHeadNode->GetWorldMatrix(headMatrix);	
	headMatrix = headMatrix * CMatrix::GetxRotation(-90);
	headMatrix = headMatrix * CMatrix::GetTranslation(0, -dHeight, 0);
	m_oRenderer.SetModelMatrix(headMatrix);
	*/
	m_oRenderer.SetModelMatrix(m_oWorldMatrix);
	m_oRenderer.DrawCylinder(50, 0, dHeight, 10, 10);
	m_oRenderer.CullFace(1);
	m_oRenderer.SetModelMatrix(backupModelMatrix);
}

void CLinkedCamera::GetEntityInfos(ILoader::CObjectInfos*& pInfos)
{
	
}