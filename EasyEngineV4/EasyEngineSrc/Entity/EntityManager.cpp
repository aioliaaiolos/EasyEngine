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
	CMobileEntity::InitStatics();
	LoadCharacterInfos();
	oInterface.HandlePluginCreation("EditorManager", HandleEditorManagerCreation, this);
}

void CEntityManager::HandleEditorManagerCreation(CPlugin* plugin, void* pData)
{
	CEntityManager* pEntityManager = static_cast<CEntityManager*>(pData);
	pEntityManager->m_pEditorManager = static_cast<IEditorManager*>(pEntityManager->m_oInterface.GetPlugin("EditorManager"));
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

CEntity* CEntityManager::CreateEntityFromType(std::string sFileName, string sTypeName, string sID, bool bDuplicate )
{
	CEntity* pEntity = NULL;
	if( sTypeName == "Entity" )
		pEntity = new CEntity(m_oInterface, sFileName, bDuplicate );
	else if( sTypeName == "Human" )
		pEntity = new CMobileEntity(m_oInterface, sFileName, sID);
	else if (sTypeName == "NPC")
		pEntity = new CNPCEntity(m_oInterface, sFileName, sID);
	else if (sTypeName == "Player") {
		pEntity = new CPlayer(m_oInterface, sFileName);
		IPlayer* pPlayer = dynamic_cast<IPlayer*>(pEntity);
		SetPlayer(pPlayer);
	}
	else if(sTypeName == "MapEntity")
		pEntity = new CMinimapEntity(m_oInterface, sFileName);

	string sName;
	pEntity->GetEntityName( sName );
	AddEntity( pEntity, sName );
	return pEntity;
}

IEntity* CEntityManager::CreateEntity(std::string sFileName, bool bDuplicate)
{
	if (sFileName.find(".bme") == -1)
		sFileName += ".bme";
	sFileName = string("Meshes/") + sFileName;
	CEntity* pEntity = NULL;
	pEntity = new CEntity(m_oInterface, sFileName, bDuplicate);
	string sName;
	pEntity->GetName(sName);
	AddEntity(pEntity, sName);
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
	CBoxEntity* pBoxEntity = new CBoxEntity(m_oRenderer, *pBox );
	AddEntity( pBoxEntity );
	return pBoxEntity;
}

ISphere& CEntityManager::GetSphere( IEntity* pEntity )
{
	CSphereEntity* pSphereEntity = static_cast< CSphereEntity* >( pEntity );
	return pSphereEntity->GetSphere();
}

IBox& CEntityManager::GetBox( IEntity* pEntity )
{
	CBoxEntity* pBoxEntity = static_cast< CBoxEntity* >( pEntity );
	return pBoxEntity->GetBox();
}

IEntity* CEntityManager::CreateMobileEntity( string sFileName, IFileSystem* pFileSystem, string sID )
{
	IEntity* pEntity = new CMobileEntity(m_oInterface, sFileName, sID);
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
	string sCharacterName;
	pEntity->GetEntityName(sCharacterName);
	map<string, CMobileEntity*>::iterator itCharacter = m_mCharacters.find(sCharacterName);
	if (itCharacter != m_mCharacters.end())
		throw CCharacterAlreadyExistsException(sCharacterName);
	CMobileEntity* pCharacter = dynamic_cast<CMobileEntity*>(pEntity);
	AddEntity(pCharacter, "Player");
	m_mCharacters[sCharacterName] = pCharacter;
}

ICharacter* CEntityManager::BuildCharacterFromDatabase(string sCharacterId, IEntity* pParent)
{
	CEntity* pEntity = nullptr;
	map<string, ILoader::CAnimatedEntityInfos>::iterator itCharacter = m_mCharacterInfos.find(sCharacterId);
	if (itCharacter == m_mCharacterInfos.end()) {
		ostringstream oss;
		oss << "Erreur : CEntityManager::BuildCharacterFromDatabase() -> id " << sCharacterId << " inexistant dans la base de donneees des personnages.";
		CEException e(oss.str());
	}
	else {
		pEntity = CreateEntityFromType(itCharacter->second.m_sRessourceFileName, itCharacter->second.m_sTypeName, sCharacterId);
		pEntity->BuildFromInfos(itCharacter->second, dynamic_cast<CEntity*>(pParent));
	}
	return dynamic_cast<ICharacter*>(pEntity);
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
	AddEntity(pEntity);
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
	map< IEntity*, int >::iterator itEntity = m_mEntitiesID.find( pEntity );
	if( itEntity != m_mEntitiesID.end() )
		return itEntity->second;
	AddEntity(pEntity);
	return pEntity->GetID();
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

IEntity* CEntityManager::CreateLightEntity( CVector Color, IRessource::TLight type, float fIntensity )
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

void CEntityManager::SetZCollisionError( float e )
{
	CBody::SetZCollisionError( e );
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
	CMobileEntity* pEntity = dynamic_cast<CMobileEntity*>(GetEntity(entityId));
	if (pEntity)
		pEntity->Die();
}


void CEntityManager::WearArmorToDummy(int entityId, string sArmorName)
{
	CMobileEntity* pEntity = dynamic_cast<CMobileEntity*>(GetEntity(entityId));
	if (pEntity)
		pEntity->WearArmorToDummy(sArmorName);
	else {
		ostringstream oss;
		oss << "Erreur dans CEntityManager::WearArmorToDummy() : entite " << entityId << " introuvable";
		CEException e(oss.str());
		throw e;
	}
}

void CEntityManager::SaveCharacter(string sNPCID)
{
	map<string, CMobileEntity*>::iterator itNPC = m_mCharacters.find(sNPCID);
	if (itNPC != m_mCharacters.end()) {
		ILoader::CObjectInfos* pInfos = nullptr;
		itNPC->second->GetEntityInfos(pInfos);
		LoadCharacterInfos();
		ILoader::CAnimatedEntityInfos* pAnimatedEntity = static_cast<ILoader::CAnimatedEntityInfos*>(pInfos);
		m_mCharacterInfos[sNPCID] = *pAnimatedEntity;
		SaveCharacterInfos(m_mCharacterInfos);
		delete pAnimatedEntity;
	}
}

void CEntityManager::LoadCharacterInfos()
{
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

void CEntityManager::SaveCharacterInfos(const map<string, ILoader::CAnimatedEntityInfos>& characterInfos)
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
		pEntity->GetEntityName(sEntityName);
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
	SerializeNodeInfos<CMobileEntity>(pRoot, oss, 0);
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