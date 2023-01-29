#include "Repere.h"
#include "IShader.h"

CRepere::CRepere( IRenderer& oRenderer ):
CShape( oRenderer )
{
	SetName("Repere");
	SetEntityName("Repere");
}

void CRepere::Update()
{
	CShape::Update();
	if (!m_bHidden)
		m_oRenderer.DrawBase(m_oBase, 1000);
}

void CRepere::SetEntityName(string sName) 
{ 
	m_sEntityID = sName;
}

void CRepere::GetEntityID(string& sName)
{
	sName = m_sEntityID;
}

const string& CRepere::GetEntityID() const
{
	return m_sEntityID;
}