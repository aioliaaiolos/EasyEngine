#ifndef GUIWINDOW_H
#define GUIWINDOW_H

#include <vector>
#include <queue>
#include "GUIWidget.h"
#include "IGUIManager.h"
#include "ILoader.h"

class ITexture;
class IRessourceManager;
class CRectangle;
class CDimension;

class CGUIWindow  : public CGUIWidget, public virtual IGUIWindow
{
public:
									CGUIWindow();
									CGUIWindow(string fileName, EEInterface& oInterface, const CDimension& windowSize);
									CGUIWindow(string fileName, EEInterface& oInterface, int nWidth, int nHeight);
									CGUIWindow(EEInterface& oInterface, const CDimension& windowSize, const CRectangle& skin);
	virtual							~CGUIWindow();
	void							AddWidget(CGUIWidget* pWidget);
	deque<CGUIWidget*>::iterator	RemoveWidget(CGUIWidget* pWidget);
	size_t							GetWidgetCount()const;
	CGUIWidget*						GetWidget(unsigned int nIndex);
	void							SetVisibility(bool bVisible);
	bool							IsVisible();
	void							Clear();
	void							Display();
	void							SetPosition(float fPosX, float fPosY);
	void							SetRelativePosition(float fPosX, float fPosY) override;
	bool							IsGUIMode();
	void							SetGUIMode(bool bGUIMode);	
	void							UpdateCallback(int nCursorXPos, int nCursorYPos, IInputManager::TMouseButtonState eButtonState);
	deque<CGUIWidget*>&				GetChildren();
	virtual void					OnShow(bool bShow);

protected:
	deque< CGUIWidget*>			m_vWidget;
	bool						m_bVisible;
	bool						m_bGUIMode;
};



#endif //GUIWINDOW_H