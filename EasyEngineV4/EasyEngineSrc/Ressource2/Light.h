#ifndef LIGHT_CPP
#ifndef RESSOURCEMANAGER_CPP
#error
#endif
#endif

#ifndef LIGHT_H
#define LIGHT_H

#include "IRessource.h"
#include "Math/Vector.h"

#include <windows.h>
#include <gl/gl.h>
#include <gl/GLU.h>


class CLight : public ILight
{	
public:
	struct Desc
	{
		CVector				Color;
		IRessource::TLight	type;
		float 				fIntensity;
		float 				fAttenuationConstant;
		float 				fAttenuationLinear;
		float 				fAttenuationQuadratic;
		IRenderer&			m_oRenderer;
		Desc( IRenderer& oRenderer, IShader* pShader );
	};
	

								CLight( const Desc& oDesc );
	virtual 					~CLight();
	void 						Update();
	TLight						GetType();
	bool						IsEnabled();
	void						Enable(bool enable);
	void						SetShader( IShader* pShader );
	void						SetIntensity( float fIntensity ) override;
	void						SetAmbient(float fAmbient) override;
	float						GetIntensity();
	CVector						GetColor();
	IShader*					GetShader() const { return NULL; }
	static void					RemoveAllLights(IRenderer& oRenderer);

private:
	static unsigned int			s_nCurrentLightID;
	static map< int, bool >		s_mEnabledLight;

	IRenderer&					m_oRenderer;
	CVector						m_Ambient;
	CVector						m_Diffuse;
	CVector						m_Specular;
	TLight						m_Type;
	float						m_fIntensity;
	unsigned int				m_ID;
	bool						m_bIsEnabled;
	GLUquadricObj*				m_pDebugQuadricObj;
};

#endif //LIGHT_H
