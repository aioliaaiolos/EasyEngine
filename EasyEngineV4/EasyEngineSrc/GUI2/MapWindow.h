#pragma once
#include "GUIWindow.h"

class CGUIManager;
class ITexture;

class CMinimapWindow :	public CGUIWindow
{
public:
	CMinimapWindow(EEInterface& oInterface, ITexture* pMinimapTexture, int nWidth, int nHeight);
	~CMinimapWindow();

	void				Display();

protected:
	
private:
	CGUIManager*		m_pGUIManager;
	// IScene&				m_oScene;
	IRenderer&			m_oRenderer;
	CGUIWidget			m_oMinimap;
};

class CShadowMapWindow : public CGUIWindow
{
public:
	CShadowMapWindow(EEInterface& oInterface, IScene& oScene, int nWidth, int nHeight);
	~CShadowMapWindow();

	void				Display();

protected:

private:
	CGUIManager*		m_pGUIManager;
	IScene&				m_oScene;
	IRenderer&			m_oRenderer;
	CGUIWidget			m_oShadowMap;
};