#define TEXTURE_CPP

#include "texture.h"
#include "Exception.h"
#include "IRenderer.h"
#include "IShader.h"

using namespace std;


CTextureBase::CDesc::CDesc( IRenderer& oRenderer, IShader* pShader, int nUnitTexture ):
m_nUnitTexture( nUnitTexture ),
m_pData( NULL ),
m_oRenderer(oRenderer)
{
}

CTextureBase::CTextureBase(const CDesc& oDesc):
m_nUnitTexture( oDesc.m_nUnitTexture ),
m_nID( -1 ),
m_pShader( oDesc.m_pShader ),
m_nFrameBufferObjectId(oDesc.m_nFrameBufferObjectId),
m_sUnitName("baseMap"),
m_oRenderer(oDesc.m_oRenderer)
{
}

void CTextureBase::SetShader( IShader* pShader )
{
	m_pShader = pShader;
}

unsigned int CTextureBase::GetFrameBufferObjectId()
{
	return m_nFrameBufferObjectId;
}

void CTextureBase::SetUnitTexture(int nUnitTexture)
{
	m_nUnitTexture = nUnitTexture;
}

void CTextureBase::SetUnitName(string sUnitName)
{
	m_sUnitName = sUnitName;
}

CTexture1D::CDesc::CDesc( IRenderer& oRenderer, IShader* pShader, int nUnitTexture ) :
CTextureBase::CDesc( oRenderer, pShader, nUnitTexture ),
m_nSize( 0 )
{
}

CTexture1D::CTexture1D( CDesc& oDesc ):
CTextureBase( oDesc ),
m_nSize( oDesc.m_nSize ),
m_bFirstUpdate( true )
{
	m_nID = m_oRenderer.CreateTexture1D( oDesc.m_pData, oDesc.m_nSize, IRenderer::T_RGBA );
	//delete oDesc.m_pData;
}

void CTexture1D::GetDimension( int& nWidth, int& nHeight )
{
	nWidth = m_nSize;
	nHeight = 1;
}
int nCounter = 0;
float fValue = 0.f;
void CTexture1D::Update()
{
	m_oRenderer.BindTexture( m_nID, m_nUnitTexture, IRenderer::T_1D );
	try
	{
		m_pShader->SendUniformValues( "MaterialMap", m_nUnitTexture );
	}
	catch(exception& e)
	{
		if( m_bFirstUpdate )
			MessageBox(NULL, e.what(), "void CTexture1D::Update()", MB_ICONEXCLAMATION);
	}
	//m_pShader->SendUniformValues( "ValueTextureMaterial", m_nID );	
	try
	{
		m_pShader->SendUniformValues( "ArraySize", (float)m_nSize );
	}
	catch( exception& e )
	{
		if ( m_bFirstUpdate )
			MessageBox(NULL, e.what(), "void CTexture1D::Update()", MB_ICONEXCLAMATION);
	}
	if ( m_bFirstUpdate )
		m_bFirstUpdate = false;

}

IShader* CTexture1D::GetShader() const
{
	return m_pShader;
}

CTexture2D::CDesc::CDesc( IRenderer& oRenderer, IShader* pShader, int nUnitTexture ):
CTextureBase::CDesc( oRenderer, pShader, m_nUnitTexture ),
m_nWidth( 0 ),
m_nHeight( 0 ),
m_bGenerateMipmaps(true),
m_bRenderTexture(false),
m_nTextureId(-1)
{
}

CTexture2D::CTexture2D(  CDesc& oDesc ):
CTextureBase( oDesc ),
m_nReponse( -1 )
{
	m_nWidth = oDesc.m_nWidth;
	m_nHeight = oDesc.m_nHeight;
	if (oDesc.m_nTextureId == -1) {
		if (oDesc.m_bGenerateMipmaps)
			m_nID = m_oRenderer.CreateMipmaps2D(oDesc.m_vTexels, m_nWidth, m_nHeight, oDesc.m_eFormat);
		else
			m_nID = m_oRenderer.CreateTexture2D(oDesc.m_vTexels, m_nWidth, m_nHeight, oDesc.m_eFormat);
	}
	else
		m_nID = oDesc.m_nTextureId;
	if (!oDesc.m_bRenderTexture && (oDesc.m_vTexels.size() == 0))
	{
		CRessourceException e("CTexture2D::CTexture2D() : texel array empty");
		throw e;
	}
}

IShader* CTexture2D::GetShader() const
{
	return m_pShader;
}

void CTexture2D::Update()
{
	m_oRenderer.BindTexture( m_nID, m_nUnitTexture, IRenderer::T_2D );
	m_pShader->SendUniformValues(m_sUnitName, m_nUnitTexture);
	try
	{
		m_pShader->SendUniformValues( "ValueTexture0", m_nID );
	}
	catch( exception& e )
	{
		if( m_nReponse != 6 )
		{
			string sMessage = e.what();
			sMessage += "\nVoulez vous continuer et ignorer tous les avertissements de ce type ?";
			m_nReponse = MessageBox( NULL, sMessage.c_str(), "", MB_YESNO );
		}
	}
}

void CTexture2D::GetDimension( int& nWidth, int& nHeight )
{
	nWidth = m_nWidth;
	nHeight = m_nHeight;
}