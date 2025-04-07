#include "TradeWindow.h"
#include "Interface.h"
#include "IEntity.h"
#include "GUIManager.h"
#include "GUIButton.h"

CTradeWindow::CTradeWindow(EEInterface& oInterface) : 
	CGUIWindow(oInterface)
{
	int baseColor = 0xBB080c0e;
	int buttonColor = 0xBB171F23;
	m_pPlayerInventory = new CInventoryWindow(oInterface, CDimension(600, 800));
	m_pTraderInventory = new CInventoryWindow(oInterface, CDimension(600, 800));
	m_pCostWindow = new CGUIWindow(oInterface, CDimension(200, 100), 1, buttonColor);
	AddWidget(m_pCostWindow);
	m_pCostWindow->SetRelativePosition(550, 850);

	CGUIButton* pTradeButton = new CGUIButton("Acheter", m_oInterface, 200, 100, 1, buttonColor);
	AddWidget(pTradeButton);
	pTradeButton->SetRelativePosition(800, 850);

	CListener* pTradeButtonListener = new CListener;
	pTradeButton->SetListener(pTradeButtonListener);
	pTradeButtonListener->SetEventCallBack([this](IGUIManager::ENUM_EVENT e, CGUIWidget* pWidget, int x, int y) 
	{
		if (e == IGUIManager::ENUM_EVENT::EVENT_LMOUSERELEASED) {
			map<CGUIItem*, CGUIWidget*>::iterator itItem = m_mItemsToBuy.begin();
			while(itItem != m_mItemsToBuy.end()) {
				CGUIWidget* pCover = itItem->second;
				if (pCover->IsVisible()) {
					IItem* pItem = itItem->first->m_pItem;
					m_pTrader->RemoveItem(pItem->GetIDStr());
					m_pPlayer->AddItem(pItem->GetIDStr());
					itItem = m_mItemsToBuy.erase(itItem);
				}
				else
					itItem++;
			}
			m_pPlayer->SetGoldAmount(m_pPlayer->GetGoldAmount() - m_nCurrentCost);
			m_pPlayerInventory->ClearItems();
			m_pTraderInventory->ClearItems();
			m_pPlayerInventory->DisplayItems(m_pPlayer);
			m_pTraderInventory->DisplayItems(m_pTrader);
			UpdateCostWindow(0, m_pPlayer->GetGoldAmount());
		}
	});

	m_pTraderInventory->SetInventoryMode(false);
	m_pPlayerInventory->SetRelativePosition(0, 0);
	m_pTraderInventory->SetRelativePosition(800, 0);
	AddWidget(m_pPlayerInventory);
	AddWidget(m_pTraderInventory);

	oInterface.HandlePluginCreation("GUIManager", [this](CPlugin* pPlugin) 
	{
		m_pGUIManager = static_cast<CGUIManager*>(pPlugin);
	});

	m_pTraderInventory->SetSelectItemCallback([this](CGUIItem* pItem) {
		m_pCostWindow->Clear();
		CGUIWidget* pCover = nullptr;
		map<CGUIItem*, CGUIWidget*>::iterator itItem = m_mItemsToBuy.find(pItem);
		if (itItem == m_mItemsToBuy.end()) {
			pCover = new CGUIWidget(m_oInterface, string("Textures/items/cover-64.bmp"));
			pCover->SetVisibility(false);
			m_mItemsToBuy[pItem] = pCover;
			pItem->AddWidget(pCover);
		}
		else
			pCover = itItem->second;

		if (pCover->IsVisible()) {
			pCover->SetVisibility(false);
			m_nCurrentCost -= pItem->m_pItem->GetValue();
		}
		else {
			pCover->SetVisibility(true);
			m_nCurrentCost += pItem->m_pItem->GetValue();
		}
		UpdateCostWindow(m_nCurrentCost, m_pPlayer->GetGoldAmount());
	});
}

void CTradeWindow::Open(ICharacter* pPlayer, ICharacter* pTrader)
{
	m_pPlayer = pPlayer;
	m_pTrader = pTrader;
	SetGUIMode(true);
	m_pGUIManager->AddWindow(this);
	SetPosition(300, 50);
	UpdateCostWindow(0, m_pPlayer->GetGoldAmount());
	m_pPlayerInventory->DisplayItems(pPlayer);
	m_pTraderInventory->DisplayItems(pTrader);
}

void CTradeWindow::Close()
{
	SetGUIMode(false);
	m_pGUIManager->RemoveWindow(this);
	m_pPlayerInventory->ClearItems();
	m_pTraderInventory->ClearItems();
}

void CTradeWindow::UpdateCostWindow(int currentCost, int GoldAmount)
{
	m_pCostWindow->Clear();
	CGUIWindow* pCostText = new CGUIWindow(m_oInterface);
	CGUIWidget* pCurrentCost = m_pGUIManager->CreateStaticText(std::to_string(currentCost));
	pCostText->AddWidget(pCurrentCost);
	pCurrentCost->SetRelativePosition(0, 0);
	CGUIWidget* pSlash = m_pGUIManager->CreateStaticText("/");
	pCostText->AddWidget(pSlash);
	pSlash->SetRelativePosition(20, 0);
	CGUIWidget* pTotalGold = m_pGUIManager->CreateStaticText(std::to_string(m_pPlayer->GetGoldAmount()));
	pCostText->AddWidget(pTotalGold);
	pTotalGold->SetRelativePosition(40, 0);
	m_pCostWindow->AddWidget(pCostText);
	pCostText->SetRelativePosition((m_pCostWindow->GetDimension().GetWidth() - pCurrentCost->GetDimension().GetWidth() * 6) / 2,
		(m_pCostWindow->GetDimension().GetHeight() - pCurrentCost->GetDimension().GetHeight()) / 2);
}