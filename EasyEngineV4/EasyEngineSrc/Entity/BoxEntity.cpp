#include "BoxEntity.h"
#include "IGeometry.h"

CBoxEntity::CBoxEntity( IRenderer& oRenderer, IBox& oBox ):
CShape( oRenderer ),
m_oBox( oBox ),
m_oColor(1.f, 1.f, 1.f)
{
}

void CBoxEntity::Update()
{
	CShape::Update();
	m_oBox.SetTM( m_oWorldMatrix );
	if( !m_bHidden )
		m_oRenderer.DrawBox( m_oBox.GetMinPoint(), m_oBox.GetDimension(), m_oColor);
}

IBox& CBoxEntity::GetBox()
{
	return m_oBox;
}

void CBoxEntity::GetEntityName(string& sName)
{
	sName = m_sEntityName;
}

IGeometry* CBoxEntity::GetBoundingGeometry()
{
	return &m_oBox;
}