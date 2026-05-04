#include "LightEntity.h"
#include "IShader.h"
#include <algorithm>
#include "Scene.h"

CLightEntity::CLightEntity(EEInterface& oInterface, IRessource* pLight):
CEntity(oInterface)
{	
	m_pRessource = pLight;
	m_pLight = static_cast<ILight*>(m_pRessource);
	SetEntityID("Light");
	m_pRepere = new CRepere(m_oRenderer);
	m_pRepere->Link(this);
}

CLightEntity::~CLightEntity()
{
	if(m_pLight)
		m_pLight->Enable(false);
}

void CLightEntity::Update()
{
	CNode::Update();
	m_oRenderer.SetModelMatrix(m_oWorldMatrix);
	UpdateRessource();
	DispatchEntityEvent();
}

void CLightEntity::Yaw(float angle)
{
	CNode::Yaw(angle);
	Update();
	if (m_pLight->GetType() == ILight::SPOT) {		
		CVector dir;
		m_oWorldMatrix.GetxBase(dir);
		m_pLight->SetSpotDirection(dir);
	}
}

void CLightEntity::Pitch(float angle)
{
	CNode::Pitch(angle);
	Update();
	if (m_pLight->GetType() == ILight::SPOT) {
		CVector dir;
		m_oWorldMatrix.GetxBase(dir);
		m_pLight->SetSpotDirection(dir);
	}	
}

void CLightEntity::Roll(float angle)
{
	CNode::Roll(angle);
	Update();
	if (m_pLight->GetType() == ILight::SPOT) {
		CVector dir;
		m_oWorldMatrix.GetxBase(dir);
		m_pLight->SetSpotDirection(dir);
	}
}

ILight* CLightEntity::GetLight()
{
	return static_cast<ILight*>(m_pRessource);
}

void CLightEntity::UpdateRessource()
{
	if (m_pRessource) {
		m_pRessource->Update();
	}
}

void CLightEntity::SetIntensity( float fIntensity )
{
	m_oRessourceManager.SetLightIntensity( m_pRessource, fIntensity );
}

float CLightEntity::GetIntensity()
{
	return m_oRessourceManager.GetLightIntensity( m_pRessource );
}

CVector CLightEntity::GetColor()
{
	return m_oRessourceManager.GetLightColor( m_pRessource );
}

ILight::Type CLightEntity::GetType()
{
	return m_oRessourceManager.GetLightType( m_pRessource );
}

void CLightEntity::Unlink()
{
	if(m_pLight)
		m_pLight->Enable(false);
	CEntity::Unlink();
}

void CLightEntity::Link(INode* pNode)
{
	__super::Link(pNode);
	if (m_pLight)
		m_pLight->Enable(true);
}

void CLightEntity::GetEntityInfos(ILoader::CObjectInfos*& pInfos)
{
	if(!pInfos)
		pInfos = new ILoader::CLightEntityInfos;
	CEntity::GetEntityInfos(pInfos);
	ILoader::CLightEntityInfos& lightInfos = static_cast< ILoader::CLightEntityInfos& >(*pInfos);
	lightInfos.m_fIntensity = GetIntensity();
	lightInfos.m_oColor = GetColor();
	ILoader::CLightInfos::TLight type;
	switch (GetType())
	{
	case ILight::Type::DIRECTIONAL:
		type = ILoader::CLightInfos::eDirectionnelle;
		break;
	case ILight::Type::OMNI:
		type = ILoader::CLightInfos::eOmni;
		break;
	case ILight::Type::SPOT:
		type = ILoader::CLightInfos::eTarget;
		break;
	}
	lightInfos.m_eType = type;
}

