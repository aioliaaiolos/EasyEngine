#include "CharacterEditor.h"
#include "IEntity.h"
#include "Interface.h"
#include "IInputManager.h"
#include "ICamera.h"
#include "IConsole.h"
#include "WorldEditor.h"
#include "EditorManager.h"

CCharacterEditor::CCharacterEditor(EEInterface& oInterface, ICameraManager::TCameraType type) :
CEditor(oInterface, type),
CPlugin(nullptr, ""),
ICharacterEditor(oInterface),
m_bIsLeftMousePressed(false),
m_pWorldEditor(nullptr),
m_pCurrentCharacter(nullptr),
m_pCurrentEditableCloth(nullptr),
m_oLoaderManager(static_cast<ILoaderManager&>(*oInterface.GetPlugin("LoaderManager")))
{
	m_pScene = m_oSceneManager.GetScene("Game");
	IEventDispatcher* pEventDispatcher = static_cast<IEventDispatcher*>(oInterface.GetPlugin("EventDispatcher"));
	pEventDispatcher->AbonneToMouseEvent(this, OnMouseEventCallback);
	pEventDispatcher->AbonneToKeyEvent(this, OnKeyPressCallback);
	oInterface.HandlePluginCreation("EditorManager", HandleEditorCreation, this);
}

void CCharacterEditor::HandleEditorCreation(CPlugin* pPlugin, void* pData)
{
	CEditorManager* pEditorManager = (CEditorManager*)pPlugin;
	CCharacterEditor* pCharacterEditor = (CCharacterEditor*)pData;
	if (pCharacterEditor && pEditorManager) {
		pCharacterEditor->m_pWorldEditor = dynamic_cast<CWorldEditor*>(pEditorManager->GetEditor(IEditor::Type::eWorld));
	}
}

void CCharacterEditor::SetEditionMode(bool bEditionMode)
{
	if (m_bEditionMode != bEditionMode) {
		CEditor::SetEditionMode(bEditionMode);
		if (m_bEditionMode) {
			m_pScene->Clear();
			ZoomCameraBody();
			IEntity* pLight = m_oEntityManager.CreateLightEntity(CVector(1, 1, 1), IRessource::TLight::OMNI, 0.1f);
			CVector pos(230, 150, -35);
			pLight->SetLocalPosition(pos);
			pLight->Link(m_pScene);
			m_oInputManager.ShowMouseCursor(true);
			m_pScene->Update();
		}
	}
}

void CCharacterEditor::InitCamera(const CVector& pos)
{
	CMatrix m;
	m_pEditorCamera->SetLocalMatrix(m);
	m_pEditorCamera->SetLocalPosition(pos);
	m_pEditorCamera->Yaw(106);
	m_pEditorCamera->Pitch(-14);
	m_pEditorCamera->Roll(-1);
	m_pEditorCamera->Update();
}

void CCharacterEditor::ZoomCameraBody()
{
	m_eZoomType = eBody;
	CVector pos(230, 150, -35);
	Zoom(pos, 106, -14, -1);
}

void CCharacterEditor::ZoomCameraHead()
{
	m_eZoomType = eHead;
	CVector pos(30, 157, -10);
	Zoom(pos, 106, -5, -1);
}

void CCharacterEditor::ZoomCameraEye()
{
	m_eZoomType = eEye;
	static float x = 230;
	static float y = 150;
	static float z = -35;
	static float yaw = 106;
	static float pitch = -14;
	static float roll = -1;
	CVector pos(x, y, z);
	Zoom(pos, yaw, pitch, roll);
}

void CCharacterEditor::Zoom(const CVector& pos, float fYaw, float fPitch, float fRoll)
{
	CMatrix m;
	m_pEditorCamera->SetLocalMatrix(m);
	m_pEditorCamera->SetLocalPosition(pos);
	m_pEditorCamera->Yaw(fYaw);
	m_pEditorCamera->Pitch(fPitch);
	m_pEditorCamera->Roll(fRoll);
	m_pEditorCamera->Update();
}

void CCharacterEditor::Load(string sCharacterId)
{
	
}

void CCharacterEditor::Save()
{
	if (m_bEditionMode && m_pCurrentCharacter) {
		string sId;
		m_pCurrentCharacter->GetEntityName(sId);
		m_oEntityManager.SaveCharacter(sId);
	}
}

void CCharacterEditor::HandleEditorManagerCreation(IEditorManager* pEditor)
{

}

string CCharacterEditor::GetName()
{
	return "CharacterEditor";
}

bool CCharacterEditor::IsEnabled()
{
	return m_bEditionMode;
}

