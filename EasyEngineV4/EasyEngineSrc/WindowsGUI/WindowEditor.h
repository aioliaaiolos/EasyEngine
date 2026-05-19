#pragma once
#include "Window2.h"


class ITopicSystem;
class IEntityManager;
class ITopic;

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

	static ITopicSystem* s_pTopicSystem;
	static IEntityManager* s_pEntityManager;
	static EEInterface* s_pInterface;
	static HWND s_hTitles;
	static HWND s_hTopics;
	static HWND s_hEditTopic;
	static HWND s_hTabCategory;
	static HWND s_hActions;
	static int s_nCurrentTab;
	static string s_sSelectedTitle;
	
};