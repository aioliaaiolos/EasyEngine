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
													CGUIWindow(EEInterface& oInterface);
													CGUIWindow(EEInterface& oInterface, string fileName);
													CGUIWindow(string fileName, EEInterface& oInterface, const CDimension& windowSize);
													CGUIWindow(string fileName, EEInterface& oInterface, int nWidth, int nHeight);
													CGUIWindow(EEInterface& oInterface, const CDimension& windowSize, int borderWidth = 0, int color = 0);
													CGUIWindow(EEInterface& oInterface, const CDimension& windowSize, const CRectangle& skin);
	virtual											~CGUIWindow();
	void											AddWidget(CGUIWidget* pWidget);
	deque<CGUIWidget*>::iterator					RemoveWidget(CGUIWidget* pWidget, bool bDelete = true);
	size_t											GetWidgetCount()const;
	CGUIWidget*										GetWidget(unsigned int nIndex);
	void											Display() override;
	void											SetCloseWindowCallback(CloseWindowCallback callback) override;
	bool											IsVisible();
	void											Clear();
	void											SetPosition(int x, int y) override;
	void											SetRelativePosition(float fPosX, float fPosY) override;
	bool											IsGUIMode();
	void											SetGUIMode(bool bGUIMode);	
	void											UpdateCallback(int nCursorXPos, int nCursorYPos, IInputManager::TMouseButtonState eButtonState);
	deque<CGUIWidget*>&								GetChildren();
	void											Close();
	bool											IsShown();
	virtual void									OnShow(bool bShow);

protected:
	IGUIManager*									GetGUIManager();

	deque< CGUIWidget*>								m_vWidget;
	bool											m_bGUIMode;
	CloseWindowCallback								m_oCloseWindowCallback;
	IGUIManager*									m_pGUIManager = nullptr;
	bool											m_bIsShown;
};



#endif //GUIWINDOW_H