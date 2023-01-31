#include "Interface.h"
#include "EntityManager.h"
#include "ICamera.h"
#include "Entity.h"
#include "Repere.h"
#include "BoxEntity.h"
#include "LightEntity.h"
#include "Exception.h"
#include "SphereEntity.h"
#include "IGeometry.h"
#include "NPCEntity.h"
#include "LineEntity.h"
#include "CylinderEntity.h"
#include "Player.h"
#include "MapEntity.h"
#include "TestEntity.h"
#include "QuadEntity.h"
#include "PlaneEntity.h"
#include "IFileSystem.h"
#include "ICollisionManager.h"
#include "Bone.h"
#include "IEditor.h"
#include "AreaEntity.h"
#include "Item.h"

#include <algorithm>

// rapidjson
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/filereadstream.h"
#include <rapidjson/prettywriter.h>
#include <fstream>
using namespace rapidjson;

CEntityManager::CEntityManager(EEInterface& oInterface):
IEntityManager(oInterface),
m_oInterface(oInterface),
m_oRessourceManager(static_cast<IRessourceManager&>(*m_oInterface.GetPlugin("RessourceManager"))),
m_oRenderer(static_cast<IRenderer&>(*m_oInterface.GetPlugin("Renderer"))),
m_oFileSystem(static_cast<IFileSystem&>(*m_oInterface.GetPlugin("FileSystem"))),
m_oCollisionManager(static_cast<ICollisionManager&>(*m_oInterface.GetPlugin("CollisionManager"))),
m_oGeometryManager(static_cast<IGeometryManager&>(*m_oInterface.GetPlugin("GeometryManager"))),
m_oPathFinder(static_cast<IPathFinder&>(*m_oInterface.GetPlugin("PathFinder"))),
m_oCameraManager(static_cast<ICameraManager&>(*m_oInterface.GetPlugin("CameraManager"))),
m_pEditorManager(nullptr),
m_nLastEntityID( -1 ),
m_pPlayer(NULL),
m_bUseInstancing(true)
{
	string root;
	m_oFileSystem.GetLastDirectory(root);
	m_sCharactersDatabaseFileName = root + "/characters.db";
	m_itCurrentParsedEntity = m_mCollideEntities.end();
	m_itCurrentIAEntity = m_mIAEntities.end();
	CCharacter::InitStatics(m_oFileSystem);
	LoadCharacterInfos();
	oInterface.HandlePluginCreation("EditorManager", HandleEditorManagerCreation, this);
	oInterface.HandlePluginCreation("EntityManager", HandleEntityManagerCreation, this);
	
}

void CEntityManager::HandleEditorManagerCreation(CPlugin* plugin, IBaseObject* pData)
{
	CEntityManager* pEntityManager = static_cast<CEntityManager*>(pData);
	pEntityManager->m_pEditorManager = static_cast<IEditorManager*>(pEntityManager->m_oInterface.GetPlugin("EditorManager"));
}

void CEntityManager::HandleEntityManagerCreation(CPlugin* plugin, IBaseObject* pData)
{
	CEntityManager* pEntityManager = static_cast<CEntityManager*>(pData);
	pEntityManager->LoadItems();
}

void CEntityManager::AddEntity( IEntity* pEntity, string sName, int id )
{
	if (id == -1)
	{
		if (m_nLastEntityID == -1)
			m_nLastEntityID = 0;
		m_nLastEntityID++;
		id = m_nLastEntityID;
	}
	m_mIDEntities[id] = pEntity;
	m_mEntitiesID[ pEntity ] = id;
	pEntity->SetID(id);
	m_mNameEntities[ sName ] = pEntity;
	m_mEntitiesName[ pEntity ] = sName;
	IAEntity* pIAEntity = dynamic_cast< IAEntity* >( pEntity );
	if( pIAEntity )
		m_mIAEntities[ pIAEntity ] = 1;
	IFighterEntity* pFighterEntity = dynamic_cast< IFighterEntity* >( pEntity );
	if( pFighterEntity )
		m_mFighterEntities[ pFighterEntity ] = 1;
}

