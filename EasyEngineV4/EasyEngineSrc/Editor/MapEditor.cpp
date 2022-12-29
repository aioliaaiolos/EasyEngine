#include "IInputManager.h"
#include "MapEditor.h"
#include "IRenderer.h"
#include "ICamera.h"
#include "IGeometry.h"
#include "Interface.h"
#include "IEntity.h"
#include "ICollisionManager.h"
#include "IHud.h"
#include "ILoader.h"
#include "IFileSystem.h"
#include "EEPlugin.h"
#include "WorldEditor.h"
#include "IConsole.h"
#include "EditorManager.h"

#include <algorithm>


CMapEditor::CMapEditor(EEInterface& oInterface, ICameraManager::TCameraType cameraType) :
ISpawnableEditor(oInterface),
CSpawnableEditor(oInterface, cameraType),
IMapEditor(oInterface),
CPlugin(nullptr, ""),
m_oLoaderManager(*static_cast<ILoaderManager*>(oInterface.GetPlugin("LoaderManager"))),
m_oRessourceManager(*static_cast<IRessourceManager*>(oInterface.GetPlugin("RessourceManager"))),
m_oFileSystem(*static_cast<IFileSystem*>(oInterface.GetPlugin("FileSystem"))),
m_fPlanHeight(3000.f),
m_fGroundAdaptationHeight(0.f),
m_pHeightMap(nullptr),
m_fBias(0.f)
{
	IEventDispatcher* pEventDispatcher = static_cast<IEventDispatcher*>(oInterface.GetPlugin("EventDispatcher"));
	pEventDispatcher->AbonneToEntityEvent(this, OnSceneLoadRessource);
	string sSceneFileName;
	ISceneManager* pSceneManager = static_cast<ISceneManager*>(oInterface.GetPlugin("SceneManager"));
	m_pScene = pSceneManager->GetScene("Game");
	m_sTmpFolder = "levels/tmp";
	m_sTmpAdaptedHeightMapFileName = "/" + m_sTmpFolder + "/HMA_tmp.bmp";
}

void CMapEditor::OnEntityAdded()
{
	AdaptGroundToEntity(m_pEditingEntity);
}

void CMapEditor::AdaptGroundToAllEntities()
{
	if (!m_bEditionMode) {
		CEException e("Error : You can use 'AdaptGroundToAllEntities' only in map edition mode");
		throw e;
	}
	else {
		for (int i = 0; i < m_pScene->GetChildCount(); i++) {
			IEntity* pEntity = dynamic_cast<IEntity*>(m_pScene->GetChild(i));
			AdaptGroundToEntity(pEntity);
		}
	}
}

void CMapEditor::SetBias(float fBias)
{
	m_fBias = fBias;
}

void CMapEditor::AdaptGroundToEntity(IEntity* pEntity)
{
	if (pEntity) {
		IBox* pBox = dynamic_cast<IBox*>(pEntity->GetBoundingGeometry());
		if (pBox) {
			CVector dim = pBox->GetDimension();
			CVector pos;
			pEntity->GetWorldPosition(pos);
			if (m_oAnimatableMeshData.m_vMeshes.size() > 0) {
				ILoader::CMeshInfos& mi = m_oAnimatableMeshData.m_vMeshes[0];
				float hMin = 99999999.f;
				for (int i = 0; i < mi.m_vVertex.size() / 3; i++) {
					float x = mi.m_vVertex[i * 3];
					float y = mi.m_vVertex[i * 3 + 1];
					float z = mi.m_vVertex[i * 3 + 2];

					if ((x >= (pos.m_x - dim.m_x)) && (x <= (pos.m_x + dim.m_x)) &&
						(z >= (pos.m_z - dim.m_z)) && (z <= (pos.m_z + dim.m_z))) {
						if (hMin > y)
							hMin = y;
					}
				}
				hMin += m_fGroundAdaptationHeight;

				for (int i = 0; i < mi.m_vVertex.size() / 3; i++) {
					float x = mi.m_vVertex[i * 3];
					float z = mi.m_vVertex[i * 3 + 2];

					if ((x >= (pos.m_x - dim.m_x)) && (x <= (pos.m_x + dim.m_x)) &&
						(z >= (pos.m_z - dim.m_z)) && (z <= (pos.m_z + dim.m_z))) {
						mi.m_vVertex[i * 3 + 1] = hMin;
					}
				}
			}
			else {

				if (!m_pHeightMap)
					m_pHeightMap = m_oCollisionManager.GetHeightMap(m_pScene->GetCurrentHeightMapIndex());
				int idx = m_oHud.Print("Adaptation du terrain en cours", m_nHudX, m_nHudY + m_oHud.GetLineCount() * m_nHudLineHeight);

				static bool multiThreaded = false;
				if (multiThreaded) {
					DWORD threadId = 0;
					AdaptGroundThreadStruct* params = new AdaptGroundThreadStruct;
					params->pos = pos;
					params->dim = dim;
					params->entityWorldTM = pBox->GetTM();
					params->editor = this;
					params->hudMsgIdx = idx;
					params->adaptationHeight = m_fGroundAdaptationHeight;
					CreateThread(nullptr, 0, AdaptGround, params, 0, &threadId);
				}
				else {
					m_pHeightMap->AdaptGroundMapToModel(pEntity->GetWorldMatrix(), pBox->GetDimension(), m_fGroundAdaptationHeight);
					m_oHud.RemoveText(idx);
					m_oHud.Print("Adaptation terminee", m_nHudX, m_nHudY + m_oHud.GetLineCount() * m_nHudLineHeight);
					UpdateGround();
				}
			}
		}
	}
}

