#ifndef TRADE_WINDOW_H

#include "InventoryWindow.h"

class CGUIManager;
class ICharacter;

class CTradeWindow : public CGUIWindow
{
public:
	
	CTradeWindow(EEInterface& oInterface);

	void Open(ICharacter* pPlayer, ICharacter* pTrader);
	void Close();

private:

	void							UpdateCostWindow(int currentCost, int GoldAmount);

	CInventoryWindow*				m_pPlayerInventory = nullptr;
	CInventoryWindow*				m_pTraderInventory = nullptr;
	CGUIWindow*						m_pCostWindow = nullptr;
	//CGUIWindow*						m_pTradeButton;
	CGUIManager*					m_pGUIManager = nullptr;
	map<CGUIItem*, CGUIWidget*>		m_mItemsToBuy;
	int								m_nCurrentCost = 0;
	ICharacter*						m_pPlayer = nullptr;
	ICharacter*						m_pTrader = nullptr;
};

#endif // TRADE_WINDOW_H