void CEntityManager::ChangeCharacterName(string sOldName, string sNewName)
{
	std::transform(sOldName.begin(), sOldName.end(), sOldName.begin(), tolower);
	map<string, ILoader::CAnimatedEntityInfos>::iterator itCharacterInfos = m_mCharacterInfos.find(sOldName);
	if (itCharacterInfos != m_mCharacterInfos.end()) {
		ILoader::CAnimatedEntityInfos oCharacterInfos = itCharacterInfos->second;
		RemoveCharacterFromDB(sOldName);
		//m_mCharacterInfos.erase(itCharacterInfos);
		oCharacterInfos.m_sObjectName = sNewName;
		m_mCharacterInfos[sNewName] = oCharacterInfos;

		map<string, CCharacter*>::iterator itCharacter = m_mCharacters.find(sOldName);
		if (itCharacter != m_mCharacters.end()) {
			CCharacter* pCharacter = itCharacter->second;
			m_mCharacters.erase(itCharacter);
			m_mCharacters[sNewName] = pCharacter;
		}

		SaveCharacterInfosToDB(m_mCharacterInfos);
	}
	else {
		throw CEException(string("Error : character '") + sOldName + "' not found");
	}
}

CItem* CEntityManager::GetItem(string sItem)
{
	map<string, CItem*>::iterator itItem = m_mItems.find(sItem);
	if (itItem != m_mItems.end()) {
		return itItem->second;
	}
	return nullptr;
}

void CEntityManager::LoadItems()
{
	string sFileName = "items.json";
	FILE* pFile = m_oFileSystem.OpenFile(sFileName, "r");
	fclose(pFile);
	string sJsonDirectory;
	m_oFileSystem.GetLastDirectory(sJsonDirectory);
	string sFilePath = sJsonDirectory + "\\" + sFileName;

	ifstream ifs(sFilePath);
	IStreamWrapper isw(ifs);
	Document doc;
	doc.ParseStream(isw);
	if (doc.IsObject()) {
		if (doc.HasMember("Items")) {
			rapidjson::Value& items = doc["Items"];
			if (items.IsArray()) {
				int count = items.Size();
				for (int iItem = 0; iItem < count; iItem++) {
					rapidjson::Value& item = items[iItem];
					if (item.IsObject()) {
						string sId, sModel, sType, sPreview;
						if (item.HasMember("ID")) {
							rapidjson::Value& id = item["ID"];
							if (id.IsString()) {
								sId = id.GetString();
							}
						}
						if (item.HasMember("Model")) {
							rapidjson::Value& model = item["Model"];
							if (model.IsString()) {
								sModel = model.GetString();
							}
						}
						if (item.HasMember("Type")) {
							rapidjson::Value& type = item["Type"];
							if (type.IsString()) {
								sType = type.GetString();
							}
						}
						if (item.HasMember("Preview")) {
							Value& type = item["Preview"];
							if (type.IsString()) {
								sPreview = type.GetString();
							}
						}
						CItem* pItem = new CItem(m_oInterface, sId, CItem::s_mTypeString[sType], sModel, sPreview);
						m_mItems[sId] = pItem;
					}
				}
			}
		}
	}
}

CEntity* CEntityManager::CreateEntityFromType(std::string sFileName, string sTypeName, string sID, bool bDuplicate )
{
	CEntity* pEntity = NULL;
	if( sTypeName == "Entity" )
		pEntity = new CEntity(m_oInterface, sFileName, bDuplicate );
	else if( sTypeName == "Human" )
		pEntity = new CCharacter(m_oInterface, sFileName, sID);
	else if (sTypeName == "NPC")
		pEntity = new CNPCEntity(m_oInterface, sFileName, sID);
	else if (sTypeName == "Player") {
		pEntity = new CPlayer(m_oInterface, sFileName);
		IPlayer* pPlayer = dynamic_cast<IPlayer*>(pEntity);
		SetPlayer(pPlayer);
	}
	else if(sTypeName == "MapEntity")
		pEntity = new CMinimapEntity(m_oInterface, sFileName);
	else if (sTypeName == "Item") {
		pEntity = new CEntity(m_oInterface, sFileName, sID, bDuplicate);
	}
	AddEntity( pEntity, pEntity->GetEntityID());
	return pEntity;
}

IEntity* CEntityManager::CreateEntity(std::string sFileName, bool bDuplicate)
{
	if (sFileName.find(".bme") == -1)
		sFileName += ".bme";
	sFileName = string("Meshes/") + sFileName;
	CEntity* pEntity = NULL;
	pEntity = new CEntity(m_oInterface, sFileName, bDuplicate);
	return pEntity;
}

