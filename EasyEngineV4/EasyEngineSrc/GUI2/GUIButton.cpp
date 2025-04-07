#include "GUIButton.h"
#include "Interface.h"
#include "GUIManager.h"

CGUIButton::CGUIButton(string caption, std::string imagePath, EEInterface& oInterface, int width, int height) :
	CGUIWindow(imagePath, oInterface, width, height)
{
	oInterface.HandlePluginCreation("GUIManager", [this](CPlugin* pPlugin) 
	{
		m_pGUIManager = static_cast<CGUIManager*>(pPlugin);
		m_pCaption = m_pGUIManager->CreateStaticText("Acheter");
		AddWidget(m_pCaption);
		m_pCaption->SetRelativePosition((GetDimension().GetWidth() - m_pCaption->GetDimension().GetWidth()) / 2,
			(GetDimension().GetHeight() - m_pCaption->GetDimension().GetHeight()) / 2);
	});
}

CGUIButton::CGUIButton(string caption, EEInterface& oInterface, int width, int height, int borderWidth, int color):
	CGUIWindow(oInterface, CDimension(width, height), borderWidth, color)
{
	oInterface.HandlePluginCreation("GUIManager", [this](CPlugin* pPlugin)
	{
		m_pGUIManager = static_cast<CGUIManager*>(pPlugin);
		m_pCaption = m_pGUIManager->CreateStaticText("Acheter");
		AddWidget(m_pCaption);
		m_pCaption->SetRelativePosition((GetDimension().GetWidth() - m_pCaption->GetDimension().GetWidth()) / 2,
			(GetDimension().GetHeight() - m_pCaption->GetDimension().GetHeight()) / 2);
	});
}