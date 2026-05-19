#include "WindowEditor.h"
#include "Resource.h"
#include "ITopicSystem.h"
#include "IEntity.h"
#include "IScriptManager.h"
#include <commctrl.h>

ITopicSystem* CWindowEditor::s_pTopicSystem = NULL;
IEntityManager* CWindowEditor::s_pEntityManager = NULL;
EEInterface* CWindowEditor::s_pInterface = NULL;

HWND CWindowEditor::s_hTitles;
HWND CWindowEditor::s_hTopics;
HWND CWindowEditor::s_hEditTopic;
HWND CWindowEditor::s_hTabCategory;

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

void CWindowEditor::GetTitleSelection(HWND hWnd, string& item)
{
	int selection = ListView_GetNextItem(s_hTitles, -1, LVNI_SELECTED);
	if (selection != -1) {
		char buf[256];
		ListView_GetItemText(s_hTitles, selection, 0, buf, sizeof(buf));
		item = buf;
	}
}

void CWindowEditor::InitTopicsWindow(HWND hWnd)
{
	LVCOLUMN col = { 0 };
	col.mask = LVCF_WIDTH;
	col.cx = 200;                       // largeur en pixels ; >= largeur visible suffit
	ListView_InsertColumn(s_hTitles, 0, &col);

	map<string, vector<ITopic*>>& topicsMap = s_pTopicSystem->GetAllTopics();
	for (const pair<string, vector<ITopic*>>& topicPair : topicsMap) {
		LVITEM item = { 0 };
		item.mask = LVIF_TEXT;
		item.iItem = ListView_GetItemCount(s_hTitles);
		item.iSubItem = 0;
		item.pszText = (LPSTR)topicPair.first.c_str();
		ListView_InsertItem(s_hTitles, &item);
	}

	vector<string> characherNames;
	s_pEntityManager->GetCharactersName(characherNames);

	HWND hID = GetDlgItem(hWnd, IDC_COMBO_ID);
	for (string& name : characherNames)
		SendMessage(hID, CB_ADDSTRING, 0, (LPARAM)name.c_str());

	int iConditionType = IDC_CONDITION_TYPE01;
	int iConditionComp = IDC_CONDITION_COMP_01;

	IScriptManager* pScriptManager = dynamic_cast<IScriptManager*>(s_pInterface->GetPlugin("ScriptManager"));
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
}

