#include "Interface.h"
#include "PlayerWindow.h"
#include "GUIManager.h"
#include "Utils2/rectangle.h"
#include "IRessource.h"
#include "IEntity.h"
#include "ICamera.h"
#include "Utils2/StringUtils.h"

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
	m_pArmorWindow = new CGUIWidget(windowSize.GetWidth(), windowSize.GetHeight());
	SetPosition(100, 100);
	
	m_pInventory = new CGUIWindow(windowSize);
	AddWidget(m_pInventory);

	IRessourceManager* pRessourceManager = static_cast<IRessourceManager*>(oInterface.GetPlugin("RessourceManager"));
	ITexture* pTexture = static_cast< ITexture* > (pRessourceManager->GetRessource("Gui/PlayerWindow.bmp"));
	m_pMesh->SetTexture(pTexture);
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
	CGUIWindow::SetVisibility(bShow);

	if (bShow) {
		if (!m_pPlayer) {
			m_pPlayer = m_pEntityManager->GetPlayer();
			m_pPlayerCamera = m_pCameraManager->GetCameraFromType(ICameraManager::TCameraType::TLinked);
			InitCamera();
			InitLight();
		}
		SwitchToWindowCamera();
		DisplayInventory();
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

void CPlayerWindow::DisplayInventory()
{
	m_pInventory->SetVisibility(true);
	int numberItemInCurrentLine = 0;
	int yLine = 0;
	for (const pair<string, vector<IItem*>>& item : m_pPlayer->GetItems()) {
		for (IItem* pItemInstance : item.second) {
			string sPreviewPath = pItemInstance->GetPreviewPath();
			string sExtension;
			CStringUtils::GetExtension(sPreviewPath, sExtension);
			CStringUtils::GetFileNameWithoutExtension(sPreviewPath, sPreviewPath);
			sPreviewPath += "-64." + sExtension;
			CGUIItem* pItem = nullptr;
			try {
				pItem = new CGUIItem(m_oInterface, string("Textures/items/") + sPreviewPath);
			}
			catch (CEException& e) {
				pItem = new CGUIItem(m_oInterface, string("Textures/items/not-found-64.bmp"));
			}
			pItem->m_pItem = pItemInstance;
			pItem->m_pItem->SetOwner(m_pPlayer);
			pItem->m_pBorder = new CGUIWidget(m_oInterface, string("Textures/items/border-64.bmp"));
			pItem->m_pBorder->SetVisibility(false);
			pItem->AddWidget(pItem->m_pBorder);
			m_pInventory->AddWidget(pItem);
			CListener* pListener = new CListener;
			pItem->SetListener(pListener);
			pListener->SetEventCallBack(HandleItemEvent);

			int x = numberItemInCurrentLine * pItem->GetDimension().GetWidth();

			if (x + pItem->GetDimension().GetWidth() > m_pInventory->GetDimension().GetWidth()) {
				x = 0;
				numberItemInCurrentLine = 0;
				yLine += pItem->GetDimension().GetHeight();
			}
			pItem->SetRelativePosition(x, yLine);
			numberItemInCurrentLine++;
		}
	}
}

void CPlayerWindow::HandleItemEvent(IGUIManager::ENUM_EVENT nEvent, CGUIWidget* pWidget, int x, int y)
{
	CGUIWindow* pInventory = static_cast<CPlayerWindow*>(pWidget->GetParent());
	CPlayerWindow* pPlayerWindow = static_cast<CPlayerWindow*>(pInventory->GetParent());
	CGUIItem* pItem = static_cast<CGUIItem*>(pWidget);
	switch (nEvent) {
	case IGUIManager::EVENT_MOUSEMOVE:
	case IGUIManager::EVENT_MOUSEENTERED:
		pItem->m_pBorder->SetVisibility(true);
		//pItem->m_pBorder->SetRelativePosition(pItem->GetRelativePosition());
		break;
	case IGUIManager::EVENT_MOUSEEXITED:
		if (!pItem->m_pItem->IsWear())
			pItem->m_pBorder->SetVisibility(false);
		break;
	case IGUIManager::EVENT_LMOUSERELEASED:
		if (!pItem->m_pItem->IsWear())
			//pPlayerWindow->m_pPlayer->WearItem(pItem->m_pItem->GetEntityID());
			pItem->m_pItem->Wear();
		else
			//pPlayerWindow->m_pPlayer->UnWearItem(pItem->m_pItem->GetEntityID());
			pItem->m_pItem->UnWear();
		break;
	default:
		if(pItem->m_pItem->IsWear())
			pItem->m_pBorder->SetVisibility(true);
		break;
	}
}

CGUIItem::CGUIItem(EEInterface& oInterface, string sFileName) :
	CGUIWindow(oInterface, sFileName)
{
}

