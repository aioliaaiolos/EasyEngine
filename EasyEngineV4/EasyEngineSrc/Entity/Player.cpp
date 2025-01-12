#include "Interface.h"
#include "Player.h"
#include "NPCEntity.h"
#include "IGUIManager.h"
#include "EntityManager.h"
#include "ICamera.h"
#include "IGeometry.h"
#include "Item.h"
#include "Scene.h"

CPlayer::CPlayer(EEInterface& oInterface, string sFileName) :
	CCharacter(oInterface, sFileName, "Player"),
	m_oGUIManager(static_cast<IGUIManager&>(*oInterface.GetPlugin("GUIManager"))),
	m_pPlayerWindow(NULL)
{
	m_sTypeName = "Player";
	m_sName = "Player";
	m_pPlayerWindow = m_oGUIManager.CreatePlayerWindow(600, 800);
	ICameraManager& oCameraManager = static_cast<ICameraManager&>(*oInterface.GetPlugin("CameraManager"));
	m_pLinkCamera = oCameraManager.CreateCamera(ICameraManager::TLinked, 60.f);

	unsigned int nWidth, nHeight;
	m_oRenderer.GetResolution(nWidth, nHeight);
	m_oVisorPos.SetX(nWidth / 2);
	m_oVisorPos.SetY(nHeight / 2);
}


CPlayer::~CPlayer()
{
}


void CPlayer::Action()
{
	IGUIWindow* pTopicWindow = m_oGUIManager.GetTopicsWindow();
	if (m_oGUIManager.IsWindowDisplayed(pTopicWindow))
		pTopicWindow->Close();
	else {
		if (m_pEntityInVisor) {
			CNPCEntity* pSpeaker = dynamic_cast<CNPCEntity*>(m_pEntityInVisor);
			if (pSpeaker) {
				m_oGUIManager.GetTopicsWindow()->SetSpeakerId(pSpeaker->GetIDStr());
				m_oGUIManager.AddWindow(m_oGUIManager.GetTopicsWindow());
			}
			else {
				CItem* pItem = dynamic_cast<CItem*>(m_pEntityInVisor);
				Loot(pItem);
			}
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
	CCharacter::Update();
	unsigned int nWidth, nHeight;
	m_oRenderer.GetResolution(nWidth, nHeight);
	CMatrix p;
	CVector ray, origin;
	m_oRenderer.GetProjectionMatrix(p);
	m_pEntityInVisor = GetEntityInVisor(m_oVisorPos.GetX(), m_oVisorPos.GetY());
	if(m_pEntityInVisor)
		m_oGUIManager.Print(m_pEntityInVisor->GetIDStr(), m_oVisorPos.GetX(), m_oVisorPos.GetY(), IGUIManager::TFontColor::eWhite);
}

void CPlayer::CollectSelectableEntity(vector<INode*>& entities)
{
	INode* pParent = GetParent();
	if (pParent) {
		for (int i = 0; i < pParent->GetChildCount(); i++) {
			INode* pChild = pParent->GetChild(i);
			if (pChild->GetTypeName() == "Item" || pChild->GetTypeName() == "NPC") {
				entities.push_back(pChild);
			}
		}
	}
}

void CPlayer::Loot(CItem* pItem)
{
	AddItem(pItem);
	pItem->Unlink();
}

INode* CPlayer::GetEntityInVisor(int x, int y)
{
	unsigned int nWidth, nHeight;
	m_oRenderer.GetResolution(nWidth, nHeight);
	CMatrix p;
	CVector ray, camPos;
	m_oRenderer.GetProjectionMatrix(p);
	m_oGeometryManager.RayCast(x, y, m_oWorldMatrix, p, nWidth, nHeight, camPos, ray);

	static float rayLength = 250.f;
	CVector farPoint = camPos - ray * rayLength;
	farPoint.m_w = 1.f;

	INode* pSelectedEntity = NULL;
	vector<INode*> entities;
	CollectSelectableEntity(entities);
	for (int i = 0; i < entities.size(); i++) {
		INode* pEntity = entities[i];
		CVector pos;
		pEntity->GetWorldPosition(pos);
		if (m_oGeometryManager.IsIntersect(camPos, farPoint, pos, pEntity->GetBoundingSphereRadius())) {
			float lastDistanceToCam = 999999999.f;
			CVector lastPos, currentPos;
			if (pSelectedEntity) {
				pSelectedEntity->GetWorldPosition(lastPos);
				lastDistanceToCam = (lastPos - camPos).Norm();
			}
			pEntity->GetWorldPosition(currentPos);
			float newDistanceToCam = (currentPos - camPos).Norm();
			if (newDistanceToCam < lastDistanceToCam) {
				pSelectedEntity = pEntity;
			}
		}
	}
	return pSelectedEntity;
}