#include "InventoryWindow.h"

#include "IEntity.h"
#include "GUIManager.h"
#include "../Utils2/StringUtils.h"
#include "Listener.h"
#include "Utils2/rectangle.h"

CInventoryWindow::CInventoryWindow(EEInterface& oInterface, const CDimension& windowSize) : 
	CGUIWindow(oInterface, windowSize, CRectangle(0, 0, windowSize.GetWidth(), windowSize.GetHeight()))
{
	IRessourceManager* pRessourceManager = static_cast<IRessourceManager*>(oInterface.GetPlugin("RessourceManager"));
	ITexture* pTexture = static_cast< ITexture* > (pRessourceManager->GetRessource("Gui/PlayerWindow.bmp"));
	m_pMesh->SetTexture(pTexture);
	m_pDescriptionWindow = new CGUIWindow("gui/PlayerWindow.bmp", oInterface, 500, 300);
	m_pDescriptionWindow->m_sUserData = "test position";
	AddWidget(m_pDescriptionWindow);
	m_pDescriptionWindow->SetRelativePosition(50, 400);
	m_pDescriptionWindow->SetVisibility(false);
	m_oInterface.HandlePluginCreation("GUIManager", [this](CPlugin* pGUIManager)
	{
		m_pGUIManager = static_cast<CGUIManager*>(pGUIManager);
	});
}

void CInventoryWindow::SetSelectItemCallback(std::function<void(CGUIItem*)> callback)
{
	m_ItemSelectedCallback = callback;
}

int CInventoryWindow::GetTotalCost()
{
	return m_nTotalCost;
}

int CInventoryWindow::GetGoldAmount()
{
	return m_nGoldAmount;
}

void CInventoryWindow::DisplayItems(ICharacter* pCharacter)
{
	SetVisibility(true);
	int numberItemInCurrentLine = 0;
	int yLine = 0;
	m_nTotalCost = 0;
	for (const pair<string, vector<IItem*>>& item : pCharacter->GetItems()) {
		for (IItem* pItemInstance : item.second) {
			m_nTotalCost += pItemInstance->GetValue();
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
			pItem->m_pItem->SetOwner(pCharacter);
			pItem->m_pBorder = new CGUIWidget(m_oInterface, string("Textures/items/border-64.bmp"));
			pItem->m_pBorder->SetVisibility(false);
			pItem->AddWidget(pItem->m_pBorder);
			AddWidget(pItem);
			CListener* pListener = new CListener;
			pItem->SetListener(pListener);
			pListener->SetEventCallBack([this](IGUIManager::ENUM_EVENT nEvent, CGUIWidget* pWidget, int x, int y) 
			{
				CGUIItem* pItem = static_cast<CGUIItem*>(pWidget);
				switch (nEvent) {
				case IGUIManager::EVENT_MOUSEMOVE:
				case IGUIManager::EVENT_MOUSEENTERED:
				{
					int yMargin = 20;
					int x = 20, y = m_pGUIManager->GetCurrentFontEspacementY();
					pItem->m_pBorder->SetVisibility(true);
					m_pDescriptionWindow->Clear();
					m_pDescriptionWindow->SetVisibility(true);
					string desc;
					CStringUtils::DecodeString(pItem->m_pItem->GetDescription(), desc);
					if (!desc.empty()) {
						CGUIWidget* pDescription = m_pGUIManager->CreateStaticText(desc);
						m_pDescriptionWindow->AddWidget(pDescription);
						pDescription->SetRelativePosition(x, yMargin);
					}
					ostringstream oss;
					oss << "Valeur " << pItem->m_pItem->GetValue();
					CGUIWidget* pValue = m_pGUIManager->CreateStaticText(oss.str());
					m_pDescriptionWindow->AddWidget(pValue);
					pValue->SetRelativePosition(x, yMargin + y);

					oss.str("");
					oss << "Armure " << pItem->m_pItem->GetArmor();
					CGUIWidget* pArmor = m_pGUIManager->CreateStaticText(oss.str());
					m_pDescriptionWindow->AddWidget(pArmor);
					pArmor->SetRelativePosition(x, yMargin + 2 * y);

					if (pItem->m_pItem->GetDamage() > -1) {
						oss.str("");
						oss << "Dommages " << pItem->m_pItem->GetDamage();
						CGUIWidget* pDamage = m_pGUIManager->CreateStaticText(oss.str());
						m_pDescriptionWindow->AddWidget(pDamage);
						pDamage->SetRelativePosition(x, yMargin + 3 * y);
					}

					break;
				}
				case IGUIManager::EVENT_MOUSEEXITED:
					if (!pItem->m_pItem->IsWear())
						pItem->m_pBorder->SetVisibility(false);
					m_pDescriptionWindow->Clear();
					m_pDescriptionWindow->SetVisibility(false);					
					break;
				case IGUIManager::EVENT_LMOUSERELEASED:
					if (m_bInventoryMode) {
						if (!pItem->m_pItem->IsWear())
							pItem->m_pItem->Wear();
						else
							pItem->m_pItem->UnWear();
					}
					else {
						m_ItemSelectedCallback(pItem);
					}
					break;
				default:
					if (pItem->m_pItem->IsWear())
						pItem->m_pBorder->SetVisibility(true);
					break;
				}
			});

			int x = numberItemInCurrentLine * pItem->GetDimension().GetWidth();

			if (x + pItem->GetDimension().GetWidth() > GetDimension().GetWidth()) {
				x = 0;
				numberItemInCurrentLine = 0;
				yLine += pItem->GetDimension().GetHeight();
			}
			pItem->SetRelativePosition(x, yLine);
			numberItemInCurrentLine++;
		}
	}
}

void CInventoryWindow::ClearItems()
{
	//for (CGUIWidget* pChild : GetChildren()) {
	deque<CGUIWidget*>::iterator itChild = GetChildren().begin();
	while (itChild != GetChildren().end()) {
		CGUIWidget* pChild = *itChild;
		CGUIItem* pItem = dynamic_cast<CGUIItem*>(pChild);
		if (pItem) {
			//itChild = RemoveWidget(pItem);
			itChild = m_vWidget.erase(itChild);
		}
		else
			itChild++;
	}
}

void CInventoryWindow::SetInventoryMode(bool tradeMode)
{
	m_bInventoryMode = tradeMode;
}

#if 0

void CInventoryWindow::HandleItemEvent(IGUIManager::ENUM_EVENT nEvent, CGUIWidget* pWidget, int x, int y)
{
	CGUIItem* pItem = static_cast<CGUIItem*>(pWidget);
	switch (nEvent) {
	case IGUIManager::EVENT_MOUSEMOVE:
	case IGUIManager::EVENT_MOUSEENTERED:
		pItem->m_pBorder->SetVisibility(true);
		break;
	case IGUIManager::EVENT_MOUSEEXITED:
		if (!pItem->m_pItem->IsWear())
			pItem->m_pBorder->SetVisibility(false);
		break;
	case IGUIManager::EVENT_LMOUSERELEASED:
		if (!pItem->m_pItem->IsWear())
			pItem->m_pItem->Wear();
		else
			pItem->m_pItem->UnWear();
		break;
	default:
		if (pItem->m_pItem->IsWear())
			pItem->m_pBorder->SetVisibility(true);
		break;
	}
}

#endif // 0