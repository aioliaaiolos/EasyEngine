#define LIGHT_CPP

#include "light.h"
#include "IRenderer.h"

unsigned int CLight::s_nCurrentLightID = GL_LIGHT0;
map< CLight*, bool > CLight::s_mEnabledLight;

map<ILight::Type, string> CLight::s_TypeToString = { { ILight::OMNI, "omni" },{ ILight::DIRECTIONAL , "dir"},{ ILight::SPOT , "spot"} };

CLight::Desc::Desc( IRenderer& oRenderer, IShader* pShader ):
fIntensity( 0 ),
fAttenuationConstant( 1 ),
fAttenuationLinear( 0 ),
fAttenuationQuadratic( 0 ),
type( CLight::OMNI ),
Color( 1,1,1,1 ),
m_oRenderer(oRenderer)
{
}

CLight::CLight( const Desc& oDesc ):
m_bIsEnabled( true ),
m_Specular( oDesc.Color ),
m_Diffuse( oDesc.Color ),
m_Type( oDesc.type ),
m_ID( s_nCurrentLightID ),
m_Ambient(0.1f, 0.1f, 0.1f, 1.f),
m_oRenderer(oDesc.m_oRenderer),
m_fAttenuationConstant(oDesc.fAttenuationConstant),
m_fAttenuationLinear(oDesc.fAttenuationLinear),
m_fAttenuationQuadratic(oDesc.fAttenuationQuadratic)
{
	s_nCurrentLightID ++;
	oDesc.m_oRenderer.SetLightAttenuation( m_ID, oDesc.fAttenuationConstant, oDesc.fAttenuationLinear, oDesc.fAttenuationQuadratic );
	SetIntensity( oDesc.fIntensity );

	SetType(m_Type);


	m_oRenderer.EnableLight( m_ID );
	s_mEnabledLight[ this ] = true;
	ostringstream ossName;
	ossName << "Light" << m_ID;
	m_sName = ossName.str();
	m_oRenderer.SetLightLocalPos(m_ID, 0.f, 0.f, 0.f, 1.f);
}

void CLight::RemoveAllLights(IRenderer& oRenderer)
{
	s_nCurrentLightID = GL_LIGHT0;
	for (map<CLight*, bool>::iterator itLight = s_mEnabledLight.begin(); itLight != s_mEnabledLight.end(); itLight++) {
		if (itLight->first)
			itLight->first->Enable(false);
	}
	for (int i = 0; i < 8; i++) {
		oRenderer.DisableLight(GL_LIGHT0 + i);
	}
}

int CLight::GetLightCount()
{
	return (s_nCurrentLightID - GL_LIGHT0);
}

CLight::~CLight(void)
{
}

void CLight::Update()
{	
	if ( m_bIsEnabled )
	{
		if (m_Type == CLight::DIRECTIONAL)
		{						
			m_oRenderer.SetLightLocalPos(m_ID, 0.f, -1.f, 0.f, 0.f);
		}
		if (m_Type == CLight::OMNI)
		{
			m_oRenderer.SetLightLocalPos(m_ID, 0.f,0.f,0.f,1.f);
		}
		if (m_Type == CLight::SPOT)
		{
			m_oRenderer.SetLightLocalPos(m_ID, 0.f, 0.f, 0.f, 1.f);
			m_oRenderer.SetLightSpotDirection(m_ID, m_oSpotDirection.m_x, m_oSpotDirection.m_y, m_oSpotDirection.m_z, 0.f);
		}
	}
}

void CLight::Enable(bool enable)
{
	if (m_bIsEnabled == enable)
		return;
	m_bIsEnabled = enable;
	if (!enable) {
		m_oRenderer.DisableLight(m_ID);
	}
	else {
		m_ID = s_nCurrentLightID;
		m_oRenderer.EnableLight(m_ID);
		s_nCurrentLightID++;
		UpdateIntensity();
		SetAttenuation(m_fAttenuationConstant, m_fAttenuationLinear, m_fAttenuationQuadratic);
	}
}

