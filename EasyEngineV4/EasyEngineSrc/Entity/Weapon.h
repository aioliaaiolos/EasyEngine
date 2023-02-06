#pragma once

#include "item.h"

class CWeapon : public CItem
{
public:
	enum Type 
	{
		eNone = -1,
		eRightHand = 0,
		eLeftHand
	};

	CWeapon(EEInterface& oInterface, string sID, CItem::Type wearType, Type attackType, string sModelName, string sPreviewPath);
	void			SetAttackType(Type type);
	void			Load() override;
	void			LinkToHand(INode* pHand);
	void			Wear() override;
	static Type		GetAttackTypeFromString(string sAttackType);
	

private:
	Type						m_eAttackType;
	INode*						m_pDummyWear = nullptr;
	INode*						m_pDummyHandle = nullptr;
	INode*						m_pModelEntity = nullptr;
	CMatrix						m_oModelLocalMatrixInHandleBase;
	CMatrix						m_oModelLocalMatrixInWearBase;

	static map<string, Type>	s_mAttackType;
};