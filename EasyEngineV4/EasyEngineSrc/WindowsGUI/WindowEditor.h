#pragma once
#include "Window2.h"


class ITopicSystem;
class IEntityManager;
class ITopic;
class ICondition;

class CWindowEditor : public CWindow2
{
public:
	struct EditorDesc : public IWindow::Desc
	{
		//ITopicSystem* m_pTopicSystem = nullptr;
		EEInterface&	m_oInterface;

		EditorDesc(const IWindow::Desc& desc, EEInterface&	oInterface) : IWindow::Desc(desc), m_oInterface(oInterface)
		{
		}
	};

	CWindowEditor(const CWindowEditor::EditorDesc& desc);

	static INT_PTR CALLBACK OnTopicCallback(HWND, UINT, WPARAM, LPARAM);
	static const int CONDITION_COUNT = 6;;

private:
	static void FillConditionFromType(HWND hWnd, int idcConditionType, EEInterface* pInterface);
	static string GetCurrentTitle(HWND hWnd);
	static map<string, vector<ITopic*>>& GetCurrentTopics();
	static void InitTopicsWindow(HWND hWnd, map<string, vector<ITopic*>>& topics);
	static void ClearSpeakerConditions(HWND hWnd);
	static void ClearAll(HWND hWnd);
	static void ClearTopicWindow();
	static void ClearEditTopicWindow();
	static void RefreshTopics(string sTitle);
	static void InsertNewTitle(string sTitleName, string sFirstTopicName = "");
	static void SelectTitle(int index);
	static int GetSeletedTitleIndex();
	static int InsertNewTopic(string sTopicName);
	static ITopic* GetSelectedTopic(HWND hWnd, string& itemName);
	static void GetLBSelection(HWND hListBox, string& selectionText);
	static void GetCBSelection(HWND hListBox, string& selectionText);
	static void GetConditionFromControls(int index, string& type, string& name, string& comp, string& value);
	static void SetCondition(ICondition* pCondition, string& type, string& name, string& comp, string& value);
	static void GetActions(HWND hWnd, vector<string>& actions);

	static ITopicSystem* s_pTopicSystem;
	static IEntityManager* s_pEntityManager;
	static EEInterface* s_pInterface;
	static HWND s_hTitles;
	static HWND s_hTopics;
	static HWND s_hEditTopic;
	static HWND s_hTabCategory;
	static HWND s_hActions;
	static HWND s_hCharacterId;
	static HWND s_hConditionTypes[CONDITION_COUNT];
	static HWND s_hConditionNames[CONDITION_COUNT];
	static HWND s_hConditionComps[CONDITION_COUNT];
	static HWND s_hConditionValues[CONDITION_COUNT];

	static int s_nCurrentTab;
	static string s_sSelectedTitle;
	//static string s_sSelectedTopic;
	
};