IEntity* CEntityManager::CreateObject(string sFileName)
{
	CObject* pEntity = NULL;
	pEntity = new CObject(m_oInterface, sFileName);
	string sName;
	pEntity->GetName(sName);
	AddEntity(pEntity, sName);
	return pEntity;
}

IEntity* CEntityManager::CreateEmptyEntity( string sName )
{
	CEntity* pEntity = new CEntity(m_oInterface);
	AddEntity( pEntity );
	m_mNameEntities[ sName ] = pEntity;
	m_mEntitiesName[ pEntity ] = sName;
	return pEntity;
}

CCollisionEntity* CEntityManager::CreateCollisionEntity(string sName)
{
	CCollisionEntity* pEntity = new CCollisionEntity(m_oInterface);
	m_mNameEntities[sName] = pEntity;
	m_mEntitiesName[pEntity] = sName;
	return pEntity;
}

IEntity* CEntityManager::GetEntity( int nEntityID )
{
	map< int, IEntity* >::iterator itEntity = m_mIDEntities.find( nEntityID );
	if( itEntity != m_mIDEntities.end() )
		return itEntity->second;
	return NULL;
}

IEntity* CEntityManager::GetEntity( string sEntityName )
{
	map< string, IEntity* >::iterator itEntity = m_mNameEntities.find( sEntityName );
	if( itEntity != m_mNameEntities.end() )
		return itEntity->second;
	return NULL;
}

IAEntity* CEntityManager::GetFirstIAEntity()
{
	m_itCurrentIAEntity = m_mIAEntities.begin();
	if( m_itCurrentIAEntity != m_mIAEntities.end() )
		return m_itCurrentIAEntity->first;
	return NULL;
}

IAEntity* CEntityManager::GetNextIAEntity()
{
	m_itCurrentIAEntity++;
	if( m_itCurrentIAEntity != m_mIAEntities.end() )
		return m_itCurrentIAEntity->first;
	return NULL;
}

IEntity* CEntityManager::GetFirstMobileEntity()
{
	m_itCurrentMobileEntity = m_mMobileEntity.begin();
	if( m_itCurrentMobileEntity != m_mMobileEntity.end() )
		return m_itCurrentMobileEntity->first;
	return NULL;
}

IEntity* CEntityManager::GetNextMobileEntity()
{
	m_itCurrentMobileEntity++;
	if( m_itCurrentMobileEntity != m_mMobileEntity.end() )
		return m_itCurrentMobileEntity->first;
	return NULL;
}

IEntity* CEntityManager::CreateRepere( IRenderer& oRenderer )
{
	IEntity* pEntity = new CRepere( oRenderer );
	AddEntity( pEntity, "Repere" );
	return pEntity;
}

IEntity* CEntityManager::CreateBox(const CVector& oDimension )
{
	IBox* pBox = m_oGeometryManager.CreateBox();
	pBox->Set( -oDimension / 2.f, oDimension );	
	CBoxEntity* pBoxEntity = new CBoxEntity(m_oInterface, *pBox );
	AddEntity( pBoxEntity );
	return pBoxEntity;
}

IBoxEntity* CEntityManager::CreateAreaEntity(string sAreaName, const CVector& oDimension)
{
	IBox* pBox = m_oGeometryManager.CreateBox();
	pBox->Set(-oDimension / 2.f, oDimension);
	CAreaEntity* pAreaEntity = new CAreaEntity(sAreaName, m_oInterface, *pBox);
	AddEntity(pAreaEntity, sAreaName);
	return pAreaEntity;
}

IEntity* CEntityManager::CreateMobileEntity( string sFileName, IFileSystem* pFileSystem, string sID )
{
	IEntity* pEntity = new CCharacter(m_oInterface, sFileName, sID);
	AddEntity( pEntity );
	return pEntity;
}

IEntity* CEntityManager::CreatePlaneEntity(int slices, int size, string heightTexture, string diffuseTexture)
{
	CPlaneEntity* planeEntity = new CPlaneEntity(m_oRenderer, m_oRessourceManager, slices, size, heightTexture, diffuseTexture);
	AddEntity(planeEntity, "PlaneEntity");
	return planeEntity;
}