CLight::Type CLight::GetType()
{
	return m_Type;
}

float CLight::GetIntensity()
{
	return m_fIntensity;
}

void CLight::SetIntensity(float fIntensity)
{
	m_fIntensity = fIntensity;
	UpdateIntensity();
}

void CLight::UpdateIntensity()
{
	GLfloat diffuse[] = { m_fIntensity*m_Diffuse.m_x,m_fIntensity*m_Diffuse.m_y,m_fIntensity*m_Diffuse.m_z,m_Diffuse.m_w };
	GLfloat specular[] = { m_fIntensity*m_Specular.m_x,m_fIntensity*m_Specular.m_y,m_fIntensity*m_Specular.m_z,m_Specular.m_w };
	GLfloat ambient[] = { m_fIntensity*m_Ambient.m_x,m_fIntensity*m_Ambient.m_y,m_fIntensity*m_Ambient.m_z,1.f };

	m_oRenderer.SetLightAmbient(m_ID, ambient[0], ambient[1], ambient[2], ambient[3]);
	m_oRenderer.SetLightDiffuse(m_ID, diffuse[0], diffuse[1], diffuse[2], diffuse[3]);
	m_oRenderer.SetLightSpecular(m_ID, specular[0], specular[1], specular[2], specular[3]);
}

float CLight::GetAmbient()
{
	return m_Ambient.m_x;
}

void CLight::SetAmbient(float fAmbient)
{
	m_Ambient = CVector(fAmbient, fAmbient, fAmbient, 1.f);
	GLfloat ambient[] = { m_fIntensity * fAmbient, m_fIntensity * fAmbient, m_fIntensity * fAmbient, 1.f };
	m_oRenderer.SetLightAmbient(m_ID, ambient[0], ambient[1], ambient[2], ambient[3]);
}

void CLight::SetSpecular(float fSpecular)
{
	m_Specular = CVector(fSpecular, fSpecular, fSpecular, 1.f);
	GLfloat specular[] = { m_fIntensity * fSpecular, m_fIntensity * fSpecular, m_fIntensity * fSpecular, 1.f };
	m_oRenderer.SetLightSpecular(m_ID, specular[0], specular[1], specular[2], specular[3]);
}

void CLight::CastShadow(bool castShadow)
{
	m_bCastShadow = castShadow;
}

bool CLight::IsCastShadow()
{
	return m_bCastShadow;
}

void CLight::SetSun(bool sun)
{
	m_bIsSun = sun;
}

bool CLight::IsSun()
{
	return m_bIsSun;
}

bool CLight::IsEnabled()
{
	return m_bIsEnabled;
}

void CLight::SetShader( IShader* pShader )
{
}

CVector CLight::GetColor()
{
	return m_Diffuse;
}

void CLight::SetSpotDirection(CVector dir)
{
	m_oSpotDirection = dir;
}

void CLight::SetSpotAngle(float angle)
{
	m_fSpotAngle = angle;
	m_oRenderer.SetLightSpotProp(m_ID, m_fSpotAngle, 1);
}

void CLight::SetAttenuation(float fAttenuationConstant, float fAttenuationLinear, float fAttenuationQuadratic)
{
	m_fAttenuationConstant = fAttenuationConstant;
	m_fAttenuationLinear = fAttenuationLinear;
	m_fAttenuationQuadratic = fAttenuationQuadratic;
	m_oRenderer.SetLightAttenuation(m_ID, fAttenuationConstant, fAttenuationLinear, fAttenuationQuadratic);
}

void CLight::SetType(Type type)
{
	m_Type = type;
	if (type != CLight::DIRECTIONAL)
		SetAttenuation(0.01f, 0.0001f, 0.000001f);
	else
		SetAttenuation(0.01f, 0.f, 0.f);

	if (type == CLight::SPOT)
		m_oRenderer.SetLightSpotProp(m_ID, 10.f, 1);
}
