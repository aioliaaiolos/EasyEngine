#ifndef PLAYER_WINDOW_H
#define PLAYER_WINDOW_H

#include "IGUIManager.h"
#include "GUIWindow.h"

class CGUIWidget;
class CGUIManager;
class IRessourceManager;
class CRectangle;
class CDimension;
class IPlayer;
class ICamera;
class ILightEntity;
class ILight;
class IItem;

class CPlayerWindow : public CGUIWindow
{
public:
	CPlayerWindow(EEInterface& oInterface, const CDimension& windowSize);
	~CPlayerWindow();

	void					OnShow(bool bShow) override;

private:
	void					DisplayInventory();
	void					SwitchToWindowCamera();
	void					RestoreCamera();
	void					InitLight();
	void					InitCamera();

	static void				HandleItemEvent(IGUIManager::ENUM_EVENT nEvent, CGUIWidget*, int, int);

	ICameraManager*			m_pCameraManager;
	ICamera*				m_pWindowCamera;
	ICamera*				m_pPlayerCamera;
	CGUIWindow*				m_pInventory;
	CGUIWidget*				m_pArmorWindow;
	CGUIWidget*				m_pWindowBackground;
	CGUIManager*			m_pGUIManager;
	EEInterface&			m_oInterface;

	IEntityManager*			m_pEntityManager;
	IPlayer*				m_pPlayer;
	ILightEntity*			m_pLightEntity;
	ILight*					m_pLight;
	float					m_fLightIntensity;
	
};

class CGUIItem : public CGUIWindow
{
public:
	CGUIItem(EEInterface& oInterface, string sFileName);

	IItem*					m_pItem;
	CGUIWidget*				m_pBorder = nullptr;

};

#endif // PLAYER_WINDOW_H