void CEntityManager::AddNewCharacter(IEntity* pEntity)
{
	string sCharacterName, sCharacterNameLow;
	pEntity->GetEntityID(sCharacterName);
	sCharacterNameLow = sCharacterName;
	std::transform(sCharacterName.begin(), sCharacterName.end(), sCharacterNameLow.begin(), tolower);

	map<string, CCharacter*>::iterator itCharacter = m_mCharacters.find(sCharacterNameLow);
	if (itCharacter != m_mCharacters.end())
		throw CCharacterAlreadyExistsException(sCharacterNameLow);
	CCharacter* pCharacter = dynamic_cast<CCharacter*>(pEntity);
	m_mCharacters[sCharacterNameLow] = pCharacter;
}

ICharacter* CEntityManager::BuildCharacterFromDatabase(string sCharacterId, IEntity* pParent)
{
	CEntity* pEntity = nullptr;
	map<string, ILoader::CAnimatedEntityInfos>::iterator itCharacter = m_mCharacterInfos.find(sCharacterId);
	if (itCharacter == m_mCharacterInfos.end()) {
		string sCharacterIdLow = sCharacterId;
		std::transform(sCharacterId.begin(), sCharacterId.end(), sCharacterIdLow.begin(), tolower);
		itCharacter = m_mCharacterInfos.find(sCharacterIdLow);
	}
	if (itCharacter != m_mCharacterInfos.end()) {
		pEntity = CreateEntityFromType(itCharacter->second.m_sRessourceFileName, itCharacter->second.m_sTypeName, sCharacterId);
		pEntity->BuildFromInfos(itCharacter->second, dynamic_cast<CEntity*>(pParent));
	}
	
	return dynamic_cast<ICharacter*>(pEntity);
}

void CEntityManager::GetCharacterInfosFromDatabase(string sCharacterId, ILoader::CAnimatedEntityInfos& infos)
{
	CEntity* pEntity = nullptr;
	map<string, ILoader::CAnimatedEntityInfos>::iterator itCharacter = m_mCharacterInfos.find(sCharacterId);
	if (itCharacter == m_mCharacterInfos.end()) {
		ostringstream oss;
		oss << "CEntityManager::GetCharacterInfosFromDatabase() : Erreur : CEntityManager::BuildCharacterFromDatabase() -> id " 
			<< sCharacterId << " inexistant dans la base de donneees des personnages.";
		CEException e(oss.str());
		throw e;
	}
	else {
		infos = itCharacter->second;
	}	
}


void CEntityManager::NormalizeCharacterDatabase()
{
	//for (map<string, ILoader::CAnimatedEntityInfos>::iterator it = m_mCharacterInfos.begin();	it != m_mCharacterInfos.end(); it++) {
	map<string, ILoader::CAnimatedEntityInfos>::iterator it = m_mCharacterInfos.begin();
	while(it != m_mCharacterInfos.end()) {
		string sNameLow = it->first;
		std::transform(it->first.begin(), it->first.end(), sNameLow.begin(), tolower);
		if (sNameLow != it->first) {
			m_mCharacterInfos[sNameLow] = it->second;
			m_mCharacterInfos[sNameLow].m_sObjectName = sNameLow;
			m_mCharacterInfos.erase(it);
			it = m_mCharacterInfos.begin();
		}
		else
			it++;
	}
	SaveCharacterInfosToDB(m_mCharacterInfos);
}

void CEntityManager::SetPlayer(IPlayer* player)
{
	m_pPlayer = dynamic_cast<CPlayer*>(player);
	ICamera* pCamera = m_oCameraManager.GetCameraFromType(ICameraManager::TLinked);
	m_oCameraManager.SetActiveCamera(pCamera);
	pCamera->Link(m_pPlayer);
}

IPlayer* CEntityManager::GetPlayer()
{
	return m_pPlayer;
}

ICharacter* CEntityManager::CreateNPC( string sFileName, string sID )
{
	string sName = sFileName;
	if (sName.find(".bme") == -1)
		sName += ".bme";
	sName = string("Meshes/Bodies/") + sName;
	ICharacter* pEntity = new CNPCEntity(m_oInterface, sName, sID);
	AddEntity( pEntity, sID );
	ICharacterEditor* pCharacterEditor = dynamic_cast<ICharacterEditor*>(m_pEditorManager->GetEditor(IEditor::Type::eCharacter));
	if (pCharacterEditor->IsEnabled()) {
		pCharacterEditor->SetCurrentEditableNPC(pEntity);
	}
	return pEntity;
}

