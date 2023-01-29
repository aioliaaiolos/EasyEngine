#include "LineEntity.h"

CLineEntity::CLineEntity( IRenderer& oRenderer ):
CShape( oRenderer )
{
}

CLineEntity::CLineEntity( IRenderer& oRenderer, const CVector& first, const CVector& last ):
CShape( oRenderer ),
m_oFirst( first ),
m_oLast( last )
{
}


void CLineEntity::Update()
{
	CShape::Update();
	if (!m_bHidden) {
		CMatrix cam, camInv;
		m_oRenderer.GetInvCameraMatrix(camInv);
		camInv.GetInverse(cam);
		m_oRenderer.SetModelViewMatrix(cam);
		m_oRenderer.DrawLine(m_oFirst, m_oLast, CVector(1.f, 1.f, 1.f));
	}
}

void CLineEntity::Set( const CVector& first, const CVector& last )
{
	m_oFirst = first;
	m_oLast = last;
}

void CLineEntity::GetEntityID(string& sName)
{
	sName = m_sEntityID;
}

const string& CLineEntity::GetEntityID() const
{
	return m_sEntityID;
}

void CLineEntity::GetPoints(CVector& first, CVector& last) const
{
	first = m_oFirst;
	last = m_oLast;
}
