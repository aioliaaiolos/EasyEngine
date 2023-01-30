#pragma once
#include "MobileEntity.h"

class IGUIWindow;
class ICamera;

class CPlayer :	public CCharacter, public IPlayer
{
public:
	CPlayer(EEInterface& oInterface, string sFileName);
	virtual ~CPlayer();

	void	Action();
	void	ToggleDisplayPlayerWindow();
	void	Update();


protected:
	IGUIManager&	m_oGUIManager;
	IGUIWindow*		m_pPlayerWindow;
	ICamera*		m_pLinkCamera;
};

