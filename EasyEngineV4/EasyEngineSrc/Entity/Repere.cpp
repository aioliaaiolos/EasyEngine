#include "Repere.h"
#include "IShader.h"

CRepere::CRepere( IRenderer& oRenderer ):
CShape( oRenderer )
{
	SetName("Repere");
	SetEntityName("Repere");
	m_sEntityType = "Repere;";
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

const string& CRepere::GetIDStr() const
{
	return m_sEntityID;
}

const string& CRepere::GetTypeName() const
{
	return m_sEntityType;
}