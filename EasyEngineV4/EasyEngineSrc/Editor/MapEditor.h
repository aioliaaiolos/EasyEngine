#include "EEPlugin.h"
#include "Interface.h"
#include "ILoader.h"
#include "IEditor.h"
#include "IEventDispatcher.h"
#include "SpawnableEditor.h"
#include <string>

class IInputManager;
class ICameraManager;
class IEventDispatcher;
class IRenderer;
class IEntityManager;
class IGeometryManager;
class IEntity;
class CVector;
class EEInterface;
class ILoaderManager;
class IRessourceManager;
class ICollisionManager;
class IHud;
class IScene;
class IHeightMap;
class IConsole;

//#define DEBUG_TEST_PLANE

using namespace std;

class CMapEditor : public IMapEditor, public CSpawnableEditor
{
public:

	CMapEditor(EEInterface& oInterface, ICameraManager::TCameraType cameraType);

	void							SpawnEntity(string sEntityFileName);
	void							Save(string fileName);
	void							Load(string sFileName);
	void							SetGroundAdaptationHeight(float fHeight);
	string							GetName() override;
	void							Edit(string id) override;
	void							AdaptGroundToAllEntities();
	void							SetBias(float fBias);
	void							FlushTmpFolder();
	void							SetSceneMap(string sRessourceFileName, string sDiffuseFileName, int lengh, float fHeight);

private:
		
	static void						OnSceneLoadRessource(CPlugin*, IEventDispatcher::TEntityEvent, IEntity*);
	
	void							AdaptGroundToEntity(IEntity* pEntity);
	void							OnEntityAdded();
	void							UpdateGround();
	bool							CreateLevelFolderIfNotExists(string levelName, string& levelFolder);
	void							SaveMap(string sFileName, float fBias);
	void							ClearCharacters(vector<ILoader::CObjectInfos*>& objects);
	void							OnEntitySelected() override;
	float							GetPlanHeight() override;
	void							SetEditionMode(bool bEditionMode) override;
	void							CollectSelectableEntity(vector<IEntity*>& entities) override;
	void							OnEntityRemoved(IEntity* pEntity) override;

	struct AdaptGroundThreadStruct
	{
		CMapEditor* editor;
		CVector pos;
		CVector dim;
		CMatrix entityWorldTM;
		float adaptationHeight;
		int hudMsgIdx;
	};
	static DWORD WINAPI				AdaptGround(void* params);
	void							OnEndAdaptGround(int hudMsgIdx);

	ILoaderManager&					m_oLoaderManager;
	IRessourceManager&				m_oRessourceManager;
	IFileSystem&					m_oFileSystem;
	float							m_fPlanHeight;
	ILoader::CAnimatableMeshData	m_oAnimatableMeshData;
	float							m_fGroundAdaptationHeight;
	IHeightMap*						m_pHeightMap;
	string							m_sTmpAdaptedHeightMapFileName;
	string							m_sTmpFolder;
	string							m_sCurrentMapName;
	float							m_fBias;

	static void						SaveResponseCallback(string sResponse, void* pData);	

#ifdef DEBUG_TEST_PLANE
	IEntity* m_pQuadEntity;
#endif // DEBUG_TEST_PLANE
};