#ifndef PLAYER_WINDOW_H
#define PLAYER_WINDOW_H

#include "IGUIManager.h"
#include "GUIWindow.h"

class CGUIWidget;
class CGUIManager;
class IRessourceManager;
class CRectangle;
class CDimension;

class CPlayerWindow : public CGUIWindow
{
public:
	CPlayerWindow(EEInterface& oInterface, const CDimension& windowSize);
	~CPlayerWindow();

	void	SetVisibility(bool bVisible);


private:
	CGUIWindow*				m_pInventory;
	CGUIWidget*				m_pArmorWindow;
	CGUIWidget*				m_pWindowBackground;
	CGUIManager*			m_pGUIManager;
};

#endif // PLAYER_WINDOW_H