IPlayer* CEntityManager::CreatePlayer(string sFileName)
{
	string sName = sFileName;
	if (sName.find(".bme") == -1)
		sName += ".bme";
	sName = string("Meshes/Bodies/") + sName;
	IPlayer* pEntity = new CPlayer(m_oInterface, sName);
	AddEntity(pEntity, "Player");
	ICharacterEditor* pCharacterEditor = dynamic_cast<ICharacterEditor*>(m_pEditorManager->GetEditor(IEditor::Type::eCharacter));
	if (pCharacterEditor->IsEnabled()) {
		pCharacterEditor->SetCurrentEditablePlayer(pEntity);
	}
	return pEntity;
}

IEntity* CEntityManager::CreateMinimapEntity(string sFileName, IFileSystem* pFileSystem)
{
	string sName = sFileName;
	if (sName.find(".bme") == -1)
		sName += ".bme";
	IEntity* pEntity = new CMinimapEntity(m_oInterface, sName);
	AddEntity(pEntity);
	return pEntity;
}

IEntity* CEntityManager::CreateTestEntity(string sFileName, IFileSystem* pFileSystem)
{
	string sName = sFileName;
	if (sName.find(".bme") == -1)
		sName += ".bme";
	IEntity* pEntity = new CTestEntity(m_oInterface, sName);
	AddEntity(pEntity);
	return pEntity;
}

void CEntityManager::GetCharactersName(vector<string>& vCharactersName)
{
	for (map<string, ILoader::CAnimatedEntityInfos>::iterator itCharacter = m_mCharacterInfos.begin(); itCharacter != m_mCharacterInfos.end(); itCharacter++)
		vCharactersName.push_back(itCharacter->first);
}

int	CEntityManager::GetEntityID( IEntity* pEntity )
{
	if (pEntity) {
		map< IEntity*, int >::iterator itEntity = m_mEntitiesID.find(pEntity);
		if (itEntity != m_mEntitiesID.end())
			return itEntity->second;
		AddEntity(pEntity);
		return pEntity->GetID();
	}
	return -1;
}

int CEntityManager::GetEntityCount()
{
	return (int)m_mEntitiesID.size();
}

CEntity* CEntityManager::CreateLightEntity()
{
	CLightEntity* pLightEntity = new CLightEntity(m_oInterface, nullptr);
	AddEntity(pLightEntity);
	return pLightEntity;
}

ILightEntity* CEntityManager::CreateLightEntity( CVector Color, IRessource::TLight type, float fIntensity )
{
	IRessource* pLight = m_oRessourceManager.CreateLight(Color, type, fIntensity);
	CLightEntity* pLightEntity = new CLightEntity(m_oInterface, pLight);
	AddEntity( pLightEntity );
	return pLightEntity;
}

float CEntityManager::GetLightIntensity(int nID)
{
	CLightEntity* pLightEntity = dynamic_cast< CLightEntity* >(m_mIDEntities[nID]);
	if (!pLightEntity)
	{
		CBadTypeException e;
		throw e;
	}
	return pLightEntity->GetIntensity();
}

void CEntityManager::SetLightIntensity( int nID, float fIntensity )
{
	CLightEntity* pLightEntity = dynamic_cast< CLightEntity* >( m_mIDEntities[ nID ] );
	if( !pLightEntity )
	{
		CBadTypeException e;
		throw e;
	}
	pLightEntity->SetIntensity( fIntensity );
}

void CEntityManager::DestroyEntity( IEntity* pEntity )
{
	map< IEntity*, int >::iterator itEntityID = m_mEntitiesID.find( pEntity );
	if( itEntityID != m_mEntitiesID.end() )
	{
		map< int, IEntity* >::iterator itIDEntity = m_mIDEntities.find( itEntityID->second );
		m_mEntitiesID.erase( itEntityID );
		m_mIDEntities.erase( itIDEntity );
		map< IEntity*, string >::iterator itEntityName = m_mEntitiesName.find( pEntity );
		if( itEntityName != m_mEntitiesName.end() )
		{
			map< string, IEntity* >::iterator itNameEntity = m_mNameEntities.find( itEntityName->second );
			if( itNameEntity != m_mNameEntities.end() )			
				m_mNameEntities.erase( itNameEntity );
			m_mEntitiesName.erase( itEntityName );
		}
		delete pEntity;
	}
}

