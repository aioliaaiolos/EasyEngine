#include "Scene.h"
#include "Interface.h"
#include <algorithm>
#include "IRenderer.h"
#include "FreeCamera.h"
#include "IRessource.h"
#include "Shape.h"
#include "IEntity.h"
#include "LightEntity.h"
#include "ICollisionManager.h"
#include "Utils2/StringUtils.h"
#include "Utils2/TimeManager.h"
#include "IPathFinder.h"
#include "IFileSystem.h"
#include "EntityManager.h"
#include "MobileEntity.h"
#include "MapEntity.h"
#include "SphereEntity.h"
#include "IShader.h"
#include "IConsole.h"
#include "Bone.h"
#include "NPCEntity.h"
#include "Player.h"
#include "Repere.h"
#include "IPhysic.h"

using namespace std;

CScene::Desc::Desc( IRessourceManager& oRessourceManager, IRenderer& oRenderer, IEntityManager* pEntityManager, 
	ICamera* pCamera, ICameraManager& oCameraManager, ILoaderManager& oLoaderManager, 
	ICollisionManager& oCollisionManager, IGeometryManager& oGeometryManager, IPathFinder& oPathFinder ):
m_oRessourceManager( oRessourceManager ),
m_oRenderer( oRenderer ),
m_pCamera( pCamera ),
m_oCameraManager( oCameraManager ),
m_oLoaderManager( oLoaderManager ),
m_oCollisionManager( oCollisionManager ),
m_oGeometryManager( oGeometryManager )
{
m_pEntityManager = pEntityManager;
}

CScene::CScene(EEInterface& oInterface, string ressourceFileName, string diffuseFileName) :
	CEntity(oInterface, ressourceFileName),
	m_oCameraManager(static_cast<ICameraManager&>(*oInterface.GetPlugin("CameraManager"))),
	m_oLoaderManager(static_cast<ILoaderManager&>(*oInterface.GetPlugin("LoaderManager"))),
	m_oCollisionManager(static_cast<ICollisionManager&>(*oInterface.GetPlugin("CollisionManager"))),
	m_oRessourceManager(static_cast<IRessourceManager&>(*oInterface.GetPlugin("RessourceManager"))),
	m_oFileSystem(static_cast<IFileSystem&>(*oInterface.GetPlugin("FileSystem"))),
	m_nHeightMapID(-1),
	m_bHeightMapCreated(true),
	m_sMiniMapFirstPassShaderName("mapFirstPass"),
	m_sMiniMapSecondPassShaderName("mapSecondPass2D"),
	m_sShadowMapFirstPassShaderName("shadowMapFirstPass"),
	m_sShadowMapFirstPassSkinningShaderName("shadowMapSkinningFirstPass"),
	m_pPlayer(NULL),
	m_bDisplayMinimap(false),
	m_fGroundMargin(-10.f),
	m_pGroundShader(NULL),
	m_pHeightMaptexture(NULL),
	m_bUseDisplacementMap(false),
	m_fDisplacementRatioHeightSize(0.04f),
	m_fTiling(200.f),
	m_sDiffuseFileName(diffuseFileName),
	m_nMapLength(1000),
	m_fMapHeight(10.f),
	m_eSceneState(eStart)
{
	m_nID = 0;
	SetName("Scene");
	SetEntityID("Scene");
	m_pRessource = NULL;
	m_pEntityManager = static_cast<CEntityManager*>(oInterface.GetPlugin("EntityManager"));


	ICamera* pMapCamera = m_oCameraManager.CreateCamera(ICameraManager::TMap, 40.f);
	pMapCamera->Link(this);

	m_pMiniMapCamera = m_oCameraManager.GetCameraFromType(ICameraManager::TMap);
	m_pMiniMapCamera->SetWorldPosition(0, 40000, 0);
	m_pMiniMapCamera->Pitch(-90);

	m_pMinimapTexture = CreateMinimapTexture();	

	m_pPlayerMapSphere = dynamic_cast<CEntity*>(m_pEntityManager->CreateEntity("playerPointer.bme"));
	m_pPlayerMapSphere->SetShader(m_oRenderer.GetShader(m_sMiniMapFirstPassShaderName));
	m_pPlayerMapSphere->SetName("PlayerPointer");

	CTerrain* pTerrain = new CTerrain;
	string root;
	m_oFileSystem.GetLastDirectory(root);

	pTerrain->m_pTerrainTexture = static_cast<ITexture*> (m_oRessourceManager.CreateTexture2D(root + "/Textures/road01.bmp", true));
	pTerrain->m_pTerrainTexture->SetUnitTexture(5);
	pTerrain->m_pTerrainTexture->SetUnitName("terrainMap01");

	pTerrain->m_pSplatTexture = static_cast<ITexture*> (m_oRessourceManager.CreateTexture2D(root + "/Textures/splat01.bmp", false));
	pTerrain->m_pSplatTexture->SetUnitTexture(6);
	pTerrain->m_pSplatTexture->SetUnitName("splatMap01");

	m_terrains.push_back(pTerrain);
}

