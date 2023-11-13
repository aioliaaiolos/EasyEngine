#include "CharacterEditor.h"
#include "IEntity.h"
#include "Interface.h"
#include "IInputManager.h"
#include "ICamera.h"
#include "IConsole.h"
#include "WorldEditor.h"
#include "EditorManager.h"
#include "../Utils2/StringUtils.h"

#include <algorithm>

CCharacterEditor::CCharacterEditor(EEInterface& oInterface, ICameraManager::TCameraType type) :
	CEditor(oInterface, type),
	CPlugin(nullptr, ""),
	ICharacterEditor(oInterface),
	m_bIsLeftMousePressed(false),
	m_pWorldEditor(nullptr),
	m_pCurrentCharacter(nullptr),
	m_pCurrentEditableCloth(nullptr),
	m_pRightEye(nullptr),
	m_oLoaderManager(static_cast<ILoaderManager&>(*oInterface.GetPlugin("LoaderManager")))
{
	m_pScene = m_oSceneManager.GetScene("Game");
	IEventDispatcher* pEventDispatcher = static_cast<IEventDispatcher*>(oInterface.GetPlugin("EventDispatcher"));
	pEventDispatcher->AbonneToMouseEvent(this, OnMouseEventCallback);
	oInterface.HandlePluginCreation("EditorManager", [this](CPlugin* pPlugin)
	{
		CEditorManager* pEditorManager = (CEditorManager*)pPlugin;
		if (pEditorManager) {
			m_pWorldEditor = dynamic_cast<CWorldEditor*>(pEditorManager->GetEditor(IEditor::Type::eWorld));
		}
	});
}

