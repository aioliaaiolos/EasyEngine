#include "Interface.h"
#include "Player.h"
#include "NPCEntity.h"
#include "IGUIManager.h"
#include "EntityManager.h"
#include "ICamera.h"

CPlayer::CPlayer(EEInterface& oInterface, string sFileName) :
	CMobileEntity(oInterface, sFileName, "Player"),
	m_oGUIManager(static_cast<IGUIManager&>(*oInterface.GetPlugin("GUIManager"))),
	m_pPlayerWindow(NULL)
{
	m_sTypeName = "Player";
	m_sName = "Player";
	m_pPlayerWindow = m_oGUIManager.CreatePlayerWindow(600, 800);
	ICameraManager& oCameraManager = static_cast<ICameraManager&>(*oInterface.GetPlugin("CameraManager"));
	m_pLinkCamera = oCameraManager.CreateCamera(ICameraManager::TLinked, 60.f);
}


CPlayer::~CPlayer()
{
}


void CPlayer::Action()
{
	if (m_oGUIManager.IsWindowDisplayed(m_oGUIManager.GetTopicsWindow())) {
		m_oGUIManager.RemoveWindow(m_oGUIManager.GetTopicsWindow());
	}
	else {
		CNPCEntity* pSpeaker = NULL;
		float fMinDistance = 200.f;
		for (int i = 0; i < m_pParent->GetChildCount(); i++) {
			CNPCEntity* pNPC = dynamic_cast<CNPCEntity*>(m_pParent->GetChild(i));
			if (pNPC) {
				float d = GetDistance(pNPC);
				if ((d < 150.f) && (fMinDistance > d)) {
					fMinDistance = d;
					pSpeaker = pNPC;
				}
			}
		}
		if (pSpeaker) {
			string sId;
			pSpeaker->GetEntityName(sId);
			m_oGUIManager.GetTopicsWindow()->SetSpeakerId(sId);
			m_oGUIManager.AddWindow(m_oGUIManager.GetTopicsWindow());
		}
	}
}

void CPlayer::ToggleDisplayPlayerWindow()
{
	bool bDisplay = m_oGUIManager.IsWindowDisplayed(m_pPlayerWindow);
	if (!bDisplay)
		m_oGUIManager.AddWindow(m_pPlayerWindow);
	else
		m_oGUIManager.RemoveWindow(m_pPlayerWindow);
}

void CPlayer::Update()
{
	CMobileEntity::Update();
}