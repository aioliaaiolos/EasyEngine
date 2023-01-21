#include "WorldEditor.h"
#include "IEntity.h"
#include "Interface.h"
#include "IFileSystem.h"
#include "ICamera.h"
#include "IGeometry.h"
#include "IConsole.h"
#include "EditorManager.h"
#include "CharacterEditor.h"
#include "../Utils2/StringUtils.h"

#include <algorithm>

CWorldEditor::CWorldEditor(EEInterface& oInterface, ICameraManager::TCameraType cameraType) :
	CPlugin(nullptr, ""),
	ISpawnableEditor(oInterface),
	IWorldEditor(oInterface),
	CSpawnableEditor(oInterface, cameraType),
	m_oSceneManager(static_cast<ISceneManager&>(*oInterface.GetPlugin("SceneManager"))),
	m_oFileSystem(static_cast<IFileSystem&>(*oInterface.GetPlugin("FileSystem")))	
{
	m_eEditingMode = TEditingType::eXForm;
	m_pScene = m_oSceneManager.GetScene("Game");
}

void CWorldEditor::HandleMapLoaded(string sMapName)
{
	m_mMaps[sMapName] = CVector();
}

void CWorldEditor::ClearWorld()
{
	CCharacterEditor* pCharacterEditor = dynamic_cast<CCharacterEditor*>(m_pEditorManager->GetEditor(IEditor::Type::eCharacter));
	if (pCharacterEditor)
	{
		pCharacterEditor->OnEditorExit();
	}
	m_pScene->Clear();
	m_vEntities.clear();
	m_mCharacterMatrices.clear();
	m_mEntityMatrices.clear();
	m_oEntityManager.Clear();
	m_pEditingEntity = nullptr;
	m_mMaps.clear();
}

void CWorldEditor::OnEntityAdded()
{
	map<string, vector<pair<IEntity*, CMatrix>>>::iterator itEntity = m_mEntityMatrices.find(m_pEditingEntity->GetName());
	if (itEntity != m_mEntityMatrices.end()) {
		vector<pair<IEntity*, CMatrix>>& vMatrices = itEntity->second;
		vMatrices.back().first = m_pEditingEntity;
		vMatrices.back().second = m_pEditingEntity->GetWorldMatrix();
	}
}

float CWorldEditor::GetPlanHeight()
{
	if (m_pEditingEntity) {
		ICamera* pCamera = m_oCameraManager.GetActiveCamera();
		CVector camPos;
		pCamera->GetWorldPosition(camPos);
		IBox* pBox = dynamic_cast<IBox*>(m_pEditingEntity->GetBoundingGeometry());
		if (pBox)
			return camPos.m_y - 200.f;
	}
	return 0.f;
}

void CWorldEditor::OnEntitySelected()
{

}

void CWorldEditor::OnEntityRemoved(IEntity* pEntity)
{
	string entityName;
	IRessource* pRessource = pEntity->GetRessource();
	if (pRessource) {
		pRessource->GetFileName(entityName);
		string prefix = "Meshes/";
		entityName = entityName.substr(prefix.size());
		map<string, vector<pair<IEntity*, CMatrix>>>::iterator itEntity = m_mEntityMatrices.find(entityName);
		if (itEntity != m_mEntityMatrices.end()) {
			vector<pair<IEntity*, CMatrix>>& vMatrices = itEntity->second;
			vector<pair<IEntity*, CMatrix>>::iterator itMat = vMatrices.begin();
			while (itMat != vMatrices.end()) {
				if (itMat->first == pEntity)
					itMat = vMatrices.erase(itMat);
				else
					itMat++;
			}

		}
		else {
			pEntity->GetEntityName(entityName);
			map<string, pair<CMatrix, string>>::iterator itCharacter = m_mCharacterMatrices.find(entityName);
			if (itCharacter != m_mCharacterMatrices.end())
				m_mCharacterMatrices.erase(itCharacter);
			else {
				ostringstream oss;
				oss << "CWorldEditor::OnEntityRemoved() : Erreur, entite " << entityName << " introuvable";
				throw CEException(oss.str());
			}
		}
	}
	else {
		pEntity->GetEntityName(entityName);
		map<string, pair<IBoxEntity*, pair<CMatrix, CVector>>>::iterator itArea = m_mAreaMatrices.find(entityName);
		if (itArea != m_mAreaMatrices.end())
			m_mAreaMatrices.erase(itArea);
		else {
			ostringstream oss;
			oss << "CWorldEditor::OnEntityRemoved() : Erreur, entite " << entityName << " introuvable";
			throw CEException(oss.str());
		}
	}
}