INT_PTR CALLBACK CWindowEditor::OnTopicCallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int controlId = 0;
	switch (msg) {
	case WM_CONTEXTMENU:
	{
		if ((HWND)wParam == s_hTitles) {
			HMENU hMenu = CreatePopupMenu();
			AppendMenuA(hMenu, MF_STRING, ID_TITLE_NEW, "New");
			AppendMenuA(hMenu, MF_STRING, ID_TITLE_RENAME, "Rename");
			AppendMenuA(hMenu, MF_STRING, ID_TITLE_DELETE, "Delete");
			int x = LOWORD(lParam), y = HIWORD(lParam);
			TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, x, y, 0, hWnd, nullptr);
			DestroyMenu(hMenu);
		}
		else if ((HWND)wParam == s_hTopics) {
			HMENU hMenu = CreatePopupMenu();
			AppendMenuA(hMenu, MF_STRING, ID_TOPIC_NEW, "New");
			AppendMenuA(hMenu, MF_STRING, ID_TOPIC_DELETE, "Delete");
			int x = LOWORD(lParam), y = HIWORD(lParam);
			TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, x, y, 0, hWnd, nullptr);
			DestroyMenu(hMenu);
		}
		break;
	}
	case WM_INITDIALOG:
	{
		INITCOMMONCONTROLSEX icc = { 0 };
		icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
		icc.dwICC = ICC_LISTVIEW_CLASSES;
		InitCommonControlsEx(&icc);

		s_hTitles = GetDlgItem(hWnd, IDC_TOPICS_TITLE);
		s_hTopics = GetDlgItem(hWnd, IDC_TOPICS);
		s_hEditTopic = GetDlgItem(hWnd, IDC_EDIT_TOPIC);
		s_hTabCategory = GetDlgItem(hWnd, IDC_TAB_CATEGORY);

		TCITEM itemTopic = {0};
		itemTopic.mask = TCIF_TEXT;
		itemTopic.cchTextMax = 16;
		itemTopic.pszText = "Topics";
		TabCtrl_InsertItem(s_hTabCategory, 0, &itemTopic);

		TCITEM itemGreetings = { 0 };
		itemGreetings.mask = TCIF_TEXT;
		itemGreetings.cchTextMax = 16;
		itemGreetings.pszText = "Greetings";
		TabCtrl_InsertItem(s_hTabCategory, 0, &itemGreetings);

		ListView_SetExtendedListViewStyleEx(s_hTitles, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
		s_pInterface = (EEInterface*)lParam;
		s_pTopicSystem = dynamic_cast<ITopicSystem*>(s_pInterface->GetPlugin("TopicSystem"));
		s_pEntityManager = dynamic_cast<IEntityManager*>(s_pInterface->GetPlugin("EntityManager"));
		if (!s_pTopicSystem)
			s_pTopicSystem = (ITopicSystem*)lParam;
		
		if (s_hTitles)
			InitTopicsWindow(hWnd);
		return TRUE;
		break;
	}
	case WM_NOTIFY:
	{
		LPNMHDR pnmh = (LPNMHDR)lParam;
		if (pnmh->idFrom == IDC_TOPICS_TITLE) {
			if (pnmh->code == LVN_ITEMCHANGED) {
				LPNMLISTVIEW pnmlv = (LPNMLISTVIEW)lParam;
				// ne réagir qu'au gain de l'état "sélectionné"
				if ((pnmlv->uChanged & LVIF_STATE) && (pnmlv->uNewState & LVIS_SELECTED) && !(pnmlv->uOldState & LVIS_SELECTED)) {
					int selection = pnmlv->iItem;
					map<string, vector<ITopic*>>& topicsMap = s_pTopicSystem->GetAllTopics();
					string sTitle;
					GetTitleSelection(hWnd, sTitle);
					map<string, vector<ITopic*>>::iterator itTopic = topicsMap.find(sTitle);
					if (itTopic != topicsMap.end()) {
						vector<ITopic*>& topics = topicsMap[sTitle];
						SendMessage(s_hTopics, LB_RESETCONTENT, 0, (LPARAM)0);
						for (ITopic* topic : topics) {
							SendMessage(s_hTopics, LB_ADDSTRING, 0, (LPARAM)topic->GetText().c_str());
						}
					}
				}
			}
			else if (pnmh->code == LVN_ENDLABELEDIT) {
				NMLVDISPINFO* pdi = (NMLVDISPINFO*)lParam;
				if (pdi->item.pszText == nullptr) {
					// l'utilisateur a annulé (Échap) : ne rien faire, refuser
					SetWindowLongPtr(hWnd, DWLP_MSGRESULT, FALSE);
					return TRUE;
				}
				// pdi->item.pszText = nouveau nom ; pdi->item.iItem = index de la ligne
				// ... ici tu mets à jour ta topicsMap avec le nouveau nom ...
				SetWindowLongPtr(hWnd, DWLP_MSGRESULT, TRUE);  // TRUE = accepter le nom
				return TRUE;
			}
			if (pnmh->code == NM_CUSTOMDRAW) {
				LPNMLVCUSTOMDRAW pcd = (LPNMLVCUSTOMDRAW)lParam;
				switch (pcd->nmcd.dwDrawStage) {
					case CDDS_PREPAINT:
						// on demande à être rappelé pour chaque élément
						SetWindowLongPtr(hWnd, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW);
						return TRUE;

					case CDDS_ITEMPREPAINT:
					{
						int iItem = (int)pcd->nmcd.dwItemSpec;
						UINT state = ListView_GetItemState(pnmh->hwndFrom, iItem, LVIS_SELECTED);

						if (state & LVIS_SELECTED) {
							pcd->clrText = RGB(255, 255, 255);   // texte blanc
							pcd->clrTextBk = RGB(0, 120, 215);     // fond bleu (bleu standard Windows)
						}
						SetWindowLongPtr(hWnd, DWLP_MSGRESULT, CDRF_DODEFAULT);
						return TRUE;
					}
				}
			}
		}
		else if (pnmh->idFrom == IDC_TAB_CATEGORY) {
			if (pnmh->code == TCN_SELCHANGE) {
				int sel = TabCtrl_GetCurSel(s_hTabCategory);
				sel = sel;
				if (sel) {

				}
			}
		}
		
		break;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_EDIT_TOPIC) {
			int hiword = HIWORD(wParam);
			hiword = hiword;
			if (hiword == 0x400) {
				char buffer[2048];
				GetWindowTextA(s_hEditTopic, buffer, 2048);
				int topicIndex = SendMessageA(s_hTopics, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				if (topicIndex >= 0) {
					SendMessageA(s_hTopics, LB_DELETESTRING, (WPARAM)topicIndex, (LPARAM)0);
					SendMessageA(s_hTopics, LB_INSERTSTRING, (WPARAM)topicIndex, (LPARAM)buffer);
					SendMessageA(s_hTopics, LB_SETCURSEL, (WPARAM)topicIndex, (LPARAM)0);
					int titleIndex = SendMessageA(s_hTitles, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
					string sTitle;
					GetTitleSelection(hWnd, sTitle);
					map<string, vector<ITopic*>>::iterator itTitle = s_pTopicSystem->GetAllTopics().find(sTitle);
					if (itTitle != s_pTopicSystem->GetAllTopics().end()) {
						if (itTitle->second.size() > titleIndex) {
							string test = buffer;
							ITopic* pTopic = itTitle->second[titleIndex];
							pTopic->SetText(test);
						}
					}
				}
			}
			break;
		}
		if (wParam == ID_TITLE_NEW) {
			string newTopicPrefix = "New_Topic";
			string newTopicName = newTopicPrefix;
			int newNameIndex = 0;
			map<string, vector<ITopic*>>::iterator itTopic = s_pTopicSystem->GetAllTopics().find(newTopicPrefix);
			while (itTopic != s_pTopicSystem->GetAllTopics().end()) {
				newTopicName = newTopicPrefix + "_" + std::to_string(newNameIndex);
				itTopic = s_pTopicSystem->GetAllTopics().find(newTopicName);
				newNameIndex++;
			}
			SendMessage(s_hTitles, CB_ADDSTRING, 0, (LPARAM)newTopicName.c_str());
			vector<ICondition*> conditions;
			vector<string> actions;
			s_pTopicSystem->AddTopic(newTopicName, "", conditions, actions);
			return TRUE;
		}
		if (wParam == ID_TITLE_RENAME) {
			int sel = ListView_GetNextItem(s_hTitles, -1, LVNI_SELECTED);
			if (sel != -1) {
				SetFocus(s_hTitles);                  // le contrôle doit avoir le focus
				ListView_EditLabel(s_hTitles, sel);   // ouvre le champ d'édition en place
			}
			break;
		}
		if (wParam == ID_TOPIC_NEW) {
			SendMessage(s_hTopics, LB_ADDSTRING, 0, (LPARAM)"New topic");
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
		else if (LOWORD(wParam) == IDC_TOPICS && HIWORD(wParam) == LBN_SELCHANGE) {
			int selectionText = SendMessage(s_hTopics, LB_GETCURSEL, 0, (LPARAM)0);
			int len = SendMessage(s_hTopics, LB_GETTEXTLEN, selectionText, (LPARAM)0);
			string buf(len + 1, '\0');
			SendMessage(s_hTopics, LB_GETTEXT, selectionText, (LPARAM)buf.data());
			string item(buf.data());
			SetWindowTextA(s_hEditTopic, item.data());

			string sTitle;
			GetTitleSelection(hWnd, sTitle);
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
						FillFieldsFromType(hWnd, iConditionType++, s_pInterface);
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
				FillFieldsFromType(hWnd, idcConditionType, s_pInterface);
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