void CLightEntity::BuildFromInfos(const ILoader::CObjectInfos& infos, IEntity* pParent, bool bExcludeChildren)
{
	CEntity::BuildFromInfos(infos, pParent);
	const ILoader::CLightEntityInfos* pLightEntityInfos = static_cast< const ILoader::CLightEntityInfos* >(&infos);
	ILight::Type type;
	switch (pLightEntityInfos->m_eType)
	{
	case ILoader::CLightInfos::eDirectionnelle:
		type = ILight::Type::DIRECTIONAL;
		break;
	case ILoader::CLightInfos::eOmni:
		type = ILight::Type::OMNI;
		break;
	case ILoader::CLightInfos::eTarget:
		type = ILight::Type::SPOT;
		break;
	default:
		throw 1;
	}
	IRessource* pLight = m_oRessourceManager.CreateLight(pLightEntityInfos->m_oColor, type, pLightEntityInfos->m_fIntensity);
	m_pRessource = pLight;
	SetLocalMatrix(infos.m_oXForm);
}

void CLightEntity::RenderFirstShadowPass(vector<IEntity*>& entities)
{
	if (m_pLight && m_pLight->IsCastShadow()) {
		m_oWorldMatrix.GetInverse(m_oLightViewMatrix);
		m_oRenderer.CalcOrthoProjection(m_oLightProjection, -m_fShadowFrustumSize / 4.f, m_fShadowFrustumSize / 4.f, -m_fShadowFrustumSize / 4.f, m_fShadowFrustumSize / 4.f, 100, m_fShadowFrustumSize);		

		int shadowWidth, shadowHeight;
		m_pShadowTexture->GetDimension(shadowWidth, shadowHeight);
		int backupViewportWidth, backupViewportHeight;
		m_oRenderer.GetViewPort(backupViewportWidth, backupViewportHeight);
		m_oRenderer.SetViewPort(shadowWidth, shadowHeight);

		// first pass
		m_oRenderer.SetCurrentFBO(m_pShadowTexture->GetFrameBufferObjectId());
		

		CMatrix oBackupInvCameraMatrix;
		m_oRenderer.GetInvCameraMatrix(oBackupInvCameraMatrix);
		m_oRenderer.SetInvCameraMatrix(m_oLightViewMatrix);
		CMatrix backupProjection;
		m_oRenderer.GetProjectionMatrix(backupProjection);
		m_oRenderer.SetProjectionMatrix(m_oLightProjection);
		m_oRenderer.ClearFrameBuffer();
		IShader* pBackupShader = NULL;
		IShader* pFirstPassShader = m_oRenderer.GetShader(m_sShadowMapFirstPassShaderName);
		IShader* pFirstPassSkinningShader = m_oRenderer.GetShader(m_sShadowMapFirstPassSkinningShaderName);

		CMatrix m;
		m_oRenderer.SetModelMatrix(m);
		IMesh* pGround = static_cast<IMesh*>(GetRessource());
		pBackupShader = pGround->GetShader();
		pGround->SetShader(pFirstPassShader);
		//pGround->Update();
		pGround->SetShader(pBackupShader);

		for (int i = 0; i < entities.size(); i++) {
			CEntity* pEntity = dynamic_cast<CEntity*>(entities[i]);
			IRessource* pRessource = pEntity->GetRessource();
			if (pRessource) {
				pBackupShader = pRessource->GetShader();
				IShader* pShader = nullptr;
				if (pBackupShader->GetName().find("Skinning") != -1) {
					pShader = pFirstPassSkinningShader;
				}
				else {
					pShader = pFirstPassShader;
				}
				pEntity->SetShader(pShader);
				m_oRenderer.SetModelMatrix(pEntity->GetWorldMatrix());
				pEntity->Update();
				pEntity->SetShader(pBackupShader);
			}
		}
		m_oRenderer.SetInvCameraMatrix(oBackupInvCameraMatrix);
		m_oRenderer.SetProjectionMatrix(backupProjection);

		m_oRenderer.SetCurrentFBO(0);
		m_oRenderer.SetViewPort(backupViewportWidth, backupViewportHeight);
	}
}

