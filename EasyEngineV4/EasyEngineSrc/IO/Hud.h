#include "IHud.h"

using namespace std;

#include <string>

class IGUIManager;

struct CText
{
	string m_sText;
	int m_nPosx;
	int m_nPosy;

	CText(string text, int x, int y) :
		m_sText(text),
		m_nPosx(x),
		m_nPosy(y)
	{}
};

class CHud : public IHud 
{
public:
	CHud(EEInterface& oInterface);
	void Update();
	int Print(string text, int x, int y) override;
	void RemoveText(int index) override;
	string GetName() override;
	void Clear() override;
	int GetLineCount() override;
	int CHud::GetSlotCount() override;
	int CreateNewSlot(int x, int y);
	void AddToSlot(int slotId, string text);
	void PrintInSlot(int nSlotPosition, int nLine, string text);
	static void WindowCallback(CPlugin* plugin, IEventDispatcher::TWindowEvent e, int, int);

private:

	struct Slot
	{
		int x;
		int y;
		vector<string> Text;
	};
	
	IGUIManager&	m_oGUIManager;
	vector<CText>	m_vText;
	map<int, Slot>	m_mSlots;
	int				m_nTextHeight;
	
};

extern "C" _declspec(dllexport) IHud* CreateHud(EEInterface& oInterface);