string CWorldEditor::GetName()
{
	return "WorldEditor";
}

void CWorldEditor::Load(string fileName)
{
	m_sCurrentWorldName = fileName;
	GetRelativeDatabasePath(fileName, fileName);

	m_pEditorManager->CloseAllEditorButThis(this);
	ClearWorld();

	CBinaryFileStorage fs;
	if (fs.OpenFile(fileName, IFileStorage::TOpenMode::eRead)) {
		int mapCount = 0;
		fs >> mapCount;
		for (int i = 0; i < mapCount; i++) {
			string mapName;
			CVector pos;
			fs >> mapName >> pos;
			m_mMaps[mapName] = pos;
		}
		if (m_mMaps.size() > 0) {
			string root;
			m_oFileSystem.GetLastDirectory(root);
			IMapEditor* pMapEditor = dynamic_cast<IMapEditor*>(m_pEditorManager->GetEditor(IEditor::Type::eMap));
			m_pScene->HandleStateChanged(HandleSceneLoadingComplete, this);
			pMapEditor->Load(m_mMaps.begin()->first);
		}

		int characterCount = 0;
		fs >> characterCount;
		for (int i = 0; i < characterCount; i++) {
			string id;
			CMatrix tm;
			string script;
			fs >> id >> tm;
			fs >> script;
			m_mCharacterMatrices[id].first = tm;
			m_mCharacterMatrices[id].second = script;
		}
		int entityCount = 0;
		fs >> entityCount;
		for (int i = 0; i < entityCount; i++) {
			string sFileName;
			int instanceCount = 0;
			fs >> sFileName >> instanceCount;
			for (int iInstance = 0; iInstance < instanceCount; iInstance++) {
				CMatrix tm;
				fs >> tm;
				m_mEntityMatrices[sFileName].push_back(pair<IEntity*, CMatrix>(nullptr, tm));
			}
		}
		int areaCount = 0;
		fs >> areaCount;
		for (int i = 0; i < areaCount; i++) {
			string sName;
			CMatrix tm;
			CVector dim;
			fs >> sName >> tm >> dim;
			m_mAreaMatrices[sName] = pair<IBoxEntity*, pair<CMatrix, CVector>>(nullptr, pair<CMatrix, CVector>(tm, dim));
		}
	}
	m_pEditorCamera->Link(m_pScene);

	if (m_bEditionMode)
		InitCamera();
}

void CWorldEditor::Save()
{
	Save(m_sCurrentWorldName);
}


void CWorldEditor::SaveGame(string fileName)
{
	if (!m_bEditionMode) {
		string fileNameLower = fileName;;
		transform(fileName.begin(), fileName.end(), fileNameLower.begin(), tolower);
		if (fileNameLower == "world" || fileNameLower == "world.db") {
			throw CEException("Erreur : Impossible de sauvegarder une partie dans un fichier qui se nomme 'world.db'");
		}
		vector<IEntity*> entities;
		m_pScene->GetCharactersInfos(entities);
		map<string, pair<CMatrix, string>> mBackupCharacterMatrices = m_mCharacterMatrices;
		for (IEntity* pEntity : entities) {
			string entityName;
			pEntity->GetEntityName(entityName);
			map<string, pair<CMatrix, string>>::iterator itCharacter = m_mCharacterMatrices.find(entityName);
			itCharacter->second.first = pEntity->GetWorldMatrix();
		}
		Save(fileName);
		m_mCharacterMatrices = mBackupCharacterMatrices;
	}
}

void CWorldEditor::Save(string fileName)
{
	GetRelativeDatabasePath(fileName, fileName);
	CopyFile(fileName.c_str(), (fileName + ".bak").c_str(), FALSE);

	CBinaryFileStorage fs;
	if (fs.OpenFile(fileName, IFileStorage::TOpenMode::eWrite)) {
		fs << (int)m_mMaps.size();
		for (map<string, CVector>::iterator itMap = m_mMaps.begin(); itMap != m_mMaps.end(); itMap++)
			fs << itMap->first << itMap->second;
		fs << (int)m_mCharacterMatrices.size();
		for (map<string, pair<CMatrix, string>>::iterator it = m_mCharacterMatrices.begin(); it != m_mCharacterMatrices.end(); it++) {
			IEntity* pEntity = m_oEntityManager.GetEntity(it->first);
			if (pEntity) {
				fs << it->first << pEntity->GetWorldMatrix();
				fs << pEntity->GetAttachedScript();
			}
			else {
				ostringstream oss;
				oss << "Erreur : CWorldEditor::Save() -> Personnage " << it->first << " introuvable dans l'EntityManager";
				throw CEException(oss.str());
			}
		}
		fs << (int)m_mEntityMatrices.size();
		for (map<string, vector<pair<IEntity*, CMatrix>>>::iterator it = m_mEntityMatrices.begin(); it != m_mEntityMatrices.end(); it++) {
			fs << it->first;
			fs << (int)it->second.size();
			for(int iMatrix = 0; iMatrix < it->second.size(); iMatrix++) {
				if(it->second[iMatrix].first)
					fs << it->second[iMatrix].first->GetWorldMatrix();
			}
		}
		fs << (int)m_mAreaMatrices.size();
		for (map<string, pair<IBoxEntity*, pair<CMatrix, CVector>>>::iterator it = m_mAreaMatrices.begin(); it != m_mAreaMatrices.end(); it++) {
			const string& name = it->first;
			fs << name;
			IBoxEntity* pBoxEntity = it->second.first;
			if (pBoxEntity) {
				fs << pBoxEntity->GetWorldMatrix();
				fs << pBoxEntity->GetBox().GetDimension();
			}
		}
	}
}

void CWorldEditor::SpawnEntity(string sFileName)
{
	if(!m_bEditionMode)
		SetEditionMode(true);

	string relativePath = string("Meshes/") + sFileName;
	if(m_pEditingEntity)
		m_pEditingEntity->DrawBoundingBox(false);
	m_pEditingEntity = m_oEntityManager.CreateObject(relativePath);
	m_pEditingEntity->SetName(sFileName);
	m_oEntityManager.AddEntity(m_pEditingEntity, relativePath);
	InitSpawnedEntity();
	CMatrix m;
	m_mEntityMatrices[sFileName].push_back(pair<IEntity*, CMatrix>(nullptr, m));
	m_vEntities.push_back(m_pEditingEntity);
	m_eEditorMode = TEditorMode::eAdding;
}

void CWorldEditor::RemoveCharacter(string sID)
{
	m_mCharacterMatrices.erase(sID);
	IEntity* pCharacter = m_oEntityManager.GetEntity(sID);
	vector<IEntity*>::iterator itCharacter = std::find(m_vEntities.begin(), m_vEntities.end(), pCharacter);
	m_vEntities.erase(itCharacter);
}

IEntity* CWorldEditor::SpawnCharacter(string sID)
{
	if (!m_bEditionMode)
		SetEditionMode(true);

	m_pEditingEntity = m_oEntityManager.BuildCharacterFromDatabase(sID, m_pScene);
	if (!m_pEditingEntity) {
		ostringstream oss;
		oss << "Erreur : Personnage " << sID << " introuvable dans la base de donnees des personnages.";
		throw CEException(oss.str());
	}
	m_oEntityManager.AddEntity(m_pEditingEntity, sID);
	InitSpawnedEntity();
	CMatrix m;
	m_mCharacterMatrices[sID].first = m;
	m_vEntities.push_back(m_pEditingEntity);
	m_oCameraManager.SetActiveCamera(m_pEditorCamera);
	m_eEditorMode = TEditorMode::eAdding;
	return m_pEditingEntity;
}

int CWorldEditor::SpawnArea(string areaName)
{
	if (!m_bEditionMode)
		SetEditionMode(true);
	m_pEditingEntity = m_oEntityManager.CreateAreaEntity(areaName, CVector(1000, 1000, 1000));
	InitSpawnedEntity();
	m_oCameraManager.SetActiveCamera(m_pEditorCamera);
	m_eEditorMode = TEditorMode::eAdding;
	m_vEntities.push_back(m_pEditingEntity);
	CMatrix m;
	CVector dim;
	IBoxEntity* pAreaEntity = dynamic_cast<IBoxEntity*>(m_pEditingEntity);
	m_mAreaMatrices[areaName] = pair<IBoxEntity*, pair<CMatrix, CVector>>(pAreaEntity, pair<CMatrix, CVector>(m, dim));
	return -1;
}

void CWorldEditor::SetEditionMode(bool bEditionMode)
{
	//if (m_bEditionMode != bEditionMode) {
		CEditor::SetEditionMode(bEditionMode);
		if (bEditionMode) {
			m_pScene->Clear();
			Load(m_sCurrentWorldName);
		}
		else {
			// Ask to save world
		}
	//}
}