CScene::~CScene()
{
}

ITexture* CScene::CreateMinimapTexture()
{
	unsigned int nMinimapWidth, nMinimapHeight;
	m_oRenderer.GetResolution(nMinimapWidth, nMinimapHeight);
	ITexture* pMinimapTexture = m_oRessourceManager.CreateRenderTexture(nMinimapWidth, nMinimapHeight, m_sMiniMapSecondPassShaderName, IRessourceManager::COLOR, 9);
	return pMinimapTexture;
}

ITexture* CScene::GetMinimapTexture()
{
	return m_pMinimapTexture;
}

void CScene::SetRessource(string sFileName, bool bDuplicate)
{
	string ext;
	int nExtPos = (int)sFileName.find(".");
	if (nExtPos != -1) {
		ext = sFileName.substr(nExtPos + 1, 3);
		string extLower = ext;
		std::transform(ext.begin(), ext.end(), extLower.begin(), tolower);
		if (extLower == "bme")
			m_bUseDisplacementMap = false;
		else if (extLower == "bmp")
			m_bUseDisplacementMap = true;
	}
	else
	{
		exception e("CScene::SetRessource() : Erreur, vous devez indiquer une extention pour le fichier de ressource de la scene");
		throw e;
	}

	bool forceReloadHeightMap = true;
	int nDotPos = (int)sFileName.find('.');
	string sLevelDirectory;
	int sliceCount = 0;
	if (m_bUseDisplacementMap) {
		m_sHMFileName = sFileName;
		sliceCount = m_nMapLength / 500;
		string prefix = "/HMA_";
		int startIdx = m_sHMFileName.find("/HMA_");
		string levelFileName;
		if (startIdx >= 0)
		{
			levelFileName = m_sHMFileName.substr(startIdx + prefix.size());
			m_sCurrentLevelName = levelFileName.substr(0, levelFileName.find('.'));
		}
		else
			m_sCurrentLevelName = "tmp";
		sLevelDirectory = string("/levels/") + m_sCurrentLevelName + "/";
		string levelPath = sLevelDirectory + "ground.bme";
		
		try {
			IAnimatableMesh* pAnimatableMesh = dynamic_cast<IAnimatableMesh*>(m_oRessourceManager.GetRessource(levelPath));
			m_pRessource = pAnimatableMesh->GetMesh(0);
		}
		catch (CFileException& e) {
			string directoryPath = "/levels/tmp";
			levelPath = directoryPath + "/ground.bme";
			try {
				IAnimatableMesh* pAnimatableMesh = dynamic_cast<IAnimatableMesh*>(m_oRessourceManager.GetRessource(levelPath));
				m_pRessource = pAnimatableMesh->GetMesh(0);
			}
			catch (CFileException& e) {
				string directoryFullPath = string("../Data") + directoryPath;
				string levelFullPath = string("../Data") + levelPath;
				CreateDirectoryA(directoryFullPath.c_str(), nullptr);
				m_pRessource = m_oRessourceManager.CreatePlane2(sliceCount, m_nMapLength, m_fMapHeight, m_sHMFileName, m_sDiffuseFileName);
				forceReloadHeightMap = false;
				if (!CopyFileA("../Data/tmp/Ground.bme", levelFullPath.c_str(), false)) {
					CFileException e("Impossible de copier le fichier");
					e.m_sFileName = "../Data/tmp/Ground.bme";
					throw e;
				}
			}
		}
	}
	else {
		m_sHMFileName = string("hm_") + sFileName.substr(0, nDotPos) + ".bmp";
		CEntity::SetRessource(sFileName, bDuplicate);
	}
	m_pMesh = static_cast< IMesh* >(m_pRessource);

	try
	{
		if (m_bUseDisplacementMap) {
			int w, h;
			m_pMesh->GetTexture(0)->GetDimension(w, h);
			m_fTiling = 4.f * m_pMesh->GetBBox()->GetDimension().m_x / w;
		}
		IBox* pBox = m_pMesh->GetBBox();
		m_nHeightMapID = m_oCollisionManager.LoadHeightMap(m_sHMFileName, pBox, forceReloadHeightMap);
		m_oCollisionManager.SetGroundBoxHeight(m_nHeightMapID, m_fMapHeight);
		IHeightMap* Heightmap = m_oCollisionManager.GetHeightMap(m_nHeightMapID);
		if(sliceCount > 0)
			Heightmap->SetSliceCount(sliceCount);
	}
	catch( CFileNotFoundException& )
	{
		m_bHeightMapCreated = false;
	}
	
	string sCollisionFileName;
	if (m_bUseDisplacementMap) {
		nDotPos = (int)m_sDiffuseFileName.find('.');
		ostringstream oss;
		oss << sLevelDirectory << "collision" << m_nID << ".bmp";
		sCollisionFileName = oss.str();
	}
	else {
		nDotPos = (int)sFileName.find('.');
		sCollisionFileName = string("collision_") + sFileName.substr(0, nDotPos) + ".bmp";
	}
	
	LoadCollisionMaps();
	
	m_pGroundShader = m_oRenderer.GetShader("ground");
	m_pRessource->SetShader(m_pGroundShader);
	
	if (m_nHeightMapID != -1) {
		m_pHeightMaptexture = static_cast<ITexture*> (m_oRessourceManager.CreateTexture2D(m_sHMFileName, false));
		m_pHeightMaptexture->SetUnitTexture(4);
		m_pHeightMaptexture->SetUnitName("heightMap");
	}

	for (int i = 0; i < m_vEntityCallback.size(); i++) {
		m_vEntityCallback[i](nullptr, IEventDispatcher::TEntityEvent::T_LOAD_RESSOURCE, this);
	}
}