void CCharacterEditor::SetEditionMode(bool bEditionMode)
{
	if (m_bEditionMode != bEditionMode) {
		CEditor::SetEditionMode(bEditionMode);
	}
	if (bEditionMode) {
		m_oEntityManager.Clear();
		m_pScene->Clear();
		ZoomCameraBody();
		IEntity* pLight = m_oEntityManager.CreateLightEntity(CVector(1, 1, 1), IRessource::TLight::OMNI, 0.1f);
		CVector pos(230, 150, -35);
		pLight->SetLocalPosition(pos);
		pLight->Link(m_pScene);
		m_oInputManager.ShowMouseCursor(true);
		//m_pScene->Update();
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

void CCharacterEditor::ZoomCameraLarge()
{
	m_eZoomType = eLarge;
	static int x = 230, y = 150, z = -35;
	CVector pos(500, 200, -35);
	Zoom(pos, 106, -14, -1);
}

void CCharacterEditor::ZoomCameraHead()
{
	m_eZoomType = eHead;
	CVector pos(30, 163, -10);
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
	CVector pos(17, 158, -5);
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
		m_pCurrentCharacter->Save();
		m_oEntityManager.LoadCharacterInfos();
	}
}

string CCharacterEditor::GetName()
{
	return "CharacterEditor";
}

bool CCharacterEditor::IsEnabled()
{
	return m_bEditionMode; 
}

void CCharacterEditor::InitEyeNodes()
{
	INode* pHead = m_pLeftEye = m_pCurrentCharacter->GetSkeletonRoot()->GetChildBoneByName("Tete");
	if (!pHead) {
		CEException e(string("Erreur : le node 'Tete' est introuvable dans le personnage ") + m_pCurrentCharacter->GetName());
		throw e;
	}
	for (int i = 0; i < pHead->GetChildCount(); i++) {
		INode* pNode = pHead->GetChild(i);
		if (pNode->GetName() == "OeilD") {
			m_pRightEye = pNode;
		}
		if (pNode->GetName() == "OeilG") {
			m_pLeftEye = pNode;
		}
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
	string sCharacterIdLow = sCharacterId;
	std::transform(sCharacterId.begin(), sCharacterId.end(), sCharacterIdLow.begin(), tolower);

	if(!m_bEditionMode)
		SetEditionMode(true);
	if (m_pCurrentCharacter)
		m_pCurrentCharacter->Unlink();
	m_pCurrentCharacter = dynamic_cast<ICharacter*>(m_oEntityManager.GetEntity(sCharacterIdLow));
	if (!m_pCurrentCharacter) {
		m_pCurrentCharacter = m_oEntityManager.BuildCharacterFromDatabase(sCharacterIdLow, m_pScene);
		if (!m_pCurrentCharacter) {
			if (!sCharacterIdLow.empty()) {
				if (sCharacterIdLow == "Player")
					m_pCurrentCharacter = m_oEntityManager.CreatePlayer("body03");
				else
					m_pCurrentCharacter = m_oEntityManager.CreateNPC("body03", sCharacterIdLow);
				m_pCurrentCharacter->Link(m_pScene);
			}
			else
				throw CEException("Erreur : CCharacterEditor::SpawnEntity() -> Vous devez indiquer un ID pour votre personnage");
		}
	}
	else
		m_pCurrentCharacter->Link(m_pScene);
	InitSpawnedCharacter();
	m_oCameraManager.SetActiveCamera(m_pEditorCamera);

	InitEyeNodes();
	InitHeadNode(m_pCurrentCharacter);
}

void CCharacterEditor::InitSpawnedCharacter()
{
	if (m_pCurrentCharacter) {
		m_pCurrentCharacter->SetWeight(0);
		m_pCurrentCharacter->SetWorldPosition(0, m_pCurrentCharacter->GetHeight() / 2.f, 0);
		m_pCurrentCharacter->RunAction("Stand", true);
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

void CCharacterEditor::SetHairs(string sHairsName)
{
	try
	{
		m_pCurrentCharacter->SetHairs(sHairsName);
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

void CCharacterEditor::SetTexture(string sTexture)
{
	CStringUtils::GetFileNameWithExtension(sTexture, "bmp", sTexture);
	m_pCurrentCharacter->SetDiffuseTexture(sTexture);
}

void CCharacterEditor::SetBody(string sBodyName)
{
	if (sBodyName.find(".bme") == -1)
		sBodyName += ".bme";
	m_pCurrentCharacter->SetBody(sBodyName);
	InitSpawnedCharacter();
	InitEyeNodes();
	InitHeadNode(m_pCurrentCharacter);
}

void CCharacterEditor::Edit(string id)
{
	SetEditionMode(true);
	string idLow = id;
	std::transform(id.begin(), id.end(), idLow.begin(), tolower);
	SpawnEntity(idLow);
}

void CCharacterEditor::SetSpecular(float r, float g, float b)
{
	m_pCurrentCharacter->SetCustomSpecular(CVector(r, g, b));
}

void CCharacterEditor::SetShininess(float fValue)
{
	IMesh* pMesh = dynamic_cast<IMesh*>(m_pCurrentCharacter->GetRessource());
	if (pMesh) {
		for (int i = 0; i < pMesh->GetMaterialCount(); i++) {
			pMesh->GetMaterial(i)->SetShininess(fValue);
		}
	}
}

void CCharacterEditor::EditCloth(string sClothName)
{
	m_sCurrentEditableCloth = sClothName;
	string sDummyName = "Dummy" + sClothName;
	IBone* pCurrentEditableDummyCloth = m_pCurrentCharacter->GetSkeletonRoot()->GetChildBoneByName(sDummyName);
	if (pCurrentEditableDummyCloth)
		m_pCurrentEditableCloth = dynamic_cast<IEntity*>(pCurrentEditableDummyCloth->GetChild(0));
	else {
		IEntity* pCloth = m_oEntityManager.GetEntity(sClothName);
		if (pCloth)
			m_pCurrentEditableCloth = pCloth;
		else
			throw CEException(sDummyName + " not found. If your cloth is skinned, check the name you entered corresponds to the object name into Max scene");
	}
}

void CCharacterEditor::OffsetCloth(float x, float y, float z)
{
	if (m_pCurrentEditableCloth) {
		IMesh* pMesh = dynamic_cast<IMesh*>(m_pCurrentEditableCloth->GetRessource());
		if (pMesh && pMesh->IsSkinned())
			m_pCurrentEditableCloth->SetSkinOffset(x, y, z);
		else
			m_pCurrentEditableCloth->WorldTranslate(x, y, z);
		m_offsetCloth += CVector(x, y, z);
	}
	else
		throw CEException("Error : no cloth selected");
}

void CCharacterEditor::OffsetEyes(float x, float y, float z)
{
	if (m_pRightEye && m_pLeftEye) {
		m_pRightEye->LocalTranslate(x, y, z);
		m_pLeftEye->LocalTranslate(x, y, z);
		m_vOffsetEyes += CVector(x, y, z);
	}
	else {
		throw CEException("Error : Eye nodes not initialized");
	}
}

void CCharacterEditor::TurnEyes(float fYaw, float fPitch, float fRoll)
{
	if (m_pRightEye && m_pLeftEye) {
		m_pRightEye->Yaw(fYaw);
		m_pRightEye->Pitch(fPitch);
		m_pRightEye->Roll(fRoll);

		m_pLeftEye->Yaw(fYaw);
		m_pLeftEye->Pitch(fPitch);
		m_pLeftEye->Roll(fRoll);
	}
	else {
		throw CEException("Error : Eye nodes not initialized");
	}
}

void CCharacterEditor::SaveCurrentEditableCloth()
{
	string sFileName;
	IMesh* pMesh = dynamic_cast<IMesh*>(m_pCurrentEditableCloth->GetRessource());
	if (pMesh->IsSkinned())
		pMesh->GetFileName(sFileName);
	else
		sFileName = string("Meshes/Clothes/") + m_sCurrentEditableCloth + ".bme";
	ILoader::CAnimatableMeshData ami;
	m_oLoaderManager.Load(sFileName, ami);

	if (pMesh->IsSkinned())	
		ami.m_vMeshes[0].m_oOrgMaxPosition = m_offsetCloth;
	else {
		map<int, pair<string, CMatrix>>::iterator it = ami.m_mBones.begin();
		CMatrix& tm = it->second.second;
		tm.m_03 -= m_offsetCloth.m_x;
		tm.m_13 -= m_offsetCloth.m_y;
		tm.m_23 -= m_offsetCloth.m_z;
	}
	m_oLoaderManager.Export(sFileName, ami);
}

void CCharacterEditor::SaveModifiedMesh()
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

void CCharacterEditor::OnEditorExit()
{
	m_pCurrentCharacter = nullptr;
}

void CCharacterEditor::AddItem(string sItemName)
{
	m_pCurrentCharacter->AddItem(sItemName);
}

void CCharacterEditor::RemoveItem(string sItemID)
{
	m_pCurrentCharacter->RemoveItem(sItemID);
}

void CCharacterEditor::WearItem(string sItemID)
{
	if(m_pCurrentCharacter)
		m_pCurrentCharacter->WearItem(sItemID);
	else {
		throw(CEException("Error in CCharacterEditor::WearItem() : no active character selected"));
	}
}

ICharacter* CCharacterEditor::GetCurrentCharacter()
{
	return m_pCurrentCharacter;
}

void CCharacterEditor::OnMouseEventCallback(CPlugin* plugin, IEventDispatcher::TMouseEvent e, int x, int y)
{
	CCharacterEditor* pEditor = dynamic_cast<CCharacterEditor*>(plugin);
	if (pEditor->m_pCurrentCharacter) {
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
					case eLarge:
						pEditor->ZoomCameraBody();
						break;
					case eBody:
						pEditor->ZoomCameraHead();
						break;
					case eHead:
						pEditor->ZoomCameraEye();
						break;
					default:
						break;
					}

				}
				else if (x < 0) {
					switch (pEditor->m_eZoomType)
					{
					case eEye:
						pEditor->ZoomCameraHead();
						break;
					case eHead:
						pEditor->ZoomCameraBody();
						break;
					case eBody:
						pEditor->ZoomCameraLarge();
						break;
					default:
						break;
					}

				}
			}
		}
	}
}