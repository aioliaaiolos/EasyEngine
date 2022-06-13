#include "EEPlugin.h"
#include "IEventDispatcher.h"

class IGUIManager;

class IHud : public CPlugin
{
public:

	IHud() :
	CPlugin(nullptr, ""){}
	virtual int Print(string text, int x, int y) = 0;
	virtual void RemoveText(int index) = 0;
	virtual void Clear() = 0;
	virtual int GetLineCount() = 0;
	virtual int GetSlotCount() = 0;
	virtual int CreateNewSlot(int x, int y) = 0;
	virtual void AddToSlot(int slotId, string text) = 0;
	virtual void PrintInSlot(int nSlotPosition, int nLine, string text) = 0;
};