IGeometry* CScene::GetBoundingGeometry()
{
	IMesh* pMesh = static_cast< IMesh* >(m_pRessource);
	if(pMesh)
		return pMesh->GetBBox();
	return nullptr;
}

void CScene::CreateHeightMap()
{
	IMesh* pMesh = static_cast< IMesh* >(m_pRessource);
	ILoader::CTextureInfos ti;
	m_oCollisionManager.CreateHeightMap(pMesh, ti, IRenderer::T_BGR);
	ti.m_ePixelFormat = ILoader::eBGR;	
	m_oLoaderManager.Export(m_sHMFileName, ti);
	m_nHeightMapID = m_oCollisionManager.LoadHeightMap(m_sHMFileName, pMesh->GetBBox());
	m_bHeightMapCreated = true;
}

IEntity* CScene::Merge( string sRessourceName, float x, float y, float z )
{
	IEntity* pEntity = m_pEntityManager->CreateEntity( sRessourceName);
	pEntity->Link( this );
	pEntity->SetLocalPosition( x, y, z );
	return pEntity;
}

IEntity* CScene::Merge( string sRessourceName, CMatrix& oXForm )
{
	IEntity* pEntity = m_pEntityManager->CreateEntity(sRessourceName);
	pEntity->SetLocalMatrix( oXForm );
	pEntity->Link( this );
	return pEntity;
}

void CScene::Update()
{
	m_oTimeManager.Update();
	if (m_bDisplayMinimap) {
		RenderMinimap();
		m_oRenderer.SetCurrentFBO(0);
	}
	
	for (CLightEntity* pLightEntity : m_vShadowLights) {
		if (pLightEntity->GetLight()->IsCastShadow()) {
			pLightEntity->RenderFirstShadowPass(m_vShadowMapEntities);
			break;
		}
	}
	RenderScene();
	DispatchEntityEvent();
}

