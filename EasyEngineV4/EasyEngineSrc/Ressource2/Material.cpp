#define MATERIAL_CPP

#include "material.h"
#include "Exception.h"
#include "Loader/AseLoader.h"
#include "Texture.h"
#include "IShader.h"

using namespace std;

//#define OLD_VERSION

CMaterial::Desc::Desc(IRenderer& oRenderer, IShader* pShader ) :
m_fShininess( 0 ),
m_pDiffuseTexture(nullptr),
m_oRenderer(oRenderer),
m_pShader(pShader)
{
	for (unsigned int i=0 ; i<4 ; i++)
	{
		m_vAmbient.push_back( 0 );
		m_vSpecular.push_back( 0 );
		m_vDiffuse.push_back( 0 );
		m_vEmissive.push_back( 0 );
	}
}

CMaterial::CMaterial(const Desc& oDesc) :
m_pDiffuseTexture(NULL),
m_pShader( oDesc.m_pShader ),
m_bUseAdditiveColor(false),
m_oRenderer(oDesc.m_oRenderer),
m_nID(oDesc.m_nID)
{
	m_vAmbient = oDesc.m_vAmbient;
	m_vDiffuse = oDesc.m_vDiffuse;
	m_vSpecular = oDesc.m_vSpecular;
	m_vEmissive = oDesc.m_vEmissive;
	m_fShininess = oDesc.m_fShininess;
	m_pDiffuseTexture = oDesc.m_pDiffuseTexture;
	if (m_pDiffuseTexture) {
		for (int i = 0; i < 3; i++)
			m_vDiffuse[i] = 0.f;
		m_vDiffuse[3] = 1.f;
	}
}

CMaterial::~CMaterial(void)
{
}

void CMaterial::Update()
{
	if ( m_pDiffuseTexture )
	{
		m_pDiffuseTexture->SetShader( m_pShader );
		m_pDiffuseTexture->Update();
	}
	else
	{
		m_oRenderer.BindTexture( 0, 0, IRenderer::T_2D );
		m_pShader->SendUniformValues( "TextureCount", 0 );
	}	
#ifdef OLD_VERSION
	if (!m_bUseAdditiveColor) {
		GetRenderer().SetMaterialAmbient(&m_vAmbient[0]);
		if (!m_pDiffuseTexture)
			GetRenderer().SetMaterialDiffuse(&m_vDiffuse[0]);
	}
	else {
		vector<float> newAmbient, newDiffuse;
		for (int i = 0; i < 3; i++) {
			newAmbient.push_back(m_vAmbient[i] * (1 - m_vAdditionalColor[3]) + m_vAdditionalColor[i] * m_vAdditionalColor[3]);
			if (!m_pDiffuseTexture)
				newDiffuse.push_back(m_vDiffuse[i] * (1 - m_vAdditionalColor[3]) + m_vAdditionalColor[i] * m_vAdditionalColor[3]);
		}
		GetRenderer().SetMaterialAmbient(&newAmbient[0]);
		if (!m_pDiffuseTexture)
			GetRenderer().SetMaterialDiffuse(&newDiffuse[0]);
		m_bUseAdditiveColor = false;
	}

	GetRenderer().SetMaterialSpecular(&m_vSpecular[0]);
	GetRenderer().SetMaterialEmissive(&m_vEmissive[0]);
	GetRenderer().SetMaterialShininess(m_fShininess);

#else

	m_oRenderer.SetMaterialAmbient(&m_vAmbient[0]);
	m_oRenderer.SetMaterialDiffuse(&m_vDiffuse[0]);
	m_oRenderer.SetMaterialSpecular(&m_vSpecular[0]);
	m_oRenderer.SetMaterialEmissive(&m_vEmissive[0]);
	m_oRenderer.SetMaterialShininess(m_fShininess);
#endif // OLD_VERSION
}

void CMaterial::GetMaterialMatrix( CMatrix& m )
{
	m.Fill(	m_vAmbient[ 0 ], m_vAmbient[ 1 ], m_vAmbient[ 2 ], m_vAmbient[ 3 ] ,
			m_vDiffuse[ 0 ], m_vDiffuse[ 1 ], m_vDiffuse[ 2 ], m_vDiffuse[ 3 ] ,
			m_vSpecular[ 0 ], m_vSpecular[ 1 ], m_vSpecular[ 2 ], m_vSpecular[ 3 ] ,
			m_vEmissive[ 0 ], m_vEmissive[ 1 ], m_vEmissive[ 2 ], m_vEmissive[ 3 ] );
}

void CMaterial::SetShader( IShader* pShader )
{
	m_pShader = pShader;
}

void CMaterial::SetAdditionalColor(float r, float g, float b, float a)
{
#ifdef OLD_VERSION
	m_vAdditionalColor.push_back(r);
	m_vAdditionalColor.push_back(g);
	m_vAdditionalColor.push_back(b);
	m_vAdditionalColor.push_back(a);
	m_bUseAdditiveColor = true;
#else
	m_vAmbient[0] += r;
	m_vAmbient[1] += g;
	m_vAmbient[2] += b;
	m_vAmbient[3] += a;

	m_vDiffuse[0] += r;
	m_vDiffuse[1] += g;
	m_vDiffuse[2] += b;
	m_vDiffuse[3] += a;
#endif // OLD_VERSION
}

void CMaterial::ClearAdditionalColor()
{
	m_vDiffuse[0] = 0.f;
	m_vDiffuse[1] = 0.f;
	m_vDiffuse[2] = 0.f;
	m_vDiffuse[3] = 1.f;
}

ITexture* CMaterial::GetTexture()
{
	return m_pDiffuseTexture;
}

void CMaterial::SetTexture(ITexture* pTexture)
{
	m_pDiffuseTexture = pTexture;
}

void CMaterial::SetAmbient(float r, float g, float b, float a)
{
	m_vAmbient.clear();
	m_vAmbient.push_back(r);
	m_vAmbient.push_back(g);
	m_vAmbient.push_back(b);
	m_vAmbient.push_back(a);
}

void CMaterial::SetDiffuse(float r, float g, float b, float a)
{
	m_vDiffuse.clear();
	m_vDiffuse.push_back(r);
	m_vDiffuse.push_back(g);
	m_vDiffuse.push_back(b);
	m_vDiffuse.push_back(a);
}

void CMaterial::SetSpecular(float r, float g, float b, float a)
{
	m_vSpecular.clear();
	m_vSpecular.push_back(r);
	m_vSpecular.push_back(g);
	m_vSpecular.push_back(b);
	m_vSpecular.push_back(a);
}

void CMaterial::SetSpecular(const CVector& pos)
{
	m_vSpecular.clear();
	m_vSpecular.push_back(pos.m_x);
	m_vSpecular.push_back(pos.m_y);
	m_vSpecular.push_back(pos.m_z);
	m_vSpecular.push_back(1.f);
}

float CMaterial::GetShininess()
{
	return m_fShininess;
}

void CMaterial::SetShininess(float shininess)
{
	m_fShininess = shininess;
}

CVector CMaterial::GetSpecular()
{
	return CVector(m_vSpecular[0], m_vSpecular[1], m_vSpecular[2], 1.f);
}

int CMaterial::GetID()
{
	return m_nID;
}

void CMaterial::SetID(int nID)
{
	m_nID = nID;
}