DWORD CMapEditor::AdaptGround(void* params)
{
	AdaptGroundThreadStruct* s = (AdaptGroundThreadStruct*)params;
	s->editor->m_pHeightMap->AdaptGroundMapToModel(s->entityWorldTM, s->dim, s->adaptationHeight);
	s->editor->OnEndAdaptGround(s->hudMsgIdx);
	delete s;
	return 0;
}

void CMapEditor::OnEndAdaptGround(int hudMsgIdx)
{
	m_oHud.RemoveText(hudMsgIdx);
	m_oHud.Print("Adaptation terminee", m_nHudX, m_nHudY + m_oHud.GetLineCount() * m_nHudLineHeight);
}

void CMapEditor::UpdateGround()
{
	if ( m_oAnimatableMeshData.m_vMeshes.empty() && m_pHeightMap) {
		string root;
		m_oFileSystem.GetLastDirectory(root);
		WIN32_FIND_DATAA wfd;
		string tmpFolder = "/" + m_sTmpFolder;
		string tmpPath = root + tmpFolder;
		HANDLE hLevelFolder = m_oFileSystem.FindFirstFile_EE(tmpPath, wfd);
		bool folderCreated = hLevelFolder != INVALID_HANDLE_VALUE;
		if ((!folderCreated) || !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			folderCreated = CreateDirectoryA(tmpPath.c_str(), nullptr);
		}
		if (!folderCreated) {
			m_oHud.Print(string("Erreur, impossible de creer le dossier '") + tmpPath + "'", m_nHudX, m_nHudY + m_oHud.GetLineCount() * m_nHudLineHeight);
			return;
		}
	
		string originalRessourceName;
		m_pScene->GetOriginalSceneFileName(originalRessourceName);
		if (originalRessourceName.empty()) {
			m_pHeightMap->GetFileName(originalRessourceName);
			m_pScene->SetOriginalSceneFileName(originalRessourceName);
		}

		string sGroundFileName;
		m_pScene->GetRessource()->GetFileName(sGroundFileName);
		string srcPath = root + sGroundFileName;

		string sRessourceFileName;
		m_pScene->GetRessource()->GetFileName(sRessourceFileName);
		string levelPath = sRessourceFileName.substr(0, sRessourceFileName.find_last_of("/") + 1);
		string destPath = root + "/" + m_sTmpFolder + "/ground.bme";
		if(srcPath != destPath)
			CopyFileA(srcPath.c_str(), destPath.c_str(), TRUE);

		m_pHeightMap->Save(m_sTmpAdaptedHeightMapFileName);
		m_pScene->SetRessource(m_sTmpAdaptedHeightMapFileName);
	}
}

