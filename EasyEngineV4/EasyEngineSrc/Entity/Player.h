#pragma once
#include "MobileEntity.h"
#include "Utils2\Position.h"

class IGUIWindow;
class CLinkedCamera;

class CPlayer :	public CCharacter, public IPlayer
{
public:
	CPlayer(EEInterface& oInterface, string sFileName);
	virtual ~CPlayer();

	void	Action() override;
	void	ToggleDisplayPlayerWindow() override;
	void	Update();
	void	SwitchToFirstPerson(bool firstPerson) override;

protected:
	INode* 			GetEntityInVisor(int x, int y);
	void			CollectSelectableEntity(vector<INode*>& entities);
	void			Loot(CItem* pItem);

	IGUIManager&	m_oGUIManager;
	IGUIWindow*		m_pPlayerWindow;
	CLinkedCamera*	m_pLinkCamera;
	CPosition		m_oVisorPos;
	INode*			m_pEntityInVisor;
};

