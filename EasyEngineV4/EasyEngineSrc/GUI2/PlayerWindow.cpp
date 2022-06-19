#include "Interface.h"
#include "PlayerWindow.h"
#include "GUIManager.h"
#include "Utils2/rectangle.h"
#include "IRessource.h"

CPlayerWindow::CPlayerWindow(EEInterface& oInterface, const CDimension& windowSize):
	CGUIWindow(oInterface, windowSize, CRectangle(0, 0, windowSize.GetWidth(), windowSize.GetHeight())),
	m_pGUIManager(static_cast<CGUIManager*>(oInterface.GetPlugin("GUIManager"))),
	m_pWindowBackground(NULL)
{
	m_pArmorWindow = new CGUIWidget(windowSize.GetWidth(), windowSize.GetHeight());
	SetPosition(100, 100);

	IRessourceManager* pRessourceManager = static_cast<IRessourceManager*>(oInterface.GetPlugin("RessourceManager"));
	ITexture* pTexture = static_cast< ITexture* > (pRessourceManager->GetRessource("Gui/PlayerWindow.bmp"));
	m_pMesh->SetTexture(pTexture);
	SetGUIMode(true);
}

CPlayerWindow::~CPlayerWindow()
{
}

void CPlayerWindow::SetVisibility(bool bVisible)
{
	CGUIWindow::SetVisibility(bVisible);
}
