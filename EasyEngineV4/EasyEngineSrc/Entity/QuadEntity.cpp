#include "QuadEntity.h"


CQuadEntity::CQuadEntity(IRenderer& oRenderer, IRessourceManager& oRessourceManager, IQuad& oQuad) :
	CShape(oRenderer),
	m_oQuad(oQuad),
	m_oRessourceManager(oRessourceManager),
	m_sName("QuadEntity"),
	m_oColor(1.f, 0.f, 0.f)
{

}

void CQuadEntity::Update()
{
	CShape::Update();
	if (!m_bHidden) {
		CMatrix cam, camInv;
		m_oRenderer.GetInvCameraMatrix(camInv);
		camInv.GetInverse(cam);
		m_oRenderer.SetModelViewMatrix(cam);
		m_oRenderer.SetColor3f(m_oColor.m_x, m_oColor.m_y, m_oColor.m_z);
		m_oQuad.Draw(m_oRenderer);
	}
}

void CQuadEntity::GetEntityID(string& sID)
{
	sID = m_sName;
}

const string& CQuadEntity::GetEntityID() const
{
	return m_sName;
}

void CQuadEntity::Colorize(float r, float g, float b, float a)
{
	m_oColor.Fill(r, g, b, 1.f);
}