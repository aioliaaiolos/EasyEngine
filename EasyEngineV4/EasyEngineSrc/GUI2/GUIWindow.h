#ifndef GUIWINDOW_H
#define GUIWINDOW_H

#include <vector>
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
								CGUIWindow(string fileName, EEInterface& oInterface, const CDimension& windowSize);
								CGUIWindow(EEInterface& oInterface, const CDimension& windowSize, const CRectangle& skin);
	virtual						~CGUIWindow(void);
	void						AddWidget(CGUIWidget* pWidget);
	size_t						GetWidgetCount()const;
	CGUIWidget*					GetWidget(unsigned int nIndex);
	void						SetVisibility(bool bVisible);
	bool						IsVisible();
	void						Clear();
	void						Display();
	void						SetPosition(float fPosX, float fPosY);
	bool						IsGUIMode();
	void						SetGUIMode(bool bGUIMode);	
	void						UpdateCallback(int nCursorXPos, int nCursorYPos, IInputManager::TMouseButtonState eButtonState);
	virtual void				OnShow(bool bShow);

protected:
	std::vector< CGUIWidget* >	m_vWidget;
	bool						m_bVisible;
	bool						m_bGUIMode;
};



#endif //GUIWINDOW_H