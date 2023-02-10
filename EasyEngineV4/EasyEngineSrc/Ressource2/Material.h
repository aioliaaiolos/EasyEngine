#ifndef MESH_CPP
#ifndef MATERIAL_CPP
#ifndef RESSOURCEMANAGER_CPP
#error
#endif
#endif
#endif

#ifndef MATERIAL_H
#define MATERIAL_H

#include "IRessource.h"
#include "IRenderer.h"

class ITexture;
class IRenderer;

class CMaterial : public IMaterial
{
public:
	enum TMaterialType
	{
		AMBIENT = 0,
		DIFFUSE,
		SPECULAR,
		EMISSIVE
	};

	struct Desc
	{
		std::vector< float >	m_vAmbient;
		std::vector< float >	m_vDiffuse;	
		std::vector< float >	m_vSpecular;
		std::vector< float >	m_vEmissive;
		float					m_fShininess;
		ITexture*				m_pDiffuseTexture;
		string					m_sName;
		IShader*				m_pShader = nullptr;
		IRenderer&				m_oRenderer;
		Desc(IRenderer& oRenderer, IShader* pShader);
	};
								CMaterial(const Desc& oDesc);
	virtual						~CMaterial();
	void						Update();
	void						GetMaterialMatrix( CMatrix& );
	void						SetShader( IShader* pShader );
	IShader*					GetShader() const { return m_pShader; }
	void						SetAdditionalColor(float r, float g, float b, float a) override;
	void						ClearAdditionalColor();
	ITexture*					GetTexture();
	void						SetTexture(ITexture* pTexture);
	void						SetAmbient(float r, float g, float b, float a) override;
	void						SetDiffuse(float r, float g, float b, float a) override;
	void						SetSpecular(float r, float g, float b, float a) override;
	void						SetSpecular(const CVector& pos) override;
	void						SetShininess(float shininess) override;
	CVector						GetSpecular() override;

private:
	std::vector< float >		m_vAmbient;
	std::vector< float >		m_vDiffuse;
	std::vector< float >		m_vSpecular;
	std::vector< float >		m_vEmissive;
	float						m_fShininess;
	ITexture*					m_pDiffuseTexture;
	IShader*					m_pShader;
	vector<float>				m_vAdditionalColor;
	bool						m_bUseAdditiveColor;
	IRenderer&					m_oRenderer;
};


#endif	//MATERIAL_H