#include "Interface.h"
#include "EEPlugin.h"
#include "Hud.h"
#include "IGUIManager.h"



CHud::CHud(EEInterface& oInterface) 
	: m_oGUIManager(*static_cast<IGUIManager*>(oInterface.GetPlugin("GUIManager"))),
	m_nTextHeight(15)
{
	IEventDispatcher* pEventDispatcher = (IEventDispatcher*) oInterface.GetPlugin("EventDispatcher");
	pEventDispatcher->AbonneToWindowEvent(this, WindowCallback);
}

void CHud::WindowCallback(CPlugin* plugin, IEventDispatcher::TWindowEvent e, int, int)
{
	if (e == IEventDispatcher::T_WINDOWUPDATE) {
		CHud* pHud = (CHud*)plugin;
		pHud->Update();
	}
}

void CHud::Update()
{
	int idx = 0;
	for (map<int, Slot>::iterator itSlot = m_mSlots.begin(); itSlot != m_mSlots.end(); itSlot++) {
		for (int i = 0; i < itSlot->second.Text.size(); i++) {
			m_oGUIManager.Print(itSlot->second.Text[i], itSlot->second.x, itSlot->second.y + i * m_nTextHeight);
		}
	}
}

int CHud::Print(string text, int x, int y)
{
	m_vText.push_back(CText(text, x, y));
	return m_vText.size() - 1;
}

void CHud::RemoveText(int index)
{
	if (index < m_vText.size()) {
		vector<CText>::iterator it = m_vText.begin() + index;
		m_vText.erase(it);
	}
}

void CHud::Clear()
{
	m_vText.clear();
}

int CHud::GetLineCount()
{
	return m_vText.size();
}

int CHud::GetSlotCount()
{
	return m_mSlots.size();
}


int CHud::CreateNewSlot(int x, int y)
{
	Slot s;
	s.x = x;
	s.y = y;
	int id = m_mSlots.size() > 0 ? m_mSlots.rbegin()->first + 1 : 0;
	m_mSlots[id] = s;
	return id;
}

void CHud::AddToSlot(int slotId, string text)
{
	m_mSlots[slotId].Text.push_back(text);
}

void CHud::PrintInSlot(int slotId, int nLine, string text)
{
	if (m_mSlots[slotId].Text.size() == nLine)
		AddToSlot(slotId, text);
	else
		m_mSlots[slotId].Text[nLine] = text;
}

string CHud::GetName()
{
	return "HUD";
}

extern "C" _declspec(dllexport) IHud* CreateHud(EEInterface& oInterface)
{
	return new CHud(oInterface);
}