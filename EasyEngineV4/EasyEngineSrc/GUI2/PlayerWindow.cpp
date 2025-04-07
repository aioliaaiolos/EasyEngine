#include "Interface.h"
#include "PlayerWindow.h"
#include "GUIManager.h"
#include "Utils2/rectangle.h"
#include "IRessource.h"
#include "IEntity.h"
#include "ICamera.h"
#include "Utils2/StringUtils.h"
#include "InventoryWindow.h"

CPlayerWindow::CPlayerWindow(EEInterface& oInterface, const CDimension& windowSize):
	CGUIWindow(oInterface, windowSize, CRectangle(0, 0, windowSize.GetWidth(), windowSize.GetHeight())),
	m_pGUIManager(static_cast<CGUIManager*>(oInterface.GetPlugin("GUIManager"))),
	m_pWindowBackground(NULL),
	m_oInterface(oInterface),
	m_pCameraManager(nullptr),
	m_pWindowCamera(nullptr),
	m_pPlayerCamera(nullptr),
	m_pLight(nullptr),
	m_fLightIntensity(0.06f)
{
	m_pArmorWindow = new CGUIWidget(oInterface, windowSize.GetWidth(), windowSize.GetHeight());
	SetPosition(100, 100);
	m_pInventory = new CInventoryWindow(oInterface, windowSize);
	AddWidget(m_pInventory);

	SetGUIMode(true);

	m_pEntityManager = static_cast<IEntityManager*>(oInterface.GetPlugin("EntityManager"));
	m_pPlayer = m_pEntityManager->GetPlayer();

	m_pCameraManager = static_cast<ICameraManager*>(oInterface.GetPlugin("CameraManager"));
	m_pWindowCamera = m_pCameraManager->CreateCamera(ICameraManager::TCameraType::TFree, 40.f);
	m_pLightEntity = m_pEntityManager->CreateLightEntity(CVector(1, 1, 1), IRessource::TLight::OMNI, 0.f);
	m_pLightEntity->SetName("PlayerWindowLightEntity");
}

CPlayerWindow::~CPlayerWindow()
{
}

void CPlayerWindow::OnShow(bool bShow)
{
	CGUIWindow::OnShow(bShow);

	if (bShow) {
		if (!m_pPlayer) {
			m_pPlayer = m_pEntityManager->GetPlayer();
			m_pPlayerCamera = m_pCameraManager->GetCameraFromType(ICameraManager::TCameraType::TLinked);
			InitCamera();
			InitLight();
		}
		SwitchToWindowCamera();
		m_pInventory->DisplayItems(m_pPlayer);
	}
	else {
		m_pInventory->Clear();
		RestoreCamera();
	}
}

void CPlayerWindow::SwitchToWindowCamera()
{
	CMatrix oPlayerLocalTM;
	m_pPlayer->GetLocalMatrix(oPlayerLocalTM);
	m_pCameraManager->SetActiveCamera(m_pWindowCamera);
	m_pLight->Enable(true);
	m_pLight->SetIntensity(m_fLightIntensity);
}

void CPlayerWindow::InitCamera()
{
	m_pWindowCamera->Link(m_pPlayer);
	static float x = 0.f, y = 80.f, z = 200.f;
	static float yaw = 10.f, pitch = -20.f, roll = 0.f;
	m_pWindowCamera->LocalTranslate(x, y, z);
	m_pWindowCamera->Yaw(yaw);
	m_pWindowCamera->Pitch(pitch);
	m_pWindowCamera->Roll(roll);
}

void CPlayerWindow::InitLight()
{
	m_pLightEntity->Link(m_pWindowCamera);
	static float x = 0, y = -50, z = 0;
	m_pLightEntity->LocalTranslate(x, y, z);

	m_pLight = dynamic_cast<ILight*>(m_pLightEntity->GetRessource());
	if (m_pLight)
		m_pLight->SetIntensity(m_fLightIntensity);
}

void CPlayerWindow::RestoreCamera()
{
	m_pCameraManager->SetActiveCamera(m_pPlayerCamera);
	m_pLight->Enable(false);
}

CGUIItem::CGUIItem(EEInterface& oInterface, string sFileName) :
	CGUIWindow(oInterface, sFileName)
{
}

