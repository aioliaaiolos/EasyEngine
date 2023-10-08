#ifndef SCENE_NODE_H
#define SCENE_NODE_H

// stl
#include <vector>

// Engine
#include "Math/Matrix.h"
#include "EEPlugin.h"
#include "Entity.h"

class CNode;
class IRessourceManager;
class IRenderer;
class CFreeCamera;
class ICamera;
class IEntityManager;
class ICollisionManager;
class IPathFinder;
class CSphereEntity;

using namespace std;

class CScene : public CEntity, public IScene
{
public:
	struct Desc
	{
		IRessourceManager&	m_oRessourceManager;
		IRenderer&			m_oRenderer;
		std::string			m_sFileName;
		ICamera*			m_pCamera;
		ICameraManager&		m_oCameraManager;
		IEntityManager*		m_pEntityManager;
		ILoaderManager&		m_oLoaderManager;
		ICollisionManager&	m_oCollisionManager;
		IGeometryManager&	m_oGeometryManager;
		Desc(	IRessourceManager& oRessourceManager, IRenderer& pRenderer, IEntityManager* pEntityManager, 
				ICamera* pCamera, ICameraManager& oCameraManager, ILoaderManager& oLoaderManager, 
				ICollisionManager& oCollisionManager, IGeometryManager& oGeometryManager, IPathFinder& oPathFinder);
	};

	enum TSceneMode
	{
		eNormalMode = 0,
		eMapMode
	};

	CScene(EEInterface& oInterface, string ressourceFileName, string diffuseFileName);
	~CScene();
	
	IEntity*													Merge( string sRessourceName, float x, float y, float z ) override;
	IEntity*													Merge( string sRessourceName, CMatrix& oXForm );
	void														Update();
	void														Clear();
	void														ClearCharacters();
	void														ClearCharacters(INode* pParent);
	float														GetGroundHeight( float x, float z );
	void														SetRessource( string sFileName, bool bDuplicate = false );
	IGeometry*													GetBoundingGeometry();
	void														RenderScene();
	void														RenderMinimap();
	void														RenderMinimap2();
	void														RenderShadowMap();
	ITexture*													CreateMinimapTexture();
	ITexture*													CreateMinimap2Texture();
	ITexture*													CreateShadowMapTexture();
	ITexture*													GetMinimapTexture();
	ITexture*													GetMinimapTexture2();
	ITexture*													GetShadowMapTexture();
	void														DisplayMinimap(bool display);
	void														DisplayMinimap2(bool display);
	void														DisplayShadowMap(bool display);
	void														SetGroundMargin(float margin);
	float														GetGroundMargin();
	void														GetOriginalSceneFileName(string& sFileName);
	void														SetOriginalSceneFileName(string sFileName);
	void														SetDiffuseFileName(string diffuseFileName) override;
	int															GetCurrentHeightMapIndex() override;
	void														SetLength(int length) override;
	void														SetHeight(float height) override;
	void														SetHMFile(string sHMFile) override;
	void														DeleteTempDirectories() override;
	void														HandleStateChanged(StateChangedCallback callback, CPlugin* pPlugin) override;
	void														UnhandleStateChanged(IScene::StateChangedCallback callback) override;
	void														SetRessourceFileName(string sNewFileName) override;

private:


	void														GetInfos(ILoader::CSceneInfos& si);
	void														GetCharactersInfos(vector<IEntity*>& si, INode* pRoot = nullptr) override;
	void														Load(const ILoader::CSceneInfos& si);
	void														LoadSceneObject(const ILoader::CObjectInfos* pSceneObjInfos, CEntity* pParent);
	void														CreateHeightMap();
	void														CollectMinimapEntities(vector<IEntity*>& entities);
	void														CollectShadowMapEntities(vector<IEntity*>& entities);
	void														DisplayEntitiesForMiniMap(const vector<IEntity*>& entities);
	void														DisplayEntitiesForMiniMap2(const vector<IEntity*>& entities);
	void														DisplayEntitiesForShadowMap(const vector<IEntity*>& entities);
	void														DisplayEntitiesForShadowMapTest(const vector<IEntity*>& entities);
	void														OnChangeSector() override;
	void														UpdateMapEntities();
	bool														IsLoadingComplete();
	void														RenderInstances();
	void														UpdateState();


	ICameraManager&												m_oCameraManager;
	ILoaderManager&												m_oLoaderManager;
	ICollisionManager&											m_oCollisionManager;
	IRessourceManager&											m_oRessourceManager;
	IFileSystem&												m_oFileSystem;
	int															m_nHeightMapID;
	string														m_sHMFileName;
	ILoader::CTextureInfos										m_oCollisionMap;
	bool														m_bHeightMapCreated;
	ICamera*													m_pMiniMapCamera = nullptr;
	ICamera*													m_pShadowMapCamera = nullptr;
	const string												m_sMiniMapFirstPassShaderName;
	const string												m_sMiniMap2FirstPassShaderName;
	const string												m_sMiniMapSecondPassShaderName;
	const string												m_sShadowMapFirstPassShaderName;
	const string												m_sShadowMapSecondPassShaderName;
	ITexture*													m_pMinimapTexture;
	ITexture*													m_pMinimapTexture2;
	ITexture*													m_pShadowMapTexture;
	vector<IEntity*>											m_vMiniMapEntities;
	vector<IEntity*>											m_vShadowMapEntities;
	CEntity*													m_pPlayer;
	CEntity*													m_pPlayerMapSphere;
	bool														m_bDisplayMinimap;
	bool														m_bDisplayMinimap2 = false;
	bool														m_bDisplayShadowMap = false;
	float														m_fGroundMargin;
	string														m_sOriginalSceneFileName;
	ITexture*													m_pHeightMaptexture;
	IShader*													m_pGroundShader;
	bool														m_bUseDisplacementMap;
	float														m_fDisplacementRatioHeightSize;
	float														m_fTiling;
	string														m_sDiffuseFileName;
	int															m_nMapLength;
	float														m_fMapHeight;
	vector<CEntity*>											m_vCollideEntities;
	vector<pair<StateChangedCallback, CPlugin*>>				m_vStateChangedCallback;
	string														m_sCurrentLevelName;
	TSceneState													m_eSceneState;
	CEntity*													m_pCastedLight = nullptr;

	CMatrix														m_oCameraTM;
};

#endif // SCENE_NODE_H