void CEntityManager::Clear()
{
	m_mIDEntities.clear();
	m_mEntitiesID.clear();
	m_mNameEntities.clear();
	m_mEntitiesName.clear();
	m_mCharacters.clear();
	m_nLastEntityID = -1;
	m_pPlayer = nullptr;
	m_oRessourceManager.RemoveAllLights();
	m_mCollideEntities.clear();
	m_mFighterEntities.clear();
}

void CEntityManager::DestroyAll()
{
	map< int, IEntity* >::iterator itIDEntity = m_mIDEntities.begin();
	while( itIDEntity != m_mIDEntities.end() )
	{
		IEntity* pEntity = itIDEntity->second;
		DestroyEntity( pEntity );
		itIDEntity = m_mIDEntities.begin();
	}
}

IEntity* CEntityManager::CreateSphere( float fSize )
{
	IEntity* pSphere = CreateEntity("sphere.bme", "");
	pSphere->SetScaleFactor( fSize, fSize, fSize );
	AddEntity( pSphere, "Sphere" );
	return pSphere;
}

IEntity* CEntityManager::CreateQuad(float lenght, float width)
{
	IQuad* pQuad = m_oGeometryManager.CreateQuad(lenght, width);
	CQuadEntity* pQuadEntity = new CQuadEntity(m_oRenderer, m_oRessourceManager, *pQuad);
	AddEntity(pQuadEntity, "Quad");
	return pQuadEntity;
}

void CEntityManager::AddCollideEntity( CEntity* pEntity )
{
	int nID = (int)m_mCollideEntities.size();
	m_mCollideEntities[ pEntity ] = nID;
}

void CEntityManager::RemoveCollideEntity( CEntity* pEntity )
{
	map< CEntity*, int >::iterator itEntity = m_mCollideEntities.find(pEntity );
	if( itEntity != m_mCollideEntities.end() )
		m_mCollideEntities.erase( itEntity );
}

CEntity* CEntityManager::GetFirstCollideEntity()
{
	m_itCurrentParsedEntity = m_mCollideEntities.begin();
	if( m_itCurrentParsedEntity != m_mCollideEntities.end() )
		return m_itCurrentParsedEntity->first;
	return NULL;
}

CEntity* CEntityManager::GetNextCollideEntity()
{
	m_itCurrentParsedEntity++;
	if( m_itCurrentParsedEntity != m_mCollideEntities.end() )
		return m_itCurrentParsedEntity->first;
	return NULL;
}

int CEntityManager::GetCollideEntityID( CEntity* pEntity )
{
	map< CEntity*, int >::iterator itEntityID = m_mCollideEntities.find( pEntity );
	if( itEntityID != m_mCollideEntities.end() )
		return itEntityID->second;
	return -1;
}

IFighterEntity* CEntityManager::GetFirstFighterEntity()
{
	m_itCurrentFighterEntity = m_mFighterEntities.begin();
	if( m_itCurrentFighterEntity != m_mFighterEntities.end() )
		return m_itCurrentFighterEntity->first;
	return NULL;
}

IFighterEntity* CEntityManager::GetNextFighterEntity()
{
	m_itCurrentFighterEntity++;
	if( m_itCurrentFighterEntity != m_mFighterEntities.end() )
		return m_itCurrentFighterEntity->first;
	return NULL;
}

IEntity* CEntityManager::CreateLineEntity( const CVector& first, const CVector& last )
{
	CLineEntity* line = new CLineEntity(m_oRenderer, first, last);
	AddEntity(line, "LineEntity");
	return line;
}

IEntity* CEntityManager::CreateCylinder( float fRadius, float fHeight )
{
	return new CCylinderEntity(m_oInterface, fRadius, fHeight );
}

void CEntityManager::SetGUIManager(IGUIManager* pGUIManager)
{
	m_pGUIManager = pGUIManager;
}

IGUIManager* CEntityManager::GetGUIManager()
{
	return m_pGUIManager;
}

void CEntityManager::Kill(int entityId)
{
	CCharacter* pEntity = dynamic_cast<CCharacter*>(GetEntity(entityId));
	if (pEntity) {
		pEntity->SetLife(0);
		pEntity->Die();
	}
}


