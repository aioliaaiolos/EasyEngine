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

// rapidjson
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/filereadstream.h"
#include <fstream>
#include <rapidjson/prettywriter.h>
using namespace rapidjson;

CWorldEditor::CWorldEditor(EEInterface& oInterface, ICameraManager::TCameraType cameraType) :
	CPlugin(nullptr, ""),
	ISpawnableEditor(oInterface),
	IWorldEditor(oInterface),
	CSpawnableEditor(oInterface, cameraType),
	m_oSceneManager(static_cast<ISceneManager&>(*oInterface.GetPlugin("SceneManager"))),
	m_oFileSystem(static_cast<IFileSystem&>(*oInterface.GetPlugin("FileSystem"))),
	m_oRessourceManager(static_cast<IRessourceManager&>(*oInterface.GetPlugin("RessourceManager")))
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
	m_mAreaMatrices.clear();
	m_mItemMatrices.clear();
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
			entityName = pEntity->GetIDStr();
			map<string, pair<CMatrix, string>>::iterator itCharacter = m_mCharacterMatrices.find(entityName);
			if (itCharacter != m_mCharacterMatrices.end()) {
				m_oEntityManager.RemoveCharacterFromWorld(itCharacter->first);
				m_mCharacterMatrices.erase(itCharacter);
			}
			else {
				map<string, pair<IBoxEntity*, pair<CMatrix, CVector>>>::iterator itArea = m_mAreaMatrices.find(entityName);
				if (itArea != m_mAreaMatrices.end())
					m_mAreaMatrices.erase(itArea);
				else {
					map<string, vector<pair<IItem*, CMatrix>>>::iterator itItem = m_mItemMatrices.find(entityName);
					if (itItem != m_mItemMatrices.end()) {
						vector<pair<IItem*, CMatrix>>& vMatrices = itItem->second;
						vector<pair<IItem*, CMatrix>>::iterator itMat = vMatrices.begin();
						while (itMat != vMatrices.end()) {
							if (itMat->first == pEntity)
								itMat = vMatrices.erase(itMat);
							else
								itMat++;
						}
					}
					else {
						ostringstream oss;
						oss << "CWorldEditor::OnEntityRemoved() : Erreur, entite " << entityName << " introuvable";
						throw CEException(oss.str());
					}
				}
			}			
		}
	}
	
}

string CWorldEditor::GetName()
{
	return "WorldEditor";
}

void CWorldEditor::Load(string fileName)
{
	LoadFromJson(fileName);
}

void CWorldEditor::LoadFromDB(string fileName)
{
	m_sCurrentWorldName = fileName;
	GetRelativeDatabasePath(fileName, fileName, "db");

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
			string entityName = pEntity->GetIDStr();
			map<string, pair<CMatrix, string>>::iterator itCharacter = m_mCharacterMatrices.find(entityName);
			itCharacter->second.first = pEntity->GetWorldMatrix();
		}
		Save(fileName);
		m_mCharacterMatrices = mBackupCharacterMatrices;
	}
}

void CWorldEditor::Save(string fileName)
{
	SaveToJson(fileName);
}

