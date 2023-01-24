#pragma once

#include "BoxEntity.h"
#include "Interface.h"

class CAreaEntity : public CBoxEntity
{
public:
	CAreaEntity(string areaName, EEInterface& oInterface, IBox& oBox);
	void							Update();
	float							GetBoundingSphereRadius() const override;
	void							DrawBoundingBox(bool bDraw) override;
	void							SetScaleFactor(float x, float y, float z) override;
	void							GetScaleFactor(CVector& scale) override;
	void							ManageGravity() override;
	void							UpdateCollision() override;

protected:

	CVector							m_oInitialMinPoint;
	CVector							m_oInitialDimension;
	CVector							m_oCurrentMinPoint;
	CVector							m_oCurrentDimension;
	CVector							m_oLastPosition;
};
