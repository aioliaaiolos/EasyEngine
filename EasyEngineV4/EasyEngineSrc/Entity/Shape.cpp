#include "Shape.h"	
#include "IRenderer.h"
#include "IShader.h"

CShape::CShape( IRenderer& oRenderer ):
m_oRenderer( oRenderer ),
m_pShader( NULL ),
m_bHidden( false )
{
	m_pShader = oRenderer.GetShader( "color" );
	m_sTypeName = "Shape";
}

void CShape::DrawBoundingBox( bool bDraw )
{

}

void CShape::ReloadShader()
{
	m_pShader = m_oRenderer.GetShader("color");
}

void CShape::Update()
{
	CNode::Update();
	if( !m_bHidden )
	{
		m_pShader->Enable( true );
		m_oRenderer.SetModelMatrix( m_oWorldMatrix );
	}
}

void CShape::SetAxesLength( float r, float g, float b )
{
}

void CShape::SetAxesColor( int r, int g, int b )
{
}

void CShape::GetRessourceFileName( string& sFileName )
{
	sFileName = "EE_Repere_19051978";
}

void CShape::SetShader( IShader* pShader )
{
	m_pShader = pShader;
}

void CShape::Hide( bool bHide )
{
	m_bHidden = bHide;
}