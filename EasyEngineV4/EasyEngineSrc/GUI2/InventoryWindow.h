#ifndef INVENTORY_WINDOW_H

#include "GUIWindow.h"


class ICharacter;
class IItem;


class CGUIItem : public CGUIWindow
{
public:
	CGUIItem(EEInterface& oInterface, string sFileName);

	IItem*					m_pItem;
	CGUIWidget*				m_pBorder = nullptr;

};

class CInventoryWindow : public CGUIWindow, public IInventoryWindow
{
public:
	CInventoryWindow(EEInterface& oInterface, const CDimension& windowSize);
	void DisplayItems(ICharacter* pCharacter);
	void ClearItems();
	void SetInventoryMode(bool tradeMode);
	void SetSelectItemCallback(std::function<void(CGUIItem*)>);
	int GetTotalCost();
	int GetGoldAmount();

private:
	bool m_bInventoryMode = false;
	std::function<void(CGUIItem*)>  m_ItemSelectedCallback;
	CGUIWindow*						m_pDescriptionWindow = nullptr;
	CGUIManager*					m_pGUIManager = nullptr;
	int								m_nTotalCost = 0;
	int								m_nGoldAmount = 32;
	static void HandleItemEvent(IGUIManager::ENUM_EVENT nEvent, CGUIWidget* pWidget, int x, int y);
};



#endif INVENTORY_WINDOW_H