void CEntityManager::WearArmorToDummy(int entityId, string sArmorName)
{
	CCharacter* pEntity = dynamic_cast<CCharacter*>(GetEntity(entityId));
	if (pEntity)
		pEntity->WearArmorToDummy(sArmorName);
	else {
		ostringstream oss;
		oss << "Erreur dans CEntityManager::WearArmorToDummy() : entite " << entityId << " introuvable";
		CEException e(oss.str());
		throw e;
	}
}

void CEntityManager::SaveCharacterToDB(string sNPCID)
{
	string sNPCIDLow = sNPCID;
	std::transform(sNPCID.begin(), sNPCID.end(), sNPCIDLow.begin(), tolower);
	map<string, CCharacter*>::iterator itNPC = m_mCharacters.find(sNPCIDLow);
	if (itNPC != m_mCharacters.end()) {
		ILoader::CObjectInfos* pInfos = nullptr;
		itNPC->second->GetEntityInfos(pInfos);
		LoadCharacterInfos();
		ILoader::CAnimatedEntityInfos* pAnimatedEntity = static_cast<ILoader::CAnimatedEntityInfos*>(pInfos);
		m_mCharacterInfos[sNPCIDLow] = *pAnimatedEntity;
		SaveCharacterInfosToDB(m_mCharacterInfos);
		delete pAnimatedEntity;
	}
	else {
		throw CEException("CEntityManager::SaveCharacterToDB : Unable to save character because it's not loaded");
	}
}

void CEntityManager::LoadCharacterInfoFromJson(map<string, ILoader::CAnimatedEntityInfos>& mCharacterInfos)
{
	string sFileName = "characters.json";
	string root;
	IFileSystem* pFileSystem = static_cast<IFileSystem*>(m_oInterface.GetPlugin("FileSystem"));
	pFileSystem->GetLastDirectory(root);
	string sFilePath = root + "/" + sFileName;

	std::ifstream ifs(sFilePath);
	IStreamWrapper isw(ifs);
	Document doc;
	doc.ParseStream(isw);
	if (doc.IsObject() && doc.HasMember("Characters")) {
		Value characters;
		characters = doc["Characters"];
		if (characters.IsArray()) {
			for (int i = 0; i < characters.GetArray().Size(); i++) {
				Value character;
				character = characters[i];
				if (character.IsObject()) {
					if (character.HasMember("Name")) {
						ILoader::CAnimatedEntityInfos infos;
						infos.m_sObjectName = character["Name"].GetString();
						infos.m_sRessourceFileName = character["RessourceFileName"].GetString();
						infos.m_sParentName = character["ParentName"].GetString();
						infos.m_nParentBoneID = character["ParentBoneID"].GetInt();
						infos.m_sTypeName = character["TypeName"].GetString();
						infos.m_fWeight = character["Weight"].GetFloat();
						infos.m_nGrandParentDummyRootID = character["GrandParentDummyRootID"].GetInt();
						infos.m_sTextureName = character["DiffuseTextureName"].GetString();
						Value& specular = character["Specular"].GetArray();
						infos.m_vSpecular = CVector(specular[0].GetFloat(), specular[1].GetFloat(), specular[2].GetFloat());
						Value& animationSpeeds = character["AnimationSpeeds"];
						for (int iSpeed = 0; iSpeed < animationSpeeds.Size(); iSpeed++)
							infos.m_mAnimationSpeed[animationSpeeds[iSpeed]["Name"].GetString()] = animationSpeeds[iSpeed]["Speed"].GetFloat();
						Value& items = character["Items"];
						for (int iItem = 0; iItem < items.Size(); iItem++) {
							Value& item = items[iItem];
							string itemName = item["ItemName"].GetString();
							Value& isWearArray = item["IsWearArray"];							
							for (int j = 0; j < isWearArray.Size(); j++) {
								int isWear = isWearArray[j].GetInt();
								infos.m_mItems[itemName].push_back(isWear);
							}
						}
						infos.m_sHairs = character["Hairs"].GetString();
						mCharacterInfos[infos.m_sObjectName] = infos;
					}
				}
			}
		}
	}
}


void CEntityManager::LoadCharacterInfos()
{
	m_mCharacterInfos.clear();
	LoadCharacterInfoFromJson(m_mCharacterInfos);
}

