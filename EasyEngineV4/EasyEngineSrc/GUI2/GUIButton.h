#include "GUIWindow.h"


class CGUIButton : public CGUIWindow
{
public:
	CGUIButton(string caption, string imagePath, EEInterface& oInterface, int width, int height);
	CGUIButton(string caption, EEInterface& oInterface, int width, int height, int borderWidth = 0, int color=0);


private:
	CGUIManager*	m_pGUIManager = nullptr;
	CGUIWidget*		m_pCaption = nullptr;
};