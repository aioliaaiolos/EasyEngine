#include "Weapon.h"
#include "Bone.h"
#include "IGeometry.h"
#include "BoxEntity.h"
#include "Interface.h"

map<string, CWeapon::Type> CWeapon::s_mAttackType = map<string, CWeapon::Type>
{ 
	{ "", CWeapon::Type::eNone},
	{ "Left-Hand", CWeapon::Type::eLeftHand},
	{ "Right-Hand", CWeapon::Type::eRightHand}
};

CWeapon::CWeapon(EEInterface& oInterface, string sID, CItem::Type wearType, Type attackType, string sModelName, string sPreviewPath):
	CItem(oInterface, sID, CItem::TClass::eWeapon, wearType, sModelName, sPreviewPath),
	m_oGeometryManager(static_cast<IGeometryManager&>(*m_oInterface.GetPlugin("GeometryManager")))
{
}

CWeapon::CWeapon(EEInterface& oInterface, CItem& item, CWeapon::Type eAttackType) :
	CWeapon(oInterface, item.GetIDStr(), item.m_eType, eAttackType, item.m_sModelName, item.GetPreviewPath())
{
}

void CWeapon::SetAttackType(Type type)
{
	m_eAttackType = type;
}

void CWeapon::Load()
{
	CItem::Load();
	m_pDummyWear = m_pSkeletonRoot;
	for (int i = 0; i < m_pSkeletonRoot->GetChildCount(); i++) {
		INode* pChild = m_pSkeletonRoot->GetChild(i);
		IBone* pBone = dynamic_cast<IBone*>(pChild);
		if (pBone)
			m_pDummyHandle = pBone;
	}
	if (m_pDummyHandle) {
		m_pDummyHandle->Unlink();
		m_pDummyHandle->GetWorldMatrix().GetInverse(m_oModelLocalMatrixInHandleBase);
	}
	m_pDummyWear->GetWorldMatrix().GetInverse(m_oModelLocalMatrixInWearBase);

	// box
	/*
	IBoxEntity* pWeaponBoxEntity = new CBoxEntity(m_oInterface, *static_cast<IBox*>(m_pModel->GetBoundingGeometry()));
	pWeaponBoxEntity->SetName("WeaponBox");
	pWeaponBoxEntity->Link(m_pModel);
	m_pModel->Update();*/
}

void CWeapon::LinkToHand(INode* pHand)
{
	m_pModel->Unlink();
	m_pDummyHandle->Unlink();
	CMatrix id;
	m_pModel->SetLocalMatrix(m_oModelLocalMatrixInHandleBase);
	m_pModel->Link(m_pDummyHandle);
	m_pDummyHandle->SetLocalMatrix(id);
	m_pDummyHandle->Link(pHand);
	pHand->Update();
}

void CWeapon::Wear()
{
	if (m_pModel) {
		m_pDummyHandle->Unlink();
		m_pModel->Unlink();
		m_pModel->Link(m_pDummyWear);
		m_pModel->SetLocalMatrix(m_oModelLocalMatrixInWearBase);
	}
	CItem::Wear();
	CCharacter* pCharacter = dynamic_cast<CCharacter*>(m_pOwner);
	pCharacter->SetCurrentWeapon(this);
}

void CWeapon::Update()
{
	CItem::Update();
}

CWeapon::Type CWeapon::GetAttackTypeFromString(string sAttackType)
{
	map<string, CWeapon::Type>::iterator itType = s_mAttackType.find(sAttackType);
	if (itType != s_mAttackType.end()) {
		return itType->second;
	}
	return CWeapon::Type::eNone;
}