void CScene::DisplayMinimap(bool display)
{
	m_bDisplayMinimap = display;
}

void CScene::SetGroundMargin(float margin)
{
	m_fGroundMargin = margin;
}

float CScene::GetGroundMargin()
{
	return m_fGroundMargin;
}

void CScene::GetOriginalSceneFileName(string& sFileName)
{
	if (!m_sOriginalSceneFileName.empty())
		sFileName = m_sOriginalSceneFileName;
	else if(m_pRessource && !m_bUseDisplacementMap)
		m_pRessource->GetFileName(sFileName);
}

void CScene::SetOriginalSceneFileName(string sFileName)
{
	m_sOriginalSceneFileName = sFileName;
}


void CScene::SetDiffuseFileName(string diffuseFileName)
{
	m_sDiffuseFileName = diffuseFileName;
}

int CScene::GetCurrentHeightMapIndex()
{
	return m_nHeightMapID;
}

void CScene::SetLength(int length)
{
	m_nMapLength = length;
}

void CScene::SetHeight(float height)
{
	m_fMapHeight = height;
}

void CScene::SetHMFile(string sHMFile)
{
	m_sHMFileName = sHMFile;
}

void CScene::DeleteTempDirectories()
{
	string root;
	m_oFileSystem.GetLastDirectory(root);
	m_oFileSystem.DeleteDirectory(root + "/Levels/Tmp");
}

void CScene::HandleStateChanged(StateChangedCallback callback, CPlugin* pData)
{
	m_vStateChangedCallback.push_back(pair<StateChangedCallback, CPlugin*>(callback, pData));
}

void CScene::UnhandleStateChanged(StateChangedCallback callback)
{
	vector<pair<StateChangedCallback, CPlugin*>>::iterator itCallback = std::find_if(m_vStateChangedCallback.begin(), m_vStateChangedCallback.end(),
		[callback](pair<StateChangedCallback, CPlugin*>& p)
	{
		return p.first == callback; 
	});
	
	if (itCallback != m_vStateChangedCallback.end())
		m_vStateChangedCallback.erase(itCallback);
}

void CScene::SetRessourceFileName(string sNewFileName)
{
	m_pRessource->SetFileName(sNewFileName);
	string sCollisionFileName;
	CStringUtils::GetFileNameWithoutExtension(sNewFileName, sCollisionFileName);
	sCollisionFileName += "-collision.bmp";
	m_pCollisionMap->SetFileName(sCollisionFileName);
}

vector<CLightEntity*> CScene::getShadowLights()
{
	return m_vShadowLights;
}

void CScene::UpdateState()
{
	switch (m_eSceneState) {

	case eStart:
		if (IsLoadingComplete()) {
			m_eSceneState = eLoadingComplete;
			for (pair<StateChangedCallback, CPlugin*>& p : m_vStateChangedCallback)
				if (p.first)
					p.first(m_eSceneState, p.second);
			m_pPhysic->SetGravity(0);
		}
		break;
	case eLoadingComplete:
		m_eSceneState = eFirstUpdateDone;
		for (pair<StateChangedCallback, CPlugin*>& p : m_vStateChangedCallback)
			if (p.first)
				p.first(m_eSceneState, p.second);
		break;
	case eFirstUpdateDone:
		m_pPhysic->RestoreGravity();
		m_eSceneState = eRunning;
		break;
	case eRunning:
		break;
	default:
		break;
	}
}

void CScene::RemoveAllLights()
{
	m_vShadowLights.clear();
}