bool CMapEditor::CreateLevelFolderIfNotExists(string levelName, string& levelFolder)
{
	string root;
	m_oFileSystem.GetLastDirectory(root);
	WIN32_FIND_DATAA wfd;
	string relativeFolderPath = "/levels";
	levelFolder = root + relativeFolderPath;
	HANDLE hLevelFolder = m_oFileSystem.FindFirstFile_EE(levelFolder, wfd);
	bool folderCreated = hLevelFolder != INVALID_HANDLE_VALUE;
	if ((!folderCreated) || !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
		folderCreated = CreateDirectoryA(levelFolder.c_str(), nullptr);
	}
	if (!folderCreated) {
		m_oHud.Print(string("Erreur, impossible de creer le dossier '") + levelFolder + "'", m_nHudX, m_nHudY + m_oHud.GetLineCount() * m_nHudLineHeight);
		return false;
	}

	levelFolder += "/" + levelName;
	hLevelFolder = m_oFileSystem.FindFirstFile_EE(levelFolder, wfd);
	folderCreated = hLevelFolder != INVALID_HANDLE_VALUE;
	if ((!folderCreated) || !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
		folderCreated = CreateDirectoryA(levelFolder.c_str(), nullptr);
	}
	if (!folderCreated) {
		m_oHud.Print(string("Erreur, impossible de creer le dossier '") + levelFolder + "'", m_nHudX, m_nHudY + m_oHud.GetLineCount() * m_nHudLineHeight);
		return false;
	}
	return true;
}

void CMapEditor::Save(string mapName)
{
	if (m_bEditionMode) {
		m_sCurrentMapName = mapName;
		string levelFolder;
		if (CreateLevelFolderIfNotExists(mapName, levelFolder)) {
			levelFolder += "/";
			string originalRessourceName;
			if (m_oAnimatableMeshData.m_vMeshes.size() > 0) {
				string sFileName = mapName + ".bme";
				m_oLoaderManager.Export(sFileName, m_oAnimatableMeshData);
				m_oCollisionManager.CreateHeightMap(sFileName);
				m_pScene->GetOriginalSceneFileName(originalRessourceName);
				m_pScene->SetOriginalSceneFileName(originalRessourceName);
				m_pScene->SetRessource(sFileName);
			}
			else {
				string root;
				m_oFileSystem.GetLastDirectory(root);
				string srcPath = root + m_sTmpAdaptedHeightMapFileName;
				string newHMFile = "levels/" + mapName + "/HMA_" + mapName + ".bmp";
				string destPath = levelFolder + "HMA_" + mapName + ".bmp";
				m_pScene->SetHMFile("/" + newHMFile);
				if (!MoveFileA(srcPath.c_str(), destPath.c_str())) {
					if (!m_pHeightMap)
						m_pHeightMap = m_oCollisionManager.GetHeightMap(m_pScene->GetCurrentHeightMapIndex());
					m_pHeightMap->Save(newHMFile);
				}
				srcPath = root + "/" + m_sTmpFolder + "/ground.bme";;
				destPath = levelFolder + "ground.bme";
				if (!MoveFileA(srcPath.c_str(), destPath.c_str())) {
					string sGroundFileName;
					m_pScene->GetRessource()->GetFileName(sGroundFileName);
					srcPath = root + sGroundFileName;
					CopyFileA(srcPath.c_str(), destPath.c_str(), FALSE);
				}
			}
			SaveMap(levelFolder + mapName + ".bse", m_fBias);
		}
	}
	else {
		CEException e("You have to enable MapEditionMode to be able to save the map.");
		throw e;
	}
}

void CMapEditor::SaveMap(string sFileName, float fBias)
{
	ILoader::CSceneInfos si;
	m_pScene->GetInfos(si);
	ClearCharacters(si.m_vObject);
	m_oLoaderManager.Export(sFileName, si);
	m_pScene->CreateCollisionMaps(fBias);
}

void CMapEditor::Load(string sFileName)
{
	string root;
	m_oFileSystem.GetLastDirectory(root);
	sFileName = root + "/levels/" + sFileName + "/" + sFileName + ".bse";
	ILoader::CSceneInfos si;
	m_oLoaderManager.Load(sFileName, si);
	m_pScene->Load(si);
	if(m_bEditionMode)
		InitCamera();
	else {
		ICamera* pCamera = m_oCameraManager.GetCameraFromType(ICameraManager::TCameraType::TFree);
		m_oCameraManager.SetActiveCamera(pCamera);
		pCamera->SetWorldPosition(0, 0, 0);
	}
	m_sCurrentMapName = sFileName;
	
	CWorldEditor* pWorldEditor = dynamic_cast<CWorldEditor*>(m_pEditorManager->GetEditor(IEditor::Type::eWorld));
	if (pWorldEditor->IsEnabled()) {
		string sExt = ".bse";
		string sMapName = sFileName.substr(sFileName.find_last_of("/") + 1);
		sMapName = sMapName.substr(0, sMapName.size() - sExt.size());
		pWorldEditor->HandleMapLoaded(sMapName);
	}
}

