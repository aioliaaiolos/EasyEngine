#include "LinkedCamera.h"
#include "MobileEntity.h"

CLinkedCamera::CLinkedCamera(EEInterface& oInterface, float fFov):
CCamera(oInterface, fFov)
{
	m_sEntityID = "Linked camera";
	m_sTypeName = "LinkedCamera";
	m_sName = "Linked camera";
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

void CLinkedCamera::Link(INode* pNode)
{
	CCharacter* pPerso = dynamic_cast<CCharacter*>(pNode);
	if (pPerso) {
		CNode::Link(pPerso);
		m_pHeadNode = pPerso->GetSkeletonRoot()->GetChildBoneByName("Tete");
		if(!m_pHeadNode)
			m_pHeadNode = pPerso->GetSkeletonRoot()->GetChildBoneByName("mixamorig:HeadTop_End");
		CMatrix heandTM;
		m_pHeadNode->GetWorldMatrix(heandTM);
		SetWorldMatrix(heandTM);
		Yaw(-90.f);
		Pitch(10.f);
		LocalTranslate(0.f, 16.f, -40.f);
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
}

void CLinkedCamera::GetEntityInfos(ILoader::CObjectInfos*& pInfos)
{
	
}