void CScene::AddChild(INode* pNode)
{
	CEntity::AddChild(pNode);
	CLightEntity* pLightEntity = dynamic_cast<CLightEntity*>(pNode);
	if (pLightEntity) {
		ILight* pLight = static_cast<ILight*>(pLightEntity->GetRessource());
		if (pLight && pLight->IsCastShadow()) {
			m_vShadowLights.push_back(pLightEntity);
			pLightEntity->CastShadow(true);
		}
	}
	else
	{
		CPlayer* pPlayer = dynamic_cast<CPlayer*>(pNode);
		if (pPlayer)
			m_pPlayer = pPlayer;
	}
	
#ifdef LIGHT_SUN
	if (m_pPlayer && m_pCastShadowLight && m_pCastShadowLight->GetLight()->IsSun()) {
		CMatrix sunTM;
		m_pCastShadowLight->GetLocalMatrix(sunTM);
		CVector playerPos, sunPos;
		m_pPlayer->GetWorldPosition(playerPos);
		m_pCastShadowLight->GetLocalPosition(sunPos);
		CVector z(sunPos - playerPos);
		sunTM.LookAt(z, sunPos);
		m_pCastShadowLight->SetLocalMatrix(sunTM);
		m_pCastShadowLight->Follow(m_pPlayer, m_fFarShadowFrustum / 2.f, m_fFarShadowFrustum / 2.f - 500.f);
	}
#endif // LIGHT_SUN
}

void CScene::AddCastShadowLight(CLightEntity* pLightEntity)
{
	m_vShadowLights.push_back(pLightEntity);
}

void  CScene::RenderScene()
{
	if (m_oCameraManager.GetActiveCamera()) {
		CMatrix oCamMatrix;
		m_oCameraManager.GetActiveCamera()->Update();
		m_oCameraManager.GetActiveCamera()->GetWorldMatrix(oCamMatrix);
		m_oRenderer.SetCameraMatrix(oCamMatrix);
	}
	UpdateState();

	CNode::Update();

	if (m_pEntityManager->IsUsingInstancing()) {
		RenderInstances();
		m_pEntityManager->ClearRenderQueue();
	}

	if (m_pRessource && !m_bHidden) {			
		for (CLightEntity* pLightEntity : m_vShadowLights) {
			pLightEntity->RenderSecondShadowPass(m_pGroundShader, nullptr);
		}
		
		if (m_pHeightMaptexture) {
			m_pHeightMaptexture->SetShader(m_pGroundShader);
			m_pGroundShader->SendUniformValues("groundHeight", m_fMapHeight);
			m_pGroundShader->SendUniformValues("tiling", m_fTiling);
			m_pHeightMaptexture->Update();
		}
		
		for (CScene::CTerrain* pTerrain : m_terrains) {
			pTerrain->m_pTerrainTexture->SetShader(m_pGroundShader);
			pTerrain->m_pTerrainTexture->Update();

			pTerrain->m_pSplatTexture->SetShader(m_pGroundShader);
			pTerrain->m_pSplatTexture->Update();
		}
		
		m_oRenderer.SetModelMatrix(m_oWorldMatrix);
		m_pRessource->Update();
	}
}

void CScene::RenderMinimap()
{
	// first pass
	m_oRenderer.SetCurrentFBO(m_pMinimapTexture->GetFrameBufferObjectId());
	DisplayEntitiesForMiniMap(m_vMiniMapEntities);
}

void CScene::CollectMinimapEntities(vector<IEntity*>& entities)
{
	entities.clear();
	for (int i = 0; i < GetChildCount(); i++) {
		CEntity* pEntity = dynamic_cast<CEntity*>(GetChild(i));
		if (pEntity) {
			if (pEntity != this) {
				IPlayer* pPlayer = dynamic_cast<IPlayer*>(pEntity);
				if (pPlayer)
					m_pPlayer = pEntity;
				CLightEntity* pLightEntity = dynamic_cast<CLightEntity*>(pEntity);
				if (!pLightEntity) {
					CMinimapEntity* pMapEntity = dynamic_cast<CMinimapEntity*>(pEntity);
					if (!pMapEntity) {
						CCamera* pCamera = dynamic_cast<CCamera*>(pEntity);
						if(!pCamera)
							entities.push_back(pEntity);
					}
				}
			}
		}
	}
}

void CScene::CollectShadowMapEntities(vector<IEntity*>& entities)
{
	CollectMinimapEntities(entities);
}

void CScene::OnChangeSector()
{
	CollectMinimapEntities(m_vMiniMapEntities);
	CollectShadowMapEntities(m_vShadowMapEntities);
}

void CScene::UpdateMapEntities()
{
	CollectMinimapEntities(m_vMiniMapEntities);
	CollectShadowMapEntities(m_vShadowMapEntities);
}

