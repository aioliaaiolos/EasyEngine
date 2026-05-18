#include "WindowEditor.h"
#include "Resource.h"
#include "ITopicSystem.h"
#include "IEntity.h"
#include "IScriptManager.h"

ITopicSystem* CWindowEditor::s_pTopicSystem = NULL;
string CWindowEditor::s_LastSelectedTitle;

CWindowEditor::CWindowEditor(const CWindowEditor::EditorDesc& desc) : 
	CWindow2(desc)
{
	m_pfnWindowCallback = [desc](IWidget* pWidget, UINT msg, WPARAM wParam, LPARAM lPAram)
	{
		switch (msg)
		{
		case WM_COMMAND:
			switch (wParam) {
			case ID_OUTILS_TOPICS:
			{
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
	case WM_CONTEXTMENU:
	{
		HWND hTitles = GetDlgItem(hWnd, IDC_TOPICS_TITLE);
		if ((HWND)wParam == hTitles) {
			HMENU hMenu = CreatePopupMenu();
			AppendMenuA(hMenu, MF_STRING, ID_TOPIC_NEW, "New");
			int x = LOWORD(lParam), y = HIWORD(lParam);
			TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, x, y, 0, hWnd, nullptr);
			DestroyMenu(hMenu);
		}
		break;
	}
	case WM_INITDIALOG:
	{
		pInterface = (EEInterface*)lParam;
		s_pTopicSystem = dynamic_cast<ITopicSystem*>(pInterface->GetPlugin("TopicSystem"));
		IEntityManager* pEntityManager = dynamic_cast<IEntityManager*>(pInterface->GetPlugin("EntityManager"));
		if (!s_pTopicSystem)
			s_pTopicSystem = (ITopicSystem*)lParam;
		HWND hTitles = GetDlgItem(hWnd, IDC_TOPICS_TITLE);
		if (hTitles)
		{
			map<string, vector<ITopic*>>& topicsMap = s_pTopicSystem->GetAllTopics();
			for (const pair<string, vector<ITopic*>>& topicPair : topicsMap) {
				SendMessage(hTitles, CB_ADDSTRING, 0, (LPARAM)topicPair.first.c_str());
			}
			// Optionally, select the first item
			SendMessage(hTitles, CB_SETCURSEL, 0, 0);
		}
		vector<string> characherNames;
		pEntityManager->GetCharactersName(characherNames);

		HWND hID = GetDlgItem(hWnd, IDC_COMBO_ID);
		for (string& name : characherNames)
			SendMessage(hID, CB_ADDSTRING, 0, (LPARAM)name.c_str());
		
		int iConditionType = IDC_CONDITION_TYPE01;
		int iConditionComp = IDC_CONDITION_COMP_01;
		
		IScriptManager* pScriptManager = dynamic_cast<IScriptManager*>(pInterface->GetPlugin("ScriptManager")); 
		vector<string> varNames;
		pScriptManager->GetVariableNames(varNames);
		for (int iConditionName = IDC_CONDITION_NAME_01; iConditionName <= IDC_CONDITION_NAME_06; iConditionName++) {
			HWND hConditionType = GetDlgItem(hWnd, iConditionType++);
			SendMessage(hConditionType, CB_ADDSTRING, 0, (LPARAM)"Global");
			SendMessage(hConditionType, CB_ADDSTRING, 0, (LPARAM)"PCItem");
			HWND hConditionComp = GetDlgItem(hWnd, iConditionComp++);
			SendMessage(hConditionComp, CB_ADDSTRING, 0, (LPARAM)"==");
			SendMessage(hConditionComp, CB_ADDSTRING, 0, (LPARAM)"!=");
			SendMessage(hConditionComp, CB_ADDSTRING, 0, (LPARAM)"<=");
			SendMessage(hConditionComp, CB_ADDSTRING, 0, (LPARAM)"<");
			SendMessage(hConditionComp, CB_ADDSTRING, 0, (LPARAM)">=");
			SendMessage(hConditionComp, CB_ADDSTRING, 0, (LPARAM)">");
			SendMessage(hConditionComp, CB_ADDSTRING, 0, (LPARAM)"is");
			SendMessage(hConditionComp, CB_ADDSTRING, 0, (LPARAM)"isNot");
		}
		return TRUE;
		break;
	}
	case WM_COMMAND:
		if (wParam == ID_TOPIC_NEW) {
			string newTopicPrefix = "New_Topic";
			string newTopicName = newTopicPrefix;
			int newNameIndex = 0;
			map<string, vector<ITopic*>>::iterator itTopic = s_pTopicSystem->GetAllTopics().find(newTopicPrefix);
			while (itTopic != s_pTopicSystem->GetAllTopics().end()) {
				newTopicName = newTopicPrefix + "_" + std::to_string(newNameIndex);
				itTopic = s_pTopicSystem->GetAllTopics().find(newTopicName);
				newNameIndex++;
			}
			HWND hTitles = GetDlgItem(hWnd, IDC_TOPICS_TITLE);
			SendMessage(hTitles, CB_ADDSTRING, 0, (LPARAM)newTopicName.c_str());
			vector<ICondition*> conditions;
			vector<string> actions;
			s_pTopicSystem->AddTopic(newTopicName, "", conditions, actions);
			return TRUE;
		}
		if (LOWORD(wParam) == ID_OK) {
			s_pTopicSystem->SaveTopics("Topics.json", s_pTopicSystem->GetAllTopics(), s_pTopicSystem->GetAllGreatings());
			EndDialog(hWnd, IDCANCEL);
			return TRUE;
		}
		else if (LOWORD(wParam) == ID_CANCEL) {
			EndDialog(hWnd, IDCANCEL);
			return TRUE;
		}
		else if (LOWORD(wParam) == IDC_TOPICS_TITLE && HIWORD(wParam) == CBN_SELCHANGE) {
			map<string, vector<ITopic*>>& topicsMap = s_pTopicSystem->GetAllTopics();
			string sTitle;
			GetTitleSelection(hWnd, s_pTopicSystem, sTitle);
			if (sTitle == s_LastSelectedTitle) {
				// rename
			}
			else {
				s_LastSelectedTitle = sTitle;
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
		}
		else if (LOWORD(wParam) == IDC_TOPICS && HIWORD(wParam) == LBN_SELCHANGE) {
			HWND hText = GetDlgItem(hWnd, IDC_TOPICS);
			HWND hEdit = GetDlgItem(hWnd, IDC_EDIT_TOPIC);
			int selectionText = SendMessage(hText, LB_GETCURSEL, 0, (LPARAM)0);
			int len = SendMessage(hText, LB_GETTEXTLEN, selectionText, (LPARAM)0);
			string buf(len + 1, '\0');
			SendMessage(hText, LB_GETTEXT, selectionText, (LPARAM)buf.data());
			string item(buf.data());
			SetWindowTextA(hEdit, item.data());

			string sTitle;
			GetTitleSelection(hWnd, s_pTopicSystem, sTitle);
			map<string, vector<ITopic*>>& topicsMap = s_pTopicSystem->GetAllTopics();
			ITopic* pSelectedTopic = nullptr;
			for (ITopic* pTopic : topicsMap[sTitle]) {
				if (pTopic->GetText() == item) {
					pSelectedTopic = pTopic;
					break;
				}
			}
			if (pSelectedTopic) {
				HWND hID = GetDlgItem(hWnd, IDC_COMBO_ID);
				SendMessage(hID, CB_SETCURSEL, (WPARAM)-1, 0);				
				for (int i = 0; i < 6; i++) {
					HWND hConditionType = GetDlgItem(hWnd, IDC_CONDITION_TYPE01 + i);
					SendMessage(hConditionType, CB_SETCURSEL, (WPARAM)-1, 0);
					HWND hConditionName = GetDlgItem(hWnd, IDC_CONDITION_NAME_01 + i);
					SendMessage(hConditionName, CB_RESETCONTENT, 0, 0);
					HWND hConditionComp = GetDlgItem(hWnd, IDC_CONDITION_COMP_01 + i);
					SendMessage(hConditionComp, CB_SETCURSEL, (WPARAM)-1, 0);
					HWND hConditionValue = GetDlgItem(hWnd, IDC_CONDITION_VALUE_01 + i);
					SendMessage(hConditionValue, WM_SETTEXT, 0, (LPARAM)"");
				}

				vector<ICondition*> conditions = pSelectedTopic->GetConditions();
				int iConditionType = IDC_CONDITION_TYPE01;
				int iConditionName = IDC_CONDITION_NAME_01;
				int iConditionComp = IDC_CONDITION_COMP_01;
				int iConditionValue = IDC_CONDITION_VALUE_01;
				for (ICondition* pCondition : conditions) {
					if (pCondition->GetType().empty()) {
						if (pCondition->GetName() == "CharacterId") {
							HWND hID = GetDlgItem(hWnd, IDC_COMBO_ID);
							SendMessage(hID, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)pCondition->GetValue().c_str());
						}
					}
					else {
						HWND hConditionType = GetDlgItem(hWnd, iConditionType);
						SendMessage(hConditionType, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)pCondition->GetType().c_str());
						FillFieldsFromType(hWnd, iConditionType++, pInterface);
						HWND hConditionName = GetDlgItem(hWnd, iConditionName++);
						SendMessage(hConditionName, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)pCondition->GetName().c_str());
						HWND hConditionComp = GetDlgItem(hWnd, iConditionComp++);
						SendMessage(hConditionComp, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)pCondition->GetCompStr().c_str());
						HWND hConditionValue = GetDlgItem(hWnd, iConditionValue++);
						SendMessage(hConditionValue, WM_SETTEXT, 0, (LPARAM)pCondition->GetValue().c_str());
					}
				}
			}
		}
		else if (LOWORD(wParam) >= IDC_CONDITION_TYPE01 && LOWORD(wParam) <= IDC_CONDITION_TYPE06) {
			int idcConditionType = LOWORD(wParam);
			int hiParam = HIWORD(wParam);
			if (hiParam == CBN_SELCHANGE) {
				FillFieldsFromType(hWnd, idcConditionType, pInterface);
			}
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


void CWindowEditor::FillFieldsFromType(HWND hWnd, int idcConditionType, EEInterface* pInterface)
{
	int index = idcConditionType - IDC_CONDITION_TYPE01;
	int idConditionName = IDC_CONDITION_NAME_01 + index;
	HWND hConditionName = GetDlgItem(hWnd, idConditionName);
	HWND hConditionType = GetDlgItem(hWnd, idcConditionType);
	SendMessage(hConditionName, CB_RESETCONTENT, 0, 0);
	HWND hConditionValue = GetDlgItem(hWnd, IDC_CONDITION_VALUE_01 + index);
	SendMessage(hConditionValue, WM_SETTEXT, 0, (LPARAM)"");

	
	char buffer[256];
	int selectionIndex = SendMessage(hConditionType, CB_GETCURSEL, 0, 0);
	SendMessage(hConditionType, CB_GETLBTEXT, selectionIndex, (LPARAM)buffer);
	string type = buffer;
	if (type == "PCItem") {
		IEntityManager* pEntityManager = dynamic_cast<IEntityManager*>(pInterface->GetPlugin("EntityManager"));
		for (pair<const string, IItem*>& item : pEntityManager->GetItems())
			SendMessage(hConditionName, CB_ADDSTRING, 0, (LPARAM)item.first.c_str());
	}
	else if (type == "Global")
	{
		IScriptManager* pScriptManager = dynamic_cast<IScriptManager*>(pInterface->GetPlugin("ScriptManager"));
		vector<string> varNames;
		pScriptManager->GetVariableNames(varNames);
		for (string& name : varNames) {
			SendMessage(hConditionName, CB_ADDSTRING, 0, (LPARAM)name.c_str());
		}
		SendMessage(hConditionName, CB_ADDSTRING, 0, (LPARAM)"choice");
	}
}