void CMapEditor::ClearCharacters(vector<ILoader::CObjectInfos*>& objects)
{
	vector<ILoader::CObjectInfos*>::iterator itObject = objects.begin();
	while (itObject != objects.end()) {
		ILoader::CAnimatedEntityInfos* pChildCharacter = dynamic_cast<ILoader::CAnimatedEntityInfos*>(*itObject);
		if (pChildCharacter)
			itObject = objects.erase(itObject);
		else {
			ILoader::CEntityInfos* pEntityInfos = dynamic_cast<ILoader::CEntityInfos*>(*itObject);
			if (pEntityInfos)
				ClearCharacters(pEntityInfos->m_vSubEntityInfos);
			itObject++;
		}
	}
}

void CMapEditor::OnEntitySelected()
{
	IBox* pBox = dynamic_cast<IBox*>(m_pEditingEntity->GetBoundingGeometry());
	string originalRessourceName;
	m_pScene->GetOriginalSceneFileName(originalRessourceName);
	if (!m_pHeightMap)
		m_pHeightMap = m_oCollisionManager.GetHeightMap(m_pScene->GetCurrentHeightMapIndex());
	m_pHeightMap->RestoreHeightMap(m_pEditingEntity->GetWorldMatrix(), pBox->GetDimension(), originalRessourceName);
	UpdateGround();
}

float CMapEditor::GetPlanHeight()
{
	IBox* pSceneBox = static_cast<IBox*>(m_pScene->GetBoundingGeometry());
	if (pSceneBox)
		m_fPlanHeight = pSceneBox->GetDimension().m_y;
	return m_fPlanHeight;
}

void CMapEditor::SetEditionMode(bool bEditionMode)
{
	if (m_bEditionMode != bEditionMode) {
		CEditor::SetEditionMode(bEditionMode);
		if (bEditionMode) {
			m_pScene->Clear();
		}
		else {
			// Ask to save
			//m_oConsole.Print("Voulez-vous sauvegarder la map O/N ? ");
			//m_oConsole.WaitForResponse(SaveResponseCallback, this);
		}
	}
}

void CMapEditor::CollectSelectableEntity(vector<IEntity*>& entities)
{
	m_pScene->CollectMinimapEntities(entities);
}

void CMapEditor::OnEntityRemoved(IEntity* pEntity)
{

}

void CMapEditor::SaveResponseCallback(string sResponse, void* pData)
{
	CMapEditor* pMapEditor = (CMapEditor*)pData;
	std::transform(sResponse.begin(), sResponse.end(), sResponse.begin(), tolower);
	if (sResponse == "y") {
		if (pMapEditor->m_sCurrentMapName.empty())
			pMapEditor->m_sCurrentMapName = "SavedMap";
		pMapEditor->Save(pMapEditor->m_sCurrentMapName);
	}
}

void CMapEditor::SetGroundAdaptationHeight(float fHeight)
{
	m_fGroundAdaptationHeight = fHeight;
}

string CMapEditor::GetName()
{
	return "Editor";
}

void CMapEditor::Edit(string id)
{
	SetEditionMode(true);
	try {
		Load(id);
	}
	catch (CEException& e) {
		m_oConsole.Println("Map inexistante, vous pouvez demarrer la creation d'une nouvelle map avec la commande 'SetSceneMap(HeightMapName, DiffuseTextureName, width, height)' ou editer une map existante");
		InitCamera();
	}
}

void CMapEditor::SpawnEntity(string sEntityFileName)
{
	if(!m_bEditionMode)
		SetEditionMode(true);
	m_eEditorMode = TEditorMode::eAdding;
	m_pEditingEntity = m_oEntityManager.CreateEntity(sEntityFileName, "");
	InitSpawnedEntity();
}

void CMapEditor::OnSceneLoadRessource(CPlugin* plugin, IEventDispatcher::TEntityEvent e, IEntity* pEntity)
{
	if (e == IEventDispatcher::TEntityEvent::T_LOAD_RESSOURCE) {
		CMapEditor* pEditor = dynamic_cast<CMapEditor*>(plugin);
		if (pEditor) {
			string sSceneFileName;
			pEditor->m_pScene->GetRessource()->GetFileName(sSceneFileName);
			pEditor->m_oLoaderManager.Load(sSceneFileName, pEditor->m_oAnimatableMeshData);
		}
	}
}