bool CScene::IsLoadingComplete()
{
	bool loadingComplete = true;
	for (int i = 0; i < m_vCollideEntities.size(); i++) {
		if (!m_vCollideEntities[i]->IsOnTheGround()) {
			loadingComplete = false;
			break;
		}
	}
	return loadingComplete;
}

void CScene::RenderInstances()
{
	map<IMesh*, vector<CEntity*>> entities;
	m_pEntityManager->GetInstancesTM(entities);
	map<IMesh*, vector<vector<CMatrix>>>& bonesTM = m_pEntityManager->GetInstancesBonesTM();

	map<IMesh*, vector<vector<CMatrix>>>::iterator itBones = bonesTM.begin();
	for (map<IMesh*, vector<CEntity*>>::iterator it = entities.begin(); it != entities.end(); it++) {
		vector<CMatrix> vPosTM;
		for (int i = 0; i < it->second.size(); i++)
			vPosTM.push_back(it->second[i]->GetWorldMatrix());

		IShader* pShader = nullptr;

		if ((itBones != bonesTM.end()) && (itBones->first == it->first) && (itBones->second.size() > 0))
			pShader = m_oRenderer.GetShader("SkinningInstanced");
		else
			pShader = m_oRenderer.GetShader("PerPixelLightingInstanced");

		it->first->SetShader(pShader);
		pShader->SendUniformMatrix4Array("vEntityMatrix", vPosTM, true);

		if (itBones != bonesTM.end() && (itBones->first == it->first) && itBones->second.size() > 0) {
			vector<vector<CMatrix>>& matBonesArray = itBones->second;
			for (int i = 0; i < matBonesArray.size(); i++) {
				ostringstream matBonesName;
				matBonesName << "matBones" << i;
				pShader->SendUniformMatrix4Array(matBonesName.str(), matBonesArray[i], true);
			}
			itBones++;
		}
		it->first->UpdateInstances(vPosTM.size());
	}
}

void CScene::DisplayEntitiesForMiniMap(const vector<IEntity*>& entities)
{
	if (m_pPlayer) {
		ICamera* pActiveCamera = m_oCameraManager.GetActiveCamera();
		CMatrix oCamMatrix;
		pActiveCamera->GetWorldMatrix(oCamMatrix);
		CMatrix oBackupInvCameraMatrix;
		m_oRenderer.GetInvCameraMatrix(oBackupInvCameraMatrix);
		m_oRenderer.SetCameraMatrix(oCamMatrix);
		//m_oCameraManager.SetActiveCamera(m_pMiniMapCamera);
		m_oRenderer.ClearFrameBuffer();
		IShader* pBackupShader = NULL;
		IShader* pFirstPassShader = m_oRenderer.GetShader(m_sMiniMapFirstPassShaderName);

		CMatrix m;
		m_oRenderer.SetModelMatrix(m);
		IMesh* pGround = static_cast<IMesh*>(GetRessource());
		pBackupShader = pGround->GetShader();
		pGround->SetShader(pFirstPassShader);
		pGround->Update();
		pGround->SetShader(pBackupShader);

		for (int i = 0; i < entities.size(); i++) {
			CEntity* pEntity = dynamic_cast<CEntity*>(entities[i]);
			IRessource* pRessource = pEntity->GetRessource();
			if (pRessource) {
				pBackupShader = pRessource->GetShader();
				pEntity->SetShader(pFirstPassShader);
				m_oRenderer.SetModelMatrix(pEntity->GetWorldMatrix());
				pEntity->UpdateRessource();
				pEntity->SetShader(pBackupShader);
			}
		}

		m_pPlayerMapSphere->SetLocalMatrix(m_pPlayer->GetWorldMatrix());
		m_pPlayerMapSphere->Update();

		//m_oCameraManager.SetActiveCamera(pActiveCamera);
		m_oRenderer.SetInvCameraMatrix(oBackupInvCameraMatrix);
	}
#if 0
	if (m_pPlayer) {
		ICamera* pActiveCamera = m_oCameraManager.GetActiveCamera();
		CMatrix oCamMatrix;
		m_pMiniMapCamera->SetLocalPosition(m_pPlayer->GetX(), m_pMiniMapCamera->GetY(), m_pPlayer->GetZ());
		m_pMiniMapCamera->Update();
		m_pMiniMapCamera->GetWorldMatrix(oCamMatrix);
		CMatrix oBackupInvCameraMatrix;
		m_oRenderer.GetInvCameraMatrix(oBackupInvCameraMatrix);
		m_oRenderer.SetCameraMatrix(oCamMatrix);
		m_oCameraManager.SetActiveCamera(m_pMiniMapCamera);
		m_oRenderer.ClearFrameBuffer();
		IShader* pBackupShader = NULL;
		IShader* pFirstPassShader = m_oRenderer.GetShader(m_sMiniMapFirstPassShaderName);

		CMatrix m;
		m_oRenderer.SetModelMatrix(m);
		IMesh* pGround = static_cast<IMesh*>(GetRessource());
		pBackupShader = pGround->GetShader();
		pGround->SetShader(pFirstPassShader);
		pGround->Update();
		pGround->SetShader(pBackupShader);

		for (int i = 0; i < entities.size(); i++) {
			CEntity* pEntity = dynamic_cast<CEntity*>(entities[i]);
			IRessource* pRessource = pEntity->GetRessource();
			if (pRessource) {
				pBackupShader = pRessource->GetShader();
				pEntity->SetShader(pFirstPassShader);
				m_oRenderer.SetModelMatrix(pEntity->GetWorldMatrix());
				pEntity->UpdateRessource();
				pEntity->SetShader(pBackupShader);
			}
		}

		m_pPlayerMapSphere->SetLocalMatrix(m_pPlayer->GetWorldMatrix());
		m_pPlayerMapSphere->Update();

		m_oCameraManager.SetActiveCamera(pActiveCamera);
		m_oRenderer.SetInvCameraMatrix(oBackupInvCameraMatrix);
	}
#endif // 0
}

