#define GUIWINDOW_CPP

#include "guiwindow.h"
#include "Utils2/Rectangle.h"
#include "IRessource.h"
#include "Interface.h"

CGUIWindow::CGUIWindow(EEInterface& oInterface):
	CGUIWidget(oInterface),
	m_bGUIMode(false),
	m_bIsShown(false)
{
}

CGUIWindow::CGUIWindow(EEInterface& oInterface, string sFileName) :
	CGUIWidget(oInterface, sFileName),
	m_bGUIMode(false),
	m_bIsShown(false)
{

}

CGUIWindow::CGUIWindow(string fileName, EEInterface& oInterface, const CDimension& windowSize) :
	CGUIWidget(oInterface, fileName, windowSize.GetWidth(), windowSize.GetHeight()),
	m_bGUIMode(false),
	m_oCloseWindowCallback(nullptr),
	m_bIsShown(false)
{
	m_pGUIManager = static_cast<IGUIManager*>(oInterface.GetPlugin("GUIManager"));
}

CGUIWindow::CGUIWindow(string fileName, EEInterface& oInterface, int nWidth, int nHeight) :
	CGUIWidget(oInterface, fileName, nWidth, nHeight),
	m_bGUIMode(false),
	m_oCloseWindowCallback(nullptr),
	m_bIsShown(false)
{
}

CGUIWindow::CGUIWindow(EEInterface& oInterface, const CDimension& windowSize, int borderWidth, int color) :
	CGUIWidget(oInterface, windowSize.GetWidth(), windowSize.GetHeight(), borderWidth, color),
	m_bGUIMode(false),
	m_oCloseWindowCallback(nullptr),
	m_bIsShown(false)
{
}

CGUIWindow::CGUIWindow(EEInterface& oInterface, const CDimension& windowSize, const CRectangle& skin) :
	CGUIWidget(oInterface, windowSize, skin),
	m_bGUIMode(false),
	m_oCloseWindowCallback(nullptr),
	m_bIsShown(false)
{
}

CGUIWindow::~CGUIWindow()
{
	for (CGUIWidget* pWidget : m_vWidget) {
		delete pWidget;
	}
}

void CGUIWindow::SetGUIMode(bool bGUIMode)
{
	m_bGUIMode = bGUIMode;
}

void CGUIWindow::AddWidget(CGUIWidget* pWidget)
{
	deque<CGUIWidget*>::iterator itWidget = std::find_if(m_vWidget.begin(), m_vWidget.end(), [pWidget](CGUIWidget* pChild) {return (pChild == pWidget); });
	if (itWidget == m_vWidget.end()) {
		pWidget->SetParent(this);
		m_vWidget.push_back(pWidget);
		pWidget->SetRelativePosition(pWidget->GetRelativePosition().GetX(), pWidget->GetRelativePosition().GetY());
	}
}

deque<CGUIWidget*>::iterator CGUIWindow::RemoveWidget(CGUIWidget* pWidget, bool bDelete)
{
	deque<CGUIWidget*>::iterator itWidget = std::find(m_vWidget.begin(), m_vWidget.end(), pWidget);
	m_vWidget.erase(itWidget);
	pWidget->SetParent(nullptr);
	if (bDelete)
		delete pWidget;
	return itWidget;
}

void CGUIWindow::SetPosition(int x, int y)
{
	CGUIWidget::SetPosition(x, y);
	for (int i = 0; i < m_vWidget.size(); i++) {
		CGUIWidget* pWidget = m_vWidget[i];
		pWidget->SetPosition(pWidget->GetRelativePosition().GetX() + x, pWidget->GetRelativePosition().GetY() + y);
	}
}

void CGUIWindow::SetRelativePosition(float fPosX, float fPosY)
{
	CGUIWidget::SetRelativePosition(fPosX, fPosY);
	for (CGUIWidget* pWidget : m_vWidget) {
		pWidget->UpdatePosition();
	}
}

bool CGUIWindow::IsGUIMode()
{
	return m_bGUIMode;
}

size_t CGUIWindow::GetWidgetCount()const
{
	return m_vWidget.size();
}

CGUIWidget* CGUIWindow::GetWidget( unsigned int nIndex )
{
	if(nIndex < m_vWidget.size())
		return m_vWidget[nIndex];
	return nullptr;
}


void CGUIWindow::SetCloseWindowCallback(CloseWindowCallback callback)
{
	m_oCloseWindowCallback = callback;
}

bool CGUIWindow::IsVisible()
{
	return m_bVisible;
}


void CGUIWindow::Clear()
{
	for (CGUIWidget* pWidget : m_vWidget) {
		delete pWidget;
	}
	m_vWidget.clear();
}

void CGUIWindow::Display()
{
	CGUIWidget::Display();
	for (int i = 0; i < m_vWidget.size(); i++) {
		CGUIWidget* pWidget = m_vWidget[i];
		pWidget->Display();
	}
}

void CGUIWindow::UpdateCallback(int nCursorXPos, int nCursorYPos, IInputManager::TMouseButtonState eButtonState)
{
	CGUIWidget::UpdateCallback(nCursorXPos, nCursorYPos, eButtonState);
	size_t nWidgetCount = GetWidgetCount();
	for (size_t i = 0; i<nWidgetCount; i++)
	{
		CGUIWidget* pWidget = GetWidget((unsigned int)i);
		if(pWidget)
			pWidget->UpdateCallback(nCursorXPos, nCursorYPos, eButtonState);
	}
}

deque<CGUIWidget*>& CGUIWindow::GetChildren()
{
	return m_vWidget;
}

void CGUIWindow::Close()
{
	GetGUIManager()->RemoveWindow(this);
	m_bIsShown = false;
}

bool CGUIWindow::IsShown()
{
	return m_bIsShown;
}

void CGUIWindow::OnShow(bool bShow)
{
	if (!bShow && m_oCloseWindowCallback)
		m_oCloseWindowCallback(this);
	m_bIsShown = bShow;
}

IGUIManager* CGUIWindow::GetGUIManager()
{
	if(!m_pGUIManager)
		m_pGUIManager = static_cast<IGUIManager*>(m_oInterface.GetPlugin("GUIManager"));
	return m_pGUIManager;
}