#pragma once

#include "IEditor.h"
#include "SpawnableEditor.h"
#include "../Utils/Math/Matrix.h"

class IFileSystem;
class ISceneManager;

class CWorldEditor : public IWorldEditor, public CSpawnableEditor
{
public:
	CWorldEditor(EEInterface& oInterface, ICameraManager::TCameraType cameraType);
	;
	void																Load(string fileName) override;
	void																Edit(string worldName) override;
	void																HandleMapLoaded(string sMapName);
	IEntity*															SpawnCharacter(string sID) override;
	int																	SpawnArea(string areaName) override;
	int																	SpawnItem(string areaName) override;
	void																LockEntity(string sEntityID) override;

private:
	void																ClearWorld();
	void																OnEntityAdded() override;
	float																GetPlanHeight() override;
	void																OnEntitySelected() override;
	void																OnEntityRemoved(IEntity* pEntity) override;
	string																GetName() override;
	void																Save(string sFileName) override;
	void																LoadFromDB(string fileName);
	void																SaveToDB(string fileName);
	void																LoadFromJson(string sFileName);
	void																SaveToJson(string fileName);
	void																Save();
	void																SaveGame(string fileName) override;	
	void																SpawnEntity(string sFileName) override;
	void																RemoveCharacter(string sID) override;
	void																SetEditionMode(bool bEditionMode) override;
	void																CollectSelectableEntity(vector<IEntity*>& entities) override;
	void																GetRelativeDatabasePath(string worldName, string& path, string sExtension);
	void																OnSceneLoaded();
	IRessourceManager&													m_oRessourceManager;
	static void															HandleSceneLoadingComplete(IScene::TSceneState state, CPlugin* pWorldEditor);

	IFileSystem&														m_oFileSystem;
	ISceneManager&														m_oSceneManager;
	map<string, CVector>												m_mMaps;
	map<string, pair<CMatrix, string>>									m_mCharacterMatrices; // (Nom, (TM, Script))
	map<string, vector<pair<IEntity*, CMatrix>>>						m_mEntityMatrices;
	map<string, pair<IBoxEntity*, pair<CMatrix, CVector>>>				m_mAreaMatrices;
	map<string, vector<pair<IItem*, CMatrix>>>							m_mItemMatrices;
	vector<IEntity*>													m_vEntities;
	string																m_sCurrentWorldName;
	const string														m_sDatabaseFileName = "world";
	bool																m_bLoadMapLights = true;
};