void CScene::RenderShadowMap(const vector<IEntity*>& entities, const CMatrix& lightView, const CMatrix& lightProjection)
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
	if (!pGround)
		return;
	pBackupShader = pGround->GetShader();
	pGround->SetShader(pFirstPassShader);
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

void CScene::GetInfos( ILoader::CSceneInfos& si )
{
	si.m_bUseDisplacementMap = m_bUseDisplacementMap;	
	if (m_bUseDisplacementMap) {
		si.m_sDiffuseFileName = m_sDiffuseFileName;
		si.m_sSceneFileName = m_sHMFileName;
		si.m_nMapLength = m_nMapLength;
		si.m_fMapHeight = m_fMapHeight;
	}
	else
		m_pRessource->GetFileName(si.m_sSceneFileName);
	si.m_sOriginalSceneFileName = m_sOriginalSceneFileName;
	GetName( si.m_sName );
	m_oRenderer.GetBackgroundColor(si.m_oBackgroundColor);
	for( unsigned int i= 0; i < m_vChild.size(); i++ ) {
		CCamera* pCamera = dynamic_cast< CCamera* >(m_vChild[i]);
		if (pCamera)
			continue;
		ILoader::CObjectInfos* pInfos = nullptr;
		CLightEntity* pLightEntity = dynamic_cast< CLightEntity* >(m_vChild[i]);
		if (pLightEntity)
			pLightEntity->GetEntityInfos(pInfos);
		else {
			CEntity* pEntity = dynamic_cast< CEntity* >( m_vChild[ i ] );
			if (pEntity)
				pEntity->GetEntityInfos(pInfos);
		}
		if (pInfos)
			si.m_vObject.push_back(pInfos);
	}
}

void CScene::GetCharactersInfos(vector<IEntity*>& si, INode* pRoot)
{
	if (pRoot == nullptr)
		pRoot = this;
	for (unsigned int i = 0; i < pRoot->GetChildCount(); i++) {
		ILoader::CObjectInfos* pInfos = nullptr;
		CCharacter* pEntity = dynamic_cast< CCharacter* >(pRoot->GetChild(i));
		if (pEntity)
			si.push_back(pEntity);
		GetCharactersInfos(si, pRoot->GetChild(i));
	}
}

