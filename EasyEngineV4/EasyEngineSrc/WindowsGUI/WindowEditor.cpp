#include "WindowEditor.h"
#include "Resource.h"
#include "Interface.h"
#include "ITopicSystem.h"
#include "IEntity.h"

CWindowEditor::CWindowEditor(const CWindowEditor::EditorDesc& desc) : CWindow2(desc)
{
	m_pfnWindowCallback = [desc](IWidget* pWidget, UINT msg, WPARAM wParam, LPARAM lPAram)
	{
		switch (msg)
		{
		case WM_COMMAND:
			switch (wParam) {
			case ID_OUTILS_TOPICS:
			{
				//ITopicSystem* pTopicSystem = dynamic_cast<ITopicSystem*>(desc.m_oInterface.GetPlugin("TopicSystem"));
				DialogBoxParamA(GetModuleHandleA("WindowsGUI.dll"), MAKEINTRESOURCE(IDD_TOPICS), pWidget->GetHandle(), OnTopicCallback, (LPARAM)&desc.m_oInterface);
			}
				break;
			
			}
			break;
		default:
			break;
		}
		return 0;
	};
}

void GetTitleSelection(HWND hDialog, ITopicSystem* pTopicSystem, string& item)
{
	HWND hTitles = GetDlgItem(hDialog, IDC_TOPICS_TITLE);
	int selection = SendMessage(hTitles, CB_GETCURSEL, 0, (LPARAM)0);
	map<string, vector<ITopic*>>& topicsMap = pTopicSystem->GetAllTopics();
	int len = (int)SendMessage(hTitles, CB_GETLBTEXTLEN, selection, 0);
	vector<char> buf(len + 1);
	SendMessage(hTitles, CB_GETLBTEXT, selection, (LPARAM)buf.data());
	item = buf.data();
}

INT_PTR CALLBACK CWindowEditor::OnTopicCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int controlId = 0;
	static EEInterface* pInterface = nullptr;
	
	switch (msg) {
	case WM_INITDIALOG:
	{
		pInterface = (EEInterface*)lParam;
		ITopicSystem* pTopicSystem = dynamic_cast<ITopicSystem*>(pInterface->GetPlugin("TopicSystem"));
		IEntityManager* pEntityManager = dynamic_cast<IEntityManager*>(pInterface->GetPlugin("EntityManager"));
		if (!pTopicSystem)
			pTopicSystem = (ITopicSystem*)lParam;
		HWND hTitles = GetDlgItem(hWnd, IDC_TOPICS_TITLE);
		if (hTitles)
		{
			map<string, vector<ITopic*>>& topicsMap = pTopicSystem->GetAllTopics();
			for (const pair<string, vector<ITopic*>>& topicPair : topicsMap) {
				SendMessage(hTitles, CB_ADDSTRING, 0, (LPARAM)topicPair.first.c_str());
			}
			// Optionally, select the first item
			//SendMessage(hTitles, CB_SETCURSEL, 0, 0);
		}
		vector<string> characherNames;
		pEntityManager->GetCharactersName(characherNames);

		HWND hID = GetDlgItem(hWnd, IDC_COMBO_ID);
		for(string& name : characherNames)
			SendMessage(hID, CB_ADDSTRING, 0, (LPARAM)name.c_str());

		return TRUE;
		break;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_TOPICS_TITLE && HIWORD(wParam) == CBN_SELCHANGE) {
			ITopicSystem* pTopicSystem = dynamic_cast<ITopicSystem*>(pInterface->GetPlugin("TopicSystem"));
			HWND hTitles = GetDlgItem(hWnd, IDC_TOPICS_TITLE);
			map<string, vector<ITopic*>>& topicsMap = pTopicSystem->GetAllTopics();
			string sTitle;
			GetTitleSelection(hWnd, pTopicSystem, sTitle);
			map<string, vector<ITopic*>>::iterator itTopic = topicsMap.find(sTitle);
			if (itTopic != topicsMap.end()) {
				vector<ITopic*>& topics = topicsMap[sTitle];
				HWND hText = GetDlgItem(hWnd, IDC_TOPICS);
				SendMessage(hText, LB_RESETCONTENT, 0, (LPARAM)0);
				for (ITopic* topic : topics) {
					SendMessage(hText, LB_ADDSTRING, 0, (LPARAM)topic->GetText().c_str());
				}
			}
		}
		else if (LOWORD(wParam) == IDC_TOPICS && HIWORD(wParam) == LBN_SELCHANGE) {
			ITopicSystem* pTopicSystem = dynamic_cast<ITopicSystem*>(pInterface->GetPlugin("TopicSystem"));
			HWND hText = GetDlgItem(hWnd, IDC_TOPICS);
			HWND hEdit = GetDlgItem(hWnd, IDC_EDIT_TOPIC);
			HWND hTitles = GetDlgItem(hWnd, IDC_TOPICS_TITLE);
			//int selectionTitle = SendMessage(hTitles, CB_GETCURSEL, 0, (LPARAM)0);
			int selectionText = SendMessage(hText, LB_GETCURSEL, 0, (LPARAM)0);
			int len = SendMessage(hText, LB_GETTEXTLEN, selectionText, (LPARAM)0);
			string buf(len + 1, '\0');
			SendMessage(hText, LB_GETTEXT, selectionText, (LPARAM)buf.data());
			string item(buf.data());
			SetWindowTextA(hEdit, item.data());

			string sTitle;
			GetTitleSelection(hWnd, pTopicSystem, sTitle);
			map<string, vector<ITopic*>>& topicsMap = pTopicSystem->GetAllTopics();
			ITopic* pTopic = topicsMap[sTitle][selectionText];
			pTopic->GetConditions();
		}
		break;
	case WM_SYSCOMMAND:
		if ((wParam & 0xFFF0) == SC_CLOSE) {
			EndDialog(hWnd, IDCANCEL);
			return TRUE;
		}
		break;
	}
	return 0;
}