void CLightEntity::RenderSecondShadowPass(IShader* pEntityShader, CEntity* pEntity)
{
	std::string shaderNameLow = pEntityShader->GetName();
	std::transform(pEntityShader->GetName().begin(), pEntityShader->GetName().end(), shaderNameLow.begin(), tolower);
	if (m_pShadowTexture && (shaderNameLow == "perpixellighting" || shaderNameLow == "skinning" || shaderNameLow == "ground")) {
		pEntityShader->Enable(true);
		m_pShadowTexture->SetShader(pEntityShader);
		m_pShadowTexture->Update();

		CVector lightDir = m_oWorldMatrix * CVector(0, 0, 1, 1);
		lightDir.Normalize();
		pEntityShader->SendUniformVec4f("lightDir", lightDir);
		pEntityShader->SendUniformMatrix4("lightSpaceModelview", m_oLightViewMatrix, true);
		pEntityShader->SendUniformMatrix4("lightSpaceProjection", m_oLightProjection, true);
		if(pEntity)
			pEntityShader->SendUniformMatrix4("model", pEntity->GetWorldMatrix(), true);
	}
}

void CLightEntity::SetShadowFrustumSize(float size)
{
	m_fShadowFrustumSize = size;
}

float CLightEntity::GetShadowFrustumSize()
{
	return m_fShadowFrustumSize;
}

void CLightEntity::RenderShadowMap(const vector<IEntity*>& entities, const CMatrix& lightView, const CMatrix& lightProjection)
{
	CMatrix oBackupInvCameraMatrix;
	m_oRenderer.GetInvCameraMatrix(oBackupInvCameraMatrix);
	m_oRenderer.SetInvCameraMatrix(lightView);
	CMatrix backupProjection;
	m_oRenderer.GetProjectionMatrix(backupProjection);
	m_oRenderer.SetProjectionMatrix(lightProjection);
	m_oRenderer.ClearFrameBuffer();
	IShader* pBackupShader = NULL;
	IShader* pFirstPassShader = m_oRenderer.GetShader(m_sShadowMapFirstPassShaderName);
	IShader* pFirstPassSkinningShader = m_oRenderer.GetShader(m_sShadowMapFirstPassSkinningShaderName);

	CMatrix m;
	m_oRenderer.SetModelMatrix(m);
	IMesh* pGround = static_cast<IMesh*>(GetRessource());
	pBackupShader = pGround->GetShader();
	pGround->SetShader(pFirstPassShader);
	//pGround->Update();
	pGround->SetShader(pBackupShader);

	for (int i = 0; i < entities.size(); i++) {
		CEntity* pEntity = dynamic_cast<CEntity*>(entities[i]);
		IRessource* pRessource = pEntity->GetRessource();
		if (pRessource) {
			pBackupShader = pRessource->GetShader();
			IShader* pShader = nullptr;
			if (pBackupShader->GetName().find("Skinning") != -1) {
				pShader = pFirstPassSkinningShader;
			}
			else {
				pShader = pFirstPassShader;
			}
			pEntity->SetShader(pShader);
			m_oRenderer.SetModelMatrix(pEntity->GetWorldMatrix());
			pEntity->Update();
			pEntity->SetShader(pBackupShader);
		}
	}
	m_oRenderer.SetInvCameraMatrix(oBackupInvCameraMatrix);
	m_oRenderer.SetProjectionMatrix(backupProjection);
}



void CLightEntity::CastShadow(bool castShadow)
{
	if(castShadow)
		m_pShadowTexture = CreateShadowTexture();
	GetLight()->CastShadow(castShadow);
}

ITexture* CLightEntity::CreateShadowTexture()
{
	unsigned int nMinimapWidth = 8192, nMinimapHeight = 8192;
	ITexture* pMinimapTexture = m_oRessourceManager.CreateRenderTexture(nMinimapWidth, nMinimapHeight, m_sMiniMapSecondPassShaderName, IRessourceManager::DEPTH, 9);
	pMinimapTexture->SetUnitName("shadowMap");
	return pMinimapTexture;
}