void CScene::LoadSceneObject( const ILoader::CObjectInfos* pSceneObjInfos, CEntity* pParent )
{
	string sRessourceFileName = pSceneObjInfos->m_sRessourceFileName;
	if( sRessourceFileName == "EE_Repere_19051978" ) {
		IEntity* pRepere = m_pEntityManager->CreateRepere( m_oRenderer );
		pRepere->Link( this );
	}
	else {
		const ILoader::CLightEntityInfos* pLightEntityInfos = dynamic_cast< const ILoader::CLightEntityInfos* >( pSceneObjInfos );
		if( pLightEntityInfos )	{
			CEntity* pEntity = m_pEntityManager->CreateLightEntity();
			pEntity->BuildFromInfos(*pLightEntityInfos, pParent);
		} else {
			const ILoader::CEntityInfos* pEntityInfos = dynamic_cast< const ILoader::CEntityInfos* >( pSceneObjInfos );
			CEntity* pEntity = m_pEntityManager->CreateEntityFromType( sRessourceFileName, pEntityInfos->m_sTypeName, pEntityInfos->m_sObjectID);
			pEntity->BuildFromInfos(*pSceneObjInfos, pParent);
			m_vCollideEntities.push_back(pEntity);
		}
	}
}

void CScene::Load( const ILoader::CSceneInfos& si )
{
	Clear();
	m_oRenderer.SetBackgroundColor(si.m_oBackgroundColor);
	if (si.m_bUseDisplacementMap) {
		m_nMapLength = si.m_nMapLength;
		m_fMapHeight = si.m_fMapHeight;
		m_sDiffuseFileName = si.m_sDiffuseFileName;
	}
	SetRessource(si.m_sSceneFileName);
	m_sOriginalSceneFileName = si.m_sOriginalSceneFileName;
	for( unsigned int i = 0; i < si.m_vObject.size(); i++ )	{
		const ILoader::CObjectInfos* pSceneObjInfos = si.m_vObject.at( i );
		LoadSceneObject( pSceneObjInfos, this );
	}
}


void CScene::Clear()
{
	m_oCameraManager.UnlinkCameras();
	for (int i = 0; i < m_vChild.size(); i++)
	{
		IEntity* pChildEntity = nullptr;
		INode* pBone = dynamic_cast<IBone*>(m_vChild[i]);
		if (!pBone)
			pChildEntity = dynamic_cast< IEntity* >( m_vChild[ i ] );

		if (pChildEntity && !dynamic_cast<CRepere*>(pChildEntity)) {
			m_vChild[i]->Unlink();
			m_pEntityManager->DestroyEntity(pChildEntity);
			i--;
		}
		else {
			if (pBone || (pChildEntity && !dynamic_cast<CRepere*>(pChildEntity))) {
				m_vChild[i]->Unlink();
				i--;
			}
		}
	}
	m_pRessource = NULL;
	m_pEntityManager->Clear();
	m_oCollisionManager.ClearHeightMaps();
	m_vCollideEntities.clear();
	ICamera* pMapCamera = m_oCameraManager.GetCameraFromType(ICameraManager::TFree);
	if(!pMapCamera)
		pMapCamera = m_oCameraManager.CreateCamera(ICameraManager::TFree, 40.f);
	pMapCamera->Link(this);
	m_eSceneState = eStart;
	m_vStateChangedCallback.clear();
	m_vShadowLights.clear();
}

void CScene::ClearCharacters()
{
	ClearCharacters(this);
}

void CScene::ClearCharacters(INode* pParent)
{
	for (int i = 0; i < pParent->GetChildCount(); i++)
	{
		INode* pChild = pParent->GetChild(i);
		IEntity* pChildCharacter = dynamic_cast< CNPCEntity* >(pChild);
		if (!pChildCharacter)
			pChildCharacter = dynamic_cast< CPlayer* >(pChild);
		if (pChildCharacter) {
			pChildCharacter->Unlink();
			m_pEntityManager->DestroyEntity(pChildCharacter);
			i--;
		}
		else
			ClearCharacters(pChild);
	}
}

float CScene::GetGroundHeight( float x, float z )
{
	if (m_nHeightMapID != -1)
		return m_oCollisionManager.GetMapHeight(m_nHeightMapID, x, z);// +margin;
	return -1000000.f;
}
