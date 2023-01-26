#pragma once

#include "MobileEntity.h"

class CBoxEntity : public CObject, public IBoxEntity
{
public:
	CBoxEntity(EEInterface& oInterface, IBox& oBox);

	void				Update();
	IBox&				GetBox();
	void				GetEntityName(string& sName);
	IGeometry*			GetBoundingGeometry();
	void				SetWeight(float fWeight) override;

protected:
	IBox&				m_oBox;
	string				m_sEntityID;
	CVector				m_oColor;
	IShader*			m_pShader;
};