void CEntityManager::LoadCharacterInfoFromDB()
{
	m_mCharacterInfos.clear();
	CBinaryFileStorage fs;
	if (!fs.OpenFile(m_sCharactersDatabaseFileName, IFileStorage::eRead)) {
		fs.OpenFile(m_sCharactersDatabaseFileName, IFileStorage::eWrite);
		fs << 0;
		fs.CloseFile();
		fs.OpenFile(m_sCharactersDatabaseFileName, IFileStorage::eRead);
	}
	int characterCount;
	fs >> characterCount;
	for (int i = 0; i < characterCount; i++) {
		ILoader::CAnimatedEntityInfos infos;
		int type;
		fs >> type;
		fs >> infos;
		m_mCharacterInfos[infos.m_sObjectName] = infos;
	}
	fs.CloseFile();
}

void CEntityManager::SaveCharacterInfosToDB(const map<string, ILoader::CAnimatedEntityInfos>& characterInfos)
{
	CBinaryFileStorage fs;
	if (fs.OpenFile(m_sCharactersDatabaseFileName, IFileStorage::eWrite)) {
		fs << (int)characterInfos.size();
		for (map<string, ILoader::CAnimatedEntityInfos>::const_iterator it = characterInfos.begin(); it != characterInfos.end(); it++)
			fs << it->second;
		fs.CloseFile();
	}
	else {
		ostringstream oss;
		oss << "Erreur : CEntityManager::LoadCharacterInfos() -> \"" << m_sCharactersDatabaseFileName << "\" introuvable";
		CEException e(oss.str());
		throw e;
	}
}

void CEntityManager::RemoveCharacterFromDB(string sID)
{
	LoadCharacterInfos();
	m_mCharacterInfos.erase(sID);
}

void CEntityManager::AddRenderQueue(INode* pNode)
{
	CEntity* pEntity = dynamic_cast<CEntity*>(pNode);
	IMesh* pMesh = pEntity ? pEntity->GetMesh() : nullptr;
	if (pMesh) {
		m_mRenderQueue[pMesh].push_back(pEntity);
		if (pEntity->GetSkeletonRoot()) {
			vector<CMatrix> vBonesMatrix;
			pEntity->GetBonesMatrix(vBonesMatrix);
			m_mBonesMatrixQueue[pMesh].push_back(vBonesMatrix);
		}
	}
	else {
		for (int i = 0; i < pEntity->GetChildCount(); i++) {
			INode* pChild = pEntity->GetChild(i);
			AddRenderQueue(pChild);
		}
	}
}

void CEntityManager::GetInstancesTM(map<IMesh*, vector<CEntity*>>& instances)
{
	instances = m_mRenderQueue;
}

map<IMesh*, vector<vector<CMatrix>>>& CEntityManager::GetInstancesBonesTM()
{
	return m_mBonesMatrixQueue;
}

void CEntityManager::ClearRenderQueue()
{
	m_mRenderQueue.clear();
	m_mBonesMatrixQueue.clear();
}

template<class T>
void CEntityManager::SerializeNodeInfos(INode* pNode, ostringstream& oss, int nLevel)
{
	IEntity* pEntity = dynamic_cast< T* >(pNode);
	if (pEntity) {
		
		for (int j = 0; j < nLevel; j++)
			oss << "\t";
		string sEntityName;
		pEntity->GetEntityID(sEntityName);
		if (sEntityName.empty())
			pEntity->GetName(sEntityName);
		oss << "Entity name = " << sEntityName << ", ID = " << GetEntityID(pEntity) << "\n";
		IBone* pSkeleton = pEntity->GetSkeletonRoot();
		if (pSkeleton)
			SerializeNodeInfos<T>(pSkeleton, oss);
	}
	for (unsigned int i = 0; i < pNode->GetChildCount(); i++)
		SerializeNodeInfos<T>(pNode->GetChild(i), oss, nLevel + 1);
}

void CEntityManager::SerializeMobileEntities(INode* pRoot, string& sText)
{
	ostringstream oss;
	SerializeNodeInfos<CCharacter>(pRoot, oss, 0);
	sText = oss.str();
}

string CEntityManager::GetName()
{
	return "EntityManager";
}

IBone* CEntityManager::CreateBone() const
{
	return new CBone(m_oGeometryManager);
}

void CEntityManager::EnableInstancing(bool enable)
{
	m_bUseInstancing = enable;
}

bool CEntityManager::IsUsingInstancing()
{
	return m_bUseInstancing;
}


extern "C" _declspec(dllexport) IEntityManager* CreateEntityManager(EEInterface& oInterface)
{
	return new CEntityManager(oInterface);
}