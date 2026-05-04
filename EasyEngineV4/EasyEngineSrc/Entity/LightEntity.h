#ifndef LIGHTENTITY_H
#define LIGHTENTITY_H

#include "Entity.h"
#include "Repere.h"

class IRessource;
class IRessourceManager;
class IRenderer;

class CLightEntity : public CEntity, public ILightEntity
{
public:
	CLightEntity(EEInterface& oInterface, IRessource* pLight);
	~CLightEntity();
	void				Update() override;
	void				UpdateRessource() override;
	void				SetIntensity( float fInstensity );
	float				GetIntensity();
	CVector				GetColor();
	ILight::Type		GetType();
	void				Unlink() override;
	void				Link(INode* pNode) override;
	void				GetEntityInfos(ILoader::CObjectInfos*& pInfos);
	void				BuildFromInfos(const ILoader::CObjectInfos& infos, IEntity* pParent, bool bExcludeChildren = false) override;
	void				GetEntityID(string& sID) {};
	const string&		GetEntityID() const { return ""; };
	void				Yaw(float yaw);
	void				Pitch(float yaw);
	void				Roll(float yaw);
	ILight*				GetLight() override;
	void				SetShadowFrustumSize(float size) override;
	float				GetShadowFrustumSize() override;
	void				RenderFirstShadowPass(vector<IEntity*>& entities);
	void				RenderSecondShadowPass(IShader* pEntityShader, CEntity* pEntity);
	void				CastShadow(bool castShadow);

private:
	void				RenderShadowMap(const vector<IEntity*>& entities, const CMatrix& lightView, const CMatrix& lightProjection);
	ITexture*			CreateShadowTexture();

	ILight*				m_pLight;
	CRepere*			m_pRepere = nullptr;
	CMatrix				m_oLightViewMatrix;
	CMatrix				m_oLightProjection;
	float				m_fShadowFrustumSize = 20000.f;
	string				m_sShadowMapFirstPassShaderName = "shadowMapFirstPass";
	string				m_sShadowMapFirstPassSkinningShaderName = "shadowMapSkinningFirstPass";
	string				m_sMiniMapSecondPassShaderName = "mapSecondPass2D";
	ITexture*			m_pShadowTexture = nullptr;
};

#endif // LIGHTENTITY_H