void CWorldEditor::SaveToDB(string fileName)
{
	GetRelativeDatabasePath(fileName, fileName, "db");
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
			for (int iMatrix = 0; iMatrix < it->second.size(); iMatrix++) {
				if (it->second[iMatrix].first)
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

void ConvertVectorToJsonArray(rapidjson::Document& doc, const CVector& v, rapidjson::Value& vector)
{
	vector.SetArray();
	Value x, y, z;
	x.SetFloat(v.m_x);
	y.SetFloat(v.m_y);
	z.SetFloat(v.m_z);
	vector.PushBack(x, doc.GetAllocator());
	vector.PushBack(y, doc.GetAllocator());
	vector.PushBack(z, doc.GetAllocator());
}

void ConvertMatrixToJsonObject(rapidjson::Document& doc, const CMatrix& m, rapidjson::Value& matrix)
{
	matrix.SetArray();
	float pData[16];
	m.Get(pData);
	for (int i = 0; i < 16; i++) {
		Value val;
		val.SetFloat(pData[i]);
		matrix.PushBack(val, doc.GetAllocator());
	}
}

void CWorldEditor::LoadFromJson(string sFileName)
{
	m_sCurrentWorldName = sFileName;
	GetRelativeDatabasePath(sFileName, sFileName, "json");

	m_pEditorManager->CloseAllEditorButThis(this);
	ClearWorld();

	std::ifstream ifs(sFileName);
	IStreamWrapper isw(ifs);
	Document doc;
	doc.ParseStream(isw);
	if (doc.IsObject() && doc.HasMember("World")) {
		Value& world = doc["World"];
		Value& maps = world["Maps"];
		for (int iMap = 0; iMap < maps.Size(); iMap++) {
			Value& map = maps[iMap];
			CVector vPosition;
			Value& position = map["Position"];			
			vPosition = CVector(position[0].GetFloat(), position[1].GetFloat(), position[2].GetFloat());
			m_mMaps[map["Name"].GetString()] = vPosition;
		}

		if (m_mMaps.size() > 0) {
			string root;
			m_oFileSystem.GetLastDirectory(root);
			IMapEditor* pMapEditor = dynamic_cast<IMapEditor*>(m_pEditorManager->GetEditor(IEditor::Type::eMap));
			pMapEditor->Load(m_mMaps.begin()->first);
			m_pScene->HandleStateChanged(HandleSceneLoadingComplete, this);
		}

		if (world.HasMember("Characters")) {
			Value& characters = world["Characters"];
			for (int iCharacter = 0; iCharacter < characters.Size(); iCharacter++) {
				Value& character = characters[iCharacter];
				CMatrix oTM;
				Value& worldTM = character["WorldTM"];
				vector<float> vTM;
				for (int iTM = 0; iTM < worldTM.Size(); iTM++)
					vTM.push_back(worldTM[iTM].GetFloat());
				oTM.Set(vTM);
				m_mCharacterMatrices[character["Name"].GetString()] = pair<CMatrix, string>(oTM, character["Script"].GetString());
			}
		}

		if (world.HasMember("StaticEntities")) {
			Value& staticEntities = world["StaticEntities"];
			for (int iEntity = 0; iEntity < staticEntities.Size(); iEntity++) {
				Value& entity = staticEntities[iEntity];
				string sEntityName = entity["Name"].GetString();
				Value& worldTMs = entity["WorldTMs"];
				for (int iTM = 0; iTM < worldTMs.Size(); iTM++) {
					Value& tm = worldTMs[iTM];
					vector<float> vMatrix;
					for (int i = 0; i < tm.Size(); i++)
						vMatrix.push_back(tm[i].GetFloat());
					CMatrix oTM;
					oTM.Set(vMatrix);
					m_mEntityMatrices[sEntityName].push_back(pair<IEntity*, CMatrix>(nullptr, oTM));
				}
			}
		}

		if (world.HasMember("Areas")) {
			Value& areas = world["Areas"];
			for (int iArea = 0; iArea < areas.Size(); iArea++) {
				Value& area = areas[iArea];
				string sName = area["Name"].GetString();
				Value& worldTM = area["WorldTM"];
				vector<float> vMatrix;
				for (int iTM = 0; iTM < worldTM.Size(); iTM++)
					vMatrix.push_back(worldTM[iTM].GetFloat());
				CMatrix oWorldTM;
				oWorldTM.Set(vMatrix);
				vector<float> vDim;
				Value& dimension = area["Dimension"];
				CVector oDimension = CVector(dimension[0].GetFloat(), dimension[1].GetFloat(), dimension[2].GetFloat());
				m_mAreaMatrices[sName] = pair<IBoxEntity*, pair<CMatrix, CVector>>{ nullptr, {oWorldTM, oDimension} };
			}
		}
		
		if (world.HasMember("Items")) {
			Value& staticEntities = world["Items"];
			for (int iEntity = 0; iEntity < staticEntities.Size(); iEntity++) {
				Value& entity = staticEntities[iEntity];
				string sEntityID = entity["ID"].GetString();
				Value& worldTMs = entity["WorldTMs"];
				for (int iTM = 0; iTM < worldTMs.Size(); iTM++) {
					Value& tm = worldTMs[iTM];
					vector<float> vMatrix;
					for (int i = 0; i < tm.Size(); i++)
						vMatrix.push_back(tm[i].GetFloat());
					CMatrix oTM;
					oTM.Set(vMatrix);
					m_mItemMatrices[sEntityID].push_back(pair<IItem*, CMatrix>(nullptr, oTM));
				}
			}
		}
	}

	m_pEditorCamera->Link(m_pScene);

	if (m_bEditionMode)
		InitCamera();
}

void CWorldEditor::SaveToJson(string sFileName)
{
	GetRelativeDatabasePath(sFileName, sFileName, "json");
	CopyFile(sFileName.c_str(), (sFileName + ".bak").c_str(), FALSE);
	rapidjson::Document doc;	

	std::ifstream ifs(sFileName);

	if (!ifs.eof())
	{
		rapidjson::IStreamWrapper isw(ifs);
		doc.ParseStream(isw);

		if (!doc.IsObject())
			doc.SetObject();
		Value world(kObjectType);

		Value maps(kArrayType);
		for (map<string, CVector>::iterator itMap = m_mMaps.begin(); itMap != m_mMaps.end(); itMap++) {
			rapidjson::Value map(kObjectType), mapName(kStringType), mapPosition(kArrayType), x, y, z;
			mapName.SetString(itMap->first.c_str(), doc.GetAllocator());
			x.SetFloat(itMap->second.m_x);
			y.SetFloat(itMap->second.m_y);
			z.SetFloat(itMap->second.m_z);
			mapPosition.PushBack(x, doc.GetAllocator());
			mapPosition.PushBack(y, doc.GetAllocator());
			mapPosition.PushBack(z, doc.GetAllocator());
			map.AddMember("Name", mapName, doc.GetAllocator());
			map.AddMember("Position", mapPosition, doc.GetAllocator());
			maps.PushBack(map, doc.GetAllocator());
		}

		Value characters(kArrayType);
		for (map<string, pair<CMatrix, string>>::iterator it = m_mCharacterMatrices.begin(); it != m_mCharacterMatrices.end(); it++) {
			IEntity* pEntity = m_oEntityManager.GetEntity(it->first);
			if (pEntity) {
				Value name(kStringType), tm, script, character(kObjectType);
				name.SetString(it->first.c_str(), doc.GetAllocator());
				ConvertMatrixToJsonObject(doc, pEntity->GetWorldMatrix(), tm);
				script.SetString(pEntity->GetAttachedScript().c_str(), doc.GetAllocator());
				character.AddMember("Name", name, doc.GetAllocator());
				character.AddMember("WorldTM", tm, doc.GetAllocator());
				character.AddMember("Script", script, doc.GetAllocator());
				characters.PushBack(character, doc.GetAllocator());
			}
		}	

		Value entities(kArrayType);
		for (map<string, vector<pair<IEntity*, CMatrix>>>::iterator it = m_mEntityMatrices.begin(); it != m_mEntityMatrices.end(); it++) {
			Value entity(kObjectType), name(kStringType), tms(kArrayType);
			name.SetString(it->first.c_str(), doc.GetAllocator());
			
			for (int iMatrix = 0; iMatrix < it->second.size(); iMatrix++) {
				if (it->second[iMatrix].first) {
					Value tm;
					ConvertMatrixToJsonObject(doc, it->second[iMatrix].first->GetWorldMatrix(), tm);
					tms.PushBack(tm, doc.GetAllocator());
				}
			}
			entity.AddMember("Name", name, doc.GetAllocator());
			entity.AddMember("WorldTMs", tms, doc.GetAllocator());
			entities.PushBack(entity, doc.GetAllocator());
		}

		Value areas(kArrayType);
		for (map<string, pair<IBoxEntity*, pair<CMatrix, CVector>>>::iterator it = m_mAreaMatrices.begin(); it != m_mAreaMatrices.end(); it++) {			
			IBoxEntity* pBoxEntity = it->second.first;
			Value name, tm, dim;
			name.SetString(it->first.c_str(), doc.GetAllocator());
			ConvertMatrixToJsonObject(doc, pBoxEntity->GetWorldMatrix(), tm);
			ConvertVectorToJsonArray(doc, pBoxEntity->GetBox().GetDimension(), dim);
			Value area(kObjectType);
			area.AddMember("Name", name, doc.GetAllocator());
			area.AddMember("WorldTM", tm, doc.GetAllocator());
			area.AddMember("Dimension", dim, doc.GetAllocator());
			areas.PushBack(area, doc.GetAllocator());
		}

		Value items(kArrayType);
		for (map<string, vector<pair<IItem*, CMatrix>>>::iterator it = m_mItemMatrices.begin(); it != m_mItemMatrices.end(); it++) {
			Value item(kObjectType), id(kStringType), tms(kArrayType);
			id.SetString(it->first.c_str(), doc.GetAllocator());
			for (int iMatrix = 0; iMatrix < it->second.size(); iMatrix++) {
				if (it->second[iMatrix].first) {
					Value tm;
					ConvertMatrixToJsonObject(doc, it->second[iMatrix].first->GetWorldMatrix(), tm);
					tms.PushBack(tm, doc.GetAllocator());
				}
			}
			item.AddMember("ID", id, doc.GetAllocator());
			item.AddMember("WorldTMs", tms, doc.GetAllocator());
			items.PushBack(item, doc.GetAllocator());
		}

		world.AddMember("Maps", maps, doc.GetAllocator());
		world.AddMember("Characters", characters, doc.GetAllocator());
		world.AddMember("StaticEntities", entities, doc.GetAllocator());
		world.AddMember("Areas", areas, doc.GetAllocator());
		world.AddMember("Items", items, doc.GetAllocator());

		if (doc.HasMember("World"))
			doc["World"] = world;
		else
			doc.AddMember("World", world, doc.GetAllocator());

		rapidjson::StringBuffer buffer;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
		writer.SetIndent('\t', 1);
		doc.Accept(writer);
		std::ofstream ofs(sFileName);
		ofs << buffer.GetString();
		ofs.close();
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

	string sIDLow = sID;
	std::transform(sID.begin(), sID.end(), sIDLow.begin(), tolower);
	m_pEditingEntity = m_oEntityManager.BuildCharacterFromDatabase(sIDLow, m_pScene);
	if (!m_pEditingEntity) {
		ostringstream oss;
		oss << "Erreur : Personnage " << sIDLow << " introuvable dans la base de donnees des personnages.";
		throw CEException(oss.str());
	}
	m_oEntityManager.AddEntity(m_pEditingEntity, sIDLow);
	InitSpawnedEntity();
	CMatrix m;
	m_mCharacterMatrices[sIDLow].first = m;
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

int CWorldEditor::SpawnItem(string sItemName)
{
	if (!m_bEditionMode)
		throw CEException("You first have to setup in editor mode by editing a world before to spawn an item");
	IItem* pItem = m_oEntityManager.CreateItemEntity(sItemName);
	if (pItem) {
		pItem->Load();
		m_pEditingEntity = pItem;
		InitSpawnedEntity();
		m_oCameraManager.SetActiveCamera(m_pEditorCamera);
		m_eEditorMode = TEditorMode::eAdding;
		m_vEntities.push_back(m_pEditingEntity);
		CMatrix m;
		m_mItemMatrices[sItemName].push_back(pair<IItem*, CMatrix>(pItem, m));
	}
	else {
		throw CEException("Error in CWorldEditor::SpawnItem() : item '" + sItemName + "' not found in 'items.json'");
	}
	return -1;
}

void CWorldEditor::LockEntity(string sEntityID)
{
	IEntity* pEntity = m_oEntityManager.GetEntity(sEntityID);
	if (pEntity) {

	}
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

void CWorldEditor::GetRelativeDatabasePath(string worldName, string& path, string sExtension)
{
	if (!worldName.empty() && worldName.find("." + sExtension) == -1)
		worldName += "." + sExtension;
	if (worldName.empty())
		worldName = m_sDatabaseFileName + "." + sExtension;

	string root;
	m_oFileSystem.GetLastDirectory(root);
	path = root + "/" + worldName;
}

void CWorldEditor::OnSceneLoaded()
{
	if (!m_bLoadMapLights) {
		for (unsigned int i = 0; i < m_pScene->GetChildCount(); i++) {
			ILightEntity* pLightEntity = dynamic_cast<ILightEntity*>(m_pScene->GetChild(i));
			if (pLightEntity) {
				pLightEntity->Unlink();
				delete pLightEntity;
			}
		}
		m_oRessourceManager.RemoveAllLights(m_oRenderer);
	}

	for (map<string, pair<CMatrix, string>>::iterator itCharacter = m_mCharacterMatrices.begin(); itCharacter != m_mCharacterMatrices.end(); itCharacter++) {
		IEntity* pEntity = m_oEntityManager.BuildCharacterFromDatabase(itCharacter->first, m_pScene);
		if (pEntity) {
			pEntity->SetLocalMatrix(itCharacter->second.first);
			if(!itCharacter->second.second.empty())
				pEntity->AttachScriptFunction(itCharacter->second.second);
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
	for (map<string, vector<pair<IItem*, CMatrix>>>::iterator itItem = m_mItemMatrices.begin(); itItem != m_mItemMatrices.end(); itItem++) {
		vector<pair<IItem*, CMatrix>>& vMatrices = itItem->second;
		for (int i = 0; i < vMatrices.size(); i++) {
			IItem* pItem = m_oEntityManager.CreateItemEntity(string(itItem->first));
			if (pItem) {
				pItem->Load();
				pItem->Link(m_pScene);
				vMatrices[i].first = pItem;
				pItem->SetLocalMatrix(vMatrices[i].second);
				pItem->SetWeight(1);
				m_vEntities.push_back(pItem);
			}
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
}