void CWorldEditor::CollectSelectableEntity(vector<IEntity*>& entities)
{
	entities = m_vEntities;
}

void CWorldEditor::Edit(string worldName)
{
	if (m_sCurrentWorldName != worldName)
		m_bEditionMode = false;
	m_sCurrentWorldName = worldName;
	CSpawnableEditor::SetEditionMode(true);
	SetEditionMode(true);
}

void CWorldEditor::GetRelativeDatabasePath(string worldName, string& path)
{
	if (!worldName.empty() && worldName.find(".db") == -1)
		worldName += ".db";
	if (worldName.empty())
		worldName = m_sDatabaseFileName;

	string root;
	m_oFileSystem.GetLastDirectory(root);
	path = root + "/" + worldName;
}

/*
void CWorldEditor::SetEntitiesWeight()
{
	for (IEntity* pEntity : m_vEntities) {
		IBoxEntity* pArea = dynamic_cast<IBoxEntity*>(pEntity);
		if (!pArea) {
			pEntity->SetWeight(1);
		}
	}
}*/

void CWorldEditor::OnSceneLoaded()
{
	for (map<string, pair<CMatrix, string>>::iterator itCharacter = m_mCharacterMatrices.begin(); itCharacter != m_mCharacterMatrices.end(); itCharacter++) {
		IEntity* pEntity = m_oEntityManager.BuildCharacterFromDatabase(itCharacter->first, m_pScene);
		if (pEntity) {
			pEntity->SetLocalMatrix(itCharacter->second.first);
			if(!itCharacter->second.second.empty())
				pEntity->AttachScript(itCharacter->second.second);
			pEntity->SetWeight(1);
			m_vEntities.push_back(pEntity);
		}
		else {
			string log = string("Error : character '") + itCharacter->first + "' not found in characters database";
			throw CEException(log);
		}
	}
	for (map<string, vector<pair<IEntity*, CMatrix>>>::iterator itEntity = m_mEntityMatrices.begin(); itEntity != m_mEntityMatrices.end(); itEntity++) {
		vector<pair<IEntity*, CMatrix>>& vMatrices = itEntity->second;
		for (int i = 0; i < vMatrices.size(); i++) {
			IEntity* pEntity = m_oEntityManager.CreateObject(string("Meshes/") + itEntity->first);
			if (pEntity) {
				pEntity->Link(m_pScene);
				vMatrices[i].first = pEntity;
				pEntity->SetLocalMatrix(vMatrices[i].second);
				pEntity->SetWeight(1);
				m_vEntities.push_back(pEntity);
			}
		}
	}
	for (map<string, pair<IBoxEntity*, pair<CMatrix, CVector>>>::iterator itArea = m_mAreaMatrices.begin(); itArea != m_mAreaMatrices.end(); itArea++) {
		pair<IBoxEntity*, pair<CMatrix, CVector>>& area = itArea->second;
		IBoxEntity* pAreaEntity = m_oEntityManager.CreateAreaEntity(itArea->first, area.second.second);
		if (pAreaEntity) {
			pAreaEntity->Link(m_pScene);
			area.first = pAreaEntity;
			CMatrix& oTM = area.second.first;
			CVector& dim = area.second.second;
			CVector minPoint = - dim / 2.f;
			pAreaEntity->SetLocalMatrix(oTM);
			pAreaEntity->GetBox().Set(minPoint, dim);
			pAreaEntity->Update();
			m_vEntities.push_back(pAreaEntity);
		}
	}
	if(m_bEditionMode)
		m_oCameraManager.SetActiveCamera(m_pEditorCamera);
}

void CWorldEditor::HandleSceneLoadingComplete(IScene::TSceneState state, CPlugin* pWorldEditorData)
{	
	CWorldEditor* pWorldEditor = dynamic_cast<CWorldEditor*>(pWorldEditorData);
	try {
		if (pWorldEditor) {
			switch (state)
			{
			case IScene::eStart:
				break;
			case IScene::eLoadingComplete:
				pWorldEditor->OnSceneLoaded();
				pWorldEditor->m_pScene->OnChangeSector();
				break;
			case IScene::eFirstUpdateDone:
				//pWorldEditor->SetEntitiesWeight();
				break;
			default:
				break;
			}
		}
	}
	catch (CEException& e)
	{
		pWorldEditor->m_oConsole.Println(e.what());
	}
	//pWorldEditor->m_pScene->UnhandleStateChanged();
	
}
