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
	void			Update() override;

	int				m_nDamage = 0;
	static Type		GetAttackTypeFromString(string sAttackType);
	

private:
	Type						m_eAttackType;
	INode*						m_pDummyWear = nullptr;
	INode*						m_pDummyHandle = nullptr;
	CMatrix						m_oModelLocalMatrixInHandleBase;
	CMatrix						m_oModelLocalMatrixInWearBase;
	IGeometryManager&			m_oGeometryManager;
	
	static map<string, Type>	s_mAttackType;
};