#include "MapWindow.h"
#include "Utils2/Rectangle.h"
#include "IRessource.h"
#include "IEntity.h"
#include "GUIManager.h"

CMinimapWindow::CMinimapWindow(IGUIManager* pGUIManager, IScene& oScene, IRessourceManager& oRessourceManager, IRenderer& oRenderer, int nWidth, int nHeight):
	CGUIWindow("Gui/map-bkg.bmp", oRessourceManager, oRenderer, CDimension(520, 296)),
	m_pGUIManager((CGUIManager*)pGUIManager),
	m_oScene(oScene),
	m_oRenderer(oRenderer),
	m_oMinimap(512, 288)
{
	unsigned int screenWidth, screenHeight;
	m_oRenderer.GetResolution(screenWidth, screenHeight);
	SetPosition(screenWidth - GetDimension().GetWidth(), 0);
	CRectangle skin;

	skin.SetPosition(0, 0);
	skin.SetDimension(m_oMinimap.GetDimension());

	IMesh* pQuadMap = CreateQuad(m_oRenderer, oRessourceManager, m_oMinimap.GetDimension(), skin);
	pQuadMap->SetTexture(m_oScene.GetMinimapTexture());
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