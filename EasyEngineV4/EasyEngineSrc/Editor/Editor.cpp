#include "Editor.h"
#include "Interface.h"
#include "EditorManager.h"
#include "IEntity.h"
#include "IInputManager.h"
#include "ICollisionManager.h"
#include "IGeometry.h"
#include "IConsole.h"
#include "ICamera.h"
#include "IHud.h"

CEditor::CEditor(EEInterface& oInterface, ICameraManager::TCameraType cameraType) :
IEditor(oInterface),
m_oInterface(oInterface),
m_pEditorManager(nullptr),
m_oCameraManager(*static_cast<ICameraManager*>(oInterface.GetPlugin("CameraManager"))),
m_oEntityManager(*static_cast<IEntityManager*>(oInterface.GetPlugin("EntityManager"))),
m_oInputManager(*static_cast<IInputManager*>(oInterface.GetPlugin("InputManager"))),
m_oSceneManager(static_cast<ISceneManager&>(*oInterface.GetPlugin("SceneManager"))),
m_oRenderer(*static_cast<IRenderer*>(oInterface.GetPlugin("Renderer"))),
m_oCollisionManager(*static_cast<ICollisionManager*>(oInterface.GetPlugin("CollisionManager"))),
m_oGeometryManager(*static_cast<IGeometryManager*>(oInterface.GetPlugin("GeometryManager"))),
m_oConsole(*static_cast<IConsole*>(oInterface.GetPlugin("Console"))),
m_pEditorCamera(nullptr),
m_bEditionMode(false),
m_oHud(*static_cast<IHud*>(oInterface.GetPlugin("HUD"))),
m_nHudX(800),
m_nHudY(150),
m_nHudLineHeight(15)
{
	oInterface.HandlePluginCreation("EditorManager", HandleEditorManagerCreation, this);

	m_pEditorCamera = m_oCameraManager.GetCameraFromType(cameraType);
	if (!m_pEditorCamera) {
		ICamera* pFreeCamera = m_oCameraManager.GetCameraFromType(ICameraManager::TCameraType::TFree);
		m_pEditorCamera = m_oCameraManager.CreateCamera(cameraType, pFreeCamera->GetFov());
	}
	m_oEntityManager.AddEntity(m_pEditorCamera, m_pEditorCamera->GetName());
}

void CEditor::HandleEditorManagerCreation(CPlugin* plugin, void* pData)
{
	if (pData) {
		CEditor* pEditor = static_cast<CEditor*>(pData);
		pEditor->m_pEditorManager = static_cast<CEditorManager*>(plugin);
	}
}

void CEditor::SetEditionMode(bool bEditionMode)
{
	//if (m_bEditionMode != bEditionMode) {
		m_bEditionMode = bEditionMode;
		m_pEditorManager->CloseAllEditorButThis(this);
		m_bEditionMode = bEditionMode;
		m_oInputManager.SetEditionMode(m_bEditionMode);

		m_oHud.Clear();
		if (m_bEditionMode) {
			m_oHud.Print("Mode Edition : Vous pouvez importer un modele en utilisant la commande 'SpawnEntity'", m_nHudX, m_nHudY);
			m_oHud.Print("Vous pouvez sauvegarder le niveau grace la commande 'SaveLavel(levelName)'", m_nHudX, m_nHudY + m_nHudLineHeight);
		}
	//}
}