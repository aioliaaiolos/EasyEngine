#pragma once
#include "MobileEntity.h"
#include "Utils2\Position.h"

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
	INode* 			GetEntityInVisor(int x, int y);
	void			CollectSelectableEntity(vector<INode*>& entities);
	void			Loot(CItem* pItem);

	IGUIManager&	m_oGUIManager;
	IGUIWindow*		m_pPlayerWindow;
	ICamera*		m_pLinkCamera;
	CPosition		m_oVisorPos;
	INode*			m_pEntityInVisor;
};

