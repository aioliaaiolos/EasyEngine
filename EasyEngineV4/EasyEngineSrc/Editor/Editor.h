#pragma once

#include "ICamera.h"
#include "IEditor.h"

class IEntityManager;
class ISceneManager;
class IInputManager;
class ICameraManager;
class IConsole;
class ICollisionManager;
class IGeometryManager;
class EEInterface;
class CPlugin;
class CEditorManager;
class IRenderer;
class IHud;

class CEditor : public IEditor
{
public:
	CEditor(EEInterface& oInterface, ICameraManager::TCameraType cameraType);

	virtual void			SetEditionMode(bool bEditionMode) override;

protected:	

	static void HandleEditorManagerCreation(CPlugin* plugin, IBaseObject* pData);

	CEditorManager*			m_pEditorManager;
	IEntityManager&			m_oEntityManager;
	ISceneManager&			m_oSceneManager;
	EEInterface&			m_oInterface;
	IInputManager&			m_oInputManager;
	IRenderer&				m_oRenderer;
	ICameraManager&			m_oCameraManager;
	ICollisionManager&		m_oCollisionManager;
	IGeometryManager&		m_oGeometryManager;
	IConsole&				m_oConsole;
	ICamera*				m_pEditorCamera;
	bool					m_bEditionMode;
	IHud&					m_oHud;
	int						m_nHudX;
	int						m_nHudY;
	int						m_nHudLineHeight;

};