#include "Interface.h"
#include "MapWindow.h"
#include "Utils2/Rectangle.h"
#include "IRessource.h"
#include "IEntity.h"
#include "GUIManager.h"

CMinimapWindow::CMinimapWindow(EEInterface& oInterface, ITexture* pMinimapTexture, int nWidth, int nHeight):
	CGUIWindow("Gui/map-bkg.bmp", oInterface, CDimension(nWidth, nHeight)),
	m_pGUIManager(static_cast<CGUIManager*>(oInterface.GetPlugin("GUIManager"))),
	m_oRenderer(static_cast<IRenderer&>(*oInterface.GetPlugin("Renderer"))),
	m_oMinimap(oInterface, nWidth, nHeight)
{
	CRectangle skin;

	skin.SetPosition(0, 0);
	skin.SetDimension(m_oMinimap.GetDimension());

	IRessourceManager* pRessourceManager = static_cast<IRessourceManager*>(oInterface.GetPlugin("RessourceManager"));
	IMesh* pQuadMap = CreateQuad(m_oRenderer, *pRessourceManager, m_oMinimap.GetDimension(), skin);
	pQuadMap->SetTexture(pMinimapTexture);
	pQuadMap->SetShader(m_pShader);
	m_oMinimap.SetQuad(pQuadMap);
	AddWidget(&m_oMinimap);
	m_oMinimap.Translate(4, 4);
}

CMinimapWindow::~CMinimapWindow()
{
}

void CMinimapWindow::Display()
{
	CGUIWindow::Display();
}


