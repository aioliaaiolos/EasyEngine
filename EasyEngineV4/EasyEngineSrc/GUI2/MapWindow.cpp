#include "Interface.h"
#include "MapWindow.h"
#include "Utils2/Rectangle.h"
#include "IRessource.h"
#include "IEntity.h"
#include "GUIManager.h"

CMinimapWindow::CMinimapWindow(EEInterface& oInterface, ITexture* pMinimapTexture, int nWidth, int nHeight):
	CGUIWindow("Gui/map-bkg.bmp", oInterface, CDimension(520, 296)),
	m_pGUIManager(static_cast<CGUIManager*>(oInterface.GetPlugin("GUIManager"))),
	m_oRenderer(static_cast<IRenderer&>(*oInterface.GetPlugin("Renderer"))),
	m_oMinimap(oInterface, 512, 288)
{
	unsigned int screenWidth, screenHeight;
	m_oRenderer.GetResolution(screenWidth, screenHeight);
	// SetPosition(screenWidth - GetDimension().GetWidth(), 0);
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



CShadowMapWindow::CShadowMapWindow(EEInterface& oInterface, IScene& oScene, int nWidth, int nHeight) :
	CGUIWindow("Gui/map-bkg.bmp", oInterface, CDimension(520, 296)),
	m_pGUIManager(static_cast<CGUIManager*>(oInterface.GetPlugin("GUIManager"))),
	m_oRenderer(static_cast<IRenderer&>(*oInterface.GetPlugin("Renderer"))),
	m_oScene(oScene),
	m_oShadowMap(oInterface, 512, 288)
{
	unsigned int screenWidth, screenHeight;
	m_oRenderer.GetResolution(screenWidth, screenHeight);
	SetPosition(screenWidth - GetDimension().GetWidth(), 0);
	CRectangle skin;

	skin.SetPosition(0, 0);
	skin.SetDimension(m_oShadowMap.GetDimension());

	IRessourceManager* pRessourceManager = static_cast<IRessourceManager*>(oInterface.GetPlugin("RessourceManager"));
	IMesh* pQuadMap = CreateQuad(m_oRenderer, *pRessourceManager, m_oShadowMap.GetDimension(), skin);
	pQuadMap->SetTexture(m_oScene.GetShadowMapTexture());
	pQuadMap->SetShader(m_pShader);
	m_oShadowMap.SetQuad(pQuadMap);
	AddWidget(&m_oShadowMap);
	m_oShadowMap.Translate(4, 4);
}

CShadowMapWindow::~CShadowMapWindow()
{
}

void CShadowMapWindow::Display()
{
	CGUIWindow::Display();
}