void CCharacterEditor::InitEyeNodes(INode* pParent)
{
	for (int i = 0; i < pParent->GetChildCount(); i++) {
		INode* pNode = pParent->GetChild(i);
		if (pNode->GetName() == "OeilD") {
			m_pRightEye = pNode;
		}
		if (pNode->GetName() == "OeilG") {
			m_pLeftEye = pNode;
		}
		if (!m_pLeftEye || !m_pRightEye)
			InitEyeNodes(pNode);
	}
}

void CCharacterEditor::InitHeadNode(INode* pParent)
{
	for (int i = 0; i < pParent->GetChildCount(); i++) {
		INode* pNode = pParent->GetChild(i);
		if (pNode->GetName() == "Tete") {
			m_pHeadNode = pNode;
		}
		if (m_pHeadNode)
			InitHeadNode(pNode);
	}
}

void CCharacterEditor::SpawnEntity(string sCharacterId)
{
	if(!m_bEditionMode)
		SetEditionMode(true);
	m_pCurrentCharacter = dynamic_cast<ICharacter*>(m_oEntityManager.GetEntity(sCharacterId));
	if (!m_pCurrentCharacter) {
		m_pCurrentCharacter = m_oEntityManager.BuildCharacterFromDatabase(sCharacterId, m_pScene);
		if (!m_pCurrentCharacter) {
			if (!sCharacterId.empty()) {
				if (sCharacterId == "Player")				
					m_pCurrentCharacter = m_oEntityManager.CreatePlayer("body03");
				else
					m_pCurrentCharacter = m_oEntityManager.CreateNPC("body01", sCharacterId);
				m_pCurrentCharacter->Link(m_pScene);
			}
			else
				throw CEException("Erreur : CCharacterEditor::SpawnEntity() -> Vous devez indiquer un ID pour votre personnage");
		}
	}
	InitSpawnedCharacter();
	m_oCameraManager.SetActiveCamera(m_pEditorCamera);

	InitEyeNodes(m_pCurrentCharacter);
	InitHeadNode(m_pCurrentCharacter);
}

void CCharacterEditor::InitSpawnedCharacter()
{
	if (m_pCurrentCharacter) {
		m_pCurrentCharacter->SetWeight(0);
		m_pCurrentCharacter->SetWorldPosition(0, m_pCurrentCharacter->GetHeight() / 2.f, 0);
		m_pCurrentCharacter->RunAction("stand", true);
	}
}

void CCharacterEditor::SetCurrentEditablePlayer(IPlayer* pPlayer)
{
	m_pCurrentCharacter = pPlayer;
	InitSpawnedCharacter();
}

void CCharacterEditor::SetCurrentEditableNPC(ICharacter* pNPCEntity)
{
	m_pCurrentCharacter = pNPCEntity;
	InitSpawnedCharacter();
}

void CCharacterEditor::AddHairs(string sHairsName)
{
	try
	{
		m_pCurrentCharacter->AddHairs(sHairsName);
	}
	catch (CEException& e) {
		m_oConsole.Println(e.what());
	}
}

void CCharacterEditor::WearShoes(string sShoesName)
{
	try
	{
		m_pCurrentCharacter->WearShoes(sShoesName);
	}
	catch (CEException& e) {
		m_oConsole.Println(e.what());
	}
}

void CCharacterEditor::UnWearShoes(string sShoesName)
{
	try
	{
		m_pCurrentCharacter->UnWearShoes(sShoesName);
	}
	catch (CEException& e) {
		m_oConsole.Println(e.what());
	}
}

void CCharacterEditor::UnWearAllShoes()
{
	try
	{
		m_pCurrentCharacter->UnWearAllShoes();
	}
	catch (CEException& e) {
		m_oConsole.Println(e.what());
	}
}

void CCharacterEditor::WearCloth(string sClothName, string sDummyName)
{
	try
	{
		m_pCurrentCharacter->WearCloth(sClothName, sDummyName);
	}
	catch (CEException& e) {
		m_oConsole.Println(e.what());
	}
}

void CCharacterEditor::SetTexture(string sTexture)
{
	m_pCurrentCharacter->SetDiffuseTexture(sTexture);
}

void CCharacterEditor::SetBody(string sBodyName)
{
	if (sBodyName.find(".bme") == -1)
		sBodyName += ".bme";
	m_pCurrentCharacter->SetBody(sBodyName);
	InitSpawnedCharacter();
	InitEyeNodes(m_pCurrentCharacter);
	InitHeadNode(m_pCurrentCharacter);
}

void CCharacterEditor::Edit(string id)
{
	SetEditionMode(true);
	SpawnEntity(id);
}

void CCharacterEditor::SetSpecular(float r, float g, float b)
{
	m_pCurrentCharacter->SetCustomSpecular(CVector(r, g, b));
}

void CCharacterEditor::EditCloth(string sClothName)
{
	m_sCurrentEditableCloth = sClothName;
	string sDummyName = "Dummy" + sClothName;
	IBone* pCurrentEditableDummyCloth = m_pCurrentCharacter->GetSkeletonRoot()->GetChildBoneByName(sDummyName);
	if (pCurrentEditableDummyCloth)
		m_pCurrentEditableCloth = dynamic_cast<IEntity*>(pCurrentEditableDummyCloth->GetChild(0));
	else
		throw CEException(sDummyName + " not found");
}

void CCharacterEditor::OffsetCloth(float x, float y, float z)
{
	if (m_pCurrentEditableCloth) {
		m_pCurrentEditableCloth->WorldTranslate(x, y, z);
		m_offsetCloth += CVector(x, y, z);
	}
	else
		throw CEException("Error : no cloth selected");
}

void CCharacterEditor::OffsetEyes(float x, float y, float z)
{
	m_pRightEye->LocalTranslate(x, y, z);
	m_pLeftEye->LocalTranslate(x, y, z);
	m_vOffsetEyes += CVector(x, y, z);
}

void CCharacterEditor::TurnEyes(float fYaw, float fPitch, float fRoll)
{
	m_pRightEye->Yaw(fYaw);
	m_pRightEye->Pitch(fPitch);
	m_pRightEye->Roll(fRoll);

	m_pLeftEye->Yaw(fYaw);
	m_pLeftEye->Pitch(fPitch);
	m_pLeftEye->Roll(fRoll);
}

void CCharacterEditor::SaveCurrentEditableCloth()
{
	string sFileName = string("Meshes/Clothes/") + m_sCurrentEditableCloth + ".bme";
	ILoader::CAnimatableMeshData ami;
	m_oLoaderManager.Load(sFileName, ami);
	map<int, pair<string, CMatrix>>::iterator it = ami.m_mBones.begin();
	CMatrix& tm = it->second.second;
	tm.m_03 -= m_offsetCloth.m_x;
	tm.m_13 -= m_offsetCloth.m_y;
	tm.m_23 -= m_offsetCloth.m_z;
	m_oLoaderManager.Export(sFileName, ami);
}

void CCharacterEditor::SaveCurrentEditableBody()
{
	string sFileName;
	m_pCurrentCharacter->GetRessource()->GetFileName(sFileName);
	ILoader::CAnimatableMeshData ami;
	m_oLoaderManager.Load(sFileName, ami);
	for (ILoader::CMeshInfos& mi : ami.m_vMeshes)
		if (mi.m_sName == "OeilD" || mi.m_sName == "OeilG")
			mi.m_oOrgMaxPosition += m_vOffsetEyes;
	m_oLoaderManager.Export(sFileName, ami);
}


void CCharacterEditor::OnMouseEventCallback(CPlugin* plugin, IEventDispatcher::TMouseEvent e, int x, int y)
{
	CCharacterEditor* pEditor = dynamic_cast<CCharacterEditor*>(plugin);
	if (pEditor->m_bEditionMode) {
		if (e == IEventDispatcher::TMouseEvent::T_LBUTTONDOWN) {
			pEditor->m_bIsLeftMousePressed = true;
			pEditor->m_nMousePosX = x;
		}
		else if (e == IEventDispatcher::TMouseEvent::T_LBUTTONUP) {
			pEditor->m_bIsLeftMousePressed = false;
		}
		else if (e == IEventDispatcher::TMouseEvent::T_MOVE) {
			if (pEditor->m_bIsLeftMousePressed) {
				int delta = (x - pEditor->m_nMousePosX) / 5;
				pEditor->m_pCurrentCharacter->Yaw(delta);
				pEditor->m_nMousePosX = x;
			}
		}
		else if (e == IEventDispatcher::TMouseEvent::T_WHEEL) {
			if (x > 0) {
				switch (pEditor->m_eZoomType)
				{
				case eBody:
					pEditor->ZoomCameraHead();
					break;
				case eHead:
					pEditor->ZoomCameraEye();
					break;
				default:
					break;
				}
				
			} else if (x < 0) {
				switch (pEditor->m_eZoomType)
				{
				case eEye:
					pEditor->ZoomCameraHead();
					break;
				case eHead:
					pEditor->ZoomCameraBody();
					break;
				default:
					break;
				}

			}
		}
	}
}

void CCharacterEditor::OnKeyPressCallback(CPlugin* plugin, IEventDispatcher::TKeyEvent e, int key)
{

}