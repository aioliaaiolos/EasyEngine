#include "Weapon.h"
#include "Bone.h"

map<string, CWeapon::Type> CWeapon::s_mAttackType = map<string, CWeapon::Type>
{ 
	{ "", CWeapon::Type::eNone},
	{ "Left-Hand", CWeapon::Type::eLeftHand},
	{ "Right-Hand", CWeapon::Type::eRightHand}
};

CWeapon::CWeapon(EEInterface& oInterface, string sID, CItem::Type wearType, Type attackType, string sModelName, string sPreviewPath):
	CItem(oInterface, sID, CItem::TClass::eWeapon, wearType, sModelName, sPreviewPath)
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
		if (pBone) {
			m_pDummyHandle = pBone;
		}
		else {
			m_pModelEntity = pChild;
		}
	}
	m_pDummyHandle->Unlink();
	m_oModelLocalMatrixInHandleBase = m_pDummyHandle->GetWorldMatrix();
	m_oModelLocalMatrixInWearBase = m_pDummyWear->GetWorldMatrix();
}

void CWeapon::LinkToHand(INode* pHand)
{
	m_pModelEntity->Unlink();
	m_pDummyHandle->Unlink();
	CMatrix id;
	CMatrix oHandleInverse;
	m_oModelLocalMatrixInHandleBase.GetInverse(oHandleInverse);
	m_pModelEntity->SetLocalMatrix(oHandleInverse);
	m_pModelEntity->Link(m_pDummyHandle);
	m_pDummyHandle->SetLocalMatrix(id);
	m_pDummyHandle->Link(pHand);
	pHand->Update();
}

void CWeapon::Wear()
{
	if (m_pModelEntity) {
		m_pDummyHandle->Unlink();
		m_pModelEntity->Unlink();
		m_pModelEntity->Link(m_pDummyWear);
		CMatrix oModelLocalMatrix;
		m_oModelLocalMatrixInWearBase.GetInverse(oModelLocalMatrix);
		m_pModelEntity->SetLocalMatrix(oModelLocalMatrix);
	}
	CItem::Wear();
	CCharacter* pCharacter = dynamic_cast<CCharacter*>(m_pOwner);
	pCharacter->SetCurrentWeapon(this);
}

CWeapon::Type CWeapon::GetAttackTypeFromString(string sAttackType)
{
	map<string, CWeapon::Type>::iterator itType = s_mAttackType.find(sAttackType);
	if (itType != s_mAttackType.end()) {
		return itType->second;
	}
	return CWeapon::Type::eNone;
}

