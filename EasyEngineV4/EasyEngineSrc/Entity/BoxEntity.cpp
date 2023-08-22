#include "BoxEntity.h"
#include "Interface.h"
#include "IShader.h"
#include "IGeometry.h"

CBoxEntity::CBoxEntity(EEInterface& oInterface, IBox& oBox) :
	CObject(oInterface, ""),
	m_oBox(oBox),
	m_oColor(1.f, 1.f, 1.f),
	m_pShader(nullptr)
{
	m_pShader = m_oRenderer.GetShader("color");
}

void CBoxEntity::Update()
{
	CObject::Update();
	m_oBox.SetTM(m_oWorldMatrix);
	if (!m_bHidden) {
		m_pShader->Enable(true);
		m_oRenderer.DrawBox(m_oBox.GetMinPoint(), m_oBox.GetDimension(), m_oColor);
	}
}

IBox& CBoxEntity::GetBox()
{
	return m_oBox;
}


IGeometry* CBoxEntity::GetBoundingGeometry()
{
	return &m_oBox;
}

void CBoxEntity::SetWeight(float fWeight)
{
}

