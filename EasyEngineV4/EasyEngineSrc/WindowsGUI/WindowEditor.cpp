#include "WindowEditor.h"
#include "Resource.h"
#include "ITopicSystem.h"
#include "IEntity.h"
#include "IScriptManager.h"
#include <commctrl.h>

const int ACTIONS_BUFFER_SIZE = 8192;
const int EDIT_TOPIC_BUFFER_SIZE = 8192;

ITopicSystem* CWindowEditor::s_pTopicSystem = NULL;
IEntityManager* CWindowEditor::s_pEntityManager = NULL;
EEInterface* CWindowEditor::s_pInterface = NULL;
int CWindowEditor::s_nCurrentTab = 0;
string CWindowEditor::s_sSelectedTitle;

HWND CWindowEditor::s_hTitles;
HWND CWindowEditor::s_hTopics;
HWND CWindowEditor::s_hEditTopic;
HWND CWindowEditor::s_hTabCategory;
HWND CWindowEditor::s_hActions;
HWND CWindowEditor::s_hCharacterId;

HWND CWindowEditor::s_hConditionTypes[CWindowEditor::CONDITION_COUNT];
HWND CWindowEditor::s_hConditionNames[CWindowEditor::CONDITION_COUNT];
HWND CWindowEditor::s_hConditionComps[CWindowEditor::CONDITION_COUNT];
HWND CWindowEditor::s_hConditionValues[CWindowEditor::CONDITION_COUNT];

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

string CWindowEditor::GetCurrentTitle(HWND hWnd)
{
	string item;
	int selection = GetSeletedTitleIndex();
	if (selection != -1) {
		char buf[256];
		ListView_GetItemText(s_hTitles, selection, 0, buf, sizeof(buf));
		item = buf;
	}
	return item;
}

void CWindowEditor::ClearAll(HWND hWnd)
{
	ClearSpeakerConditions(hWnd);
	ListView_DeleteAllItems(s_hTitles);
	ClearTopicWindow();
	ClearEditTopicWindow();
}

void CWindowEditor::ClearEditTopicWindow()
{
	SetWindowTextA(s_hEditTopic, "");
}

int CWindowEditor::GetSeletedTitleIndex()
{
	return ListView_GetNextItem(s_hTitles, -1, LVNI_SELECTED);
}

void CWindowEditor::InsertNewTitle(string sTitleName, string sFirstTopicName)
{
	LVITEM item = { 0 };
	item.mask = LVIF_TEXT;
	item.iItem = ListView_GetItemCount(s_hTitles);
	item.iSubItem = 0;
	item.pszText = (LPSTR)sTitleName.c_str();
	int index = ListView_InsertItem(s_hTitles, &item);

	if (!sFirstTopicName.empty()) {
		SelectTitle(index);
		InsertNewTopic(sFirstTopicName);
	}
}

void CWindowEditor::SelectTitle(int index)
{
	if (index != -1) {
		ListView_SetItemState(s_hTitles, index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		ListView_EnsureVisible(s_hTitles, index, FALSE);
	}
}

int CWindowEditor::InsertNewTopic(string sTopicName)
{
	return SendMessage(s_hTopics, LB_ADDSTRING, 0, (LPARAM)sTopicName.c_str());
}

void CWindowEditor::InitTopicsWindow(HWND hWnd, map<string, vector<ITopic*>>& topics)
{
	ClearAll(hWnd);
	LVCOLUMN col = { 0 };
	col.mask = LVCF_WIDTH;
	col.cx = 200;                       // largeur en pixels ; >= largeur visible suffit
	ListView_InsertColumn(s_hTitles, 0, &col);
	
	for (const pair<string, vector<ITopic*>>& topicPair : topics) {
		InsertNewTitle(topicPair.first);
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

map<string, vector<ITopic*>>& CWindowEditor::GetCurrentTopics()
{
	return s_nCurrentTab == 0 ? s_pTopicSystem->GetAllTopics() : s_pTopicSystem->GetAllGreetings();
}

void CWindowEditor::RefreshTopics(string sTitle)
{
	ClearTopicWindow();
	vector<ITopic*>& topics = GetCurrentTopics()[sTitle];
	for (ITopic* topic : topics) {
		SendMessage(s_hTopics, LB_ADDSTRING, 0, (LPARAM)topic->GetText().c_str());
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
		s_hActions = GetDlgItem(hWnd, IDC_EDIT_ACTIONS);
		s_hCharacterId = GetDlgItem(hWnd, IDC_COMBO_ID);

		for (int i = 0; i < CONDITION_COUNT; i++) {
			s_hConditionTypes[i] = GetDlgItem(hWnd, IDC_CONDITION_TYPE01 + i);
			s_hConditionNames[i] = GetDlgItem(hWnd, IDC_CONDITION_NAME_01 + i);
			s_hConditionComps[i] = GetDlgItem(hWnd, IDC_CONDITION_COMP_01 + i);
			s_hConditionValues[i] = GetDlgItem(hWnd, IDC_CONDITION_VALUE_01 + i);
		}

		TCITEM itemTopic = {0};
		itemTopic.mask = TCIF_TEXT;
		itemTopic.cchTextMax = 16;
		itemTopic.pszText = "Topics";
		TabCtrl_InsertItem(s_hTabCategory, 0, &itemTopic);

		TCITEM itemGreetings = { 0 };
		itemGreetings.mask = TCIF_TEXT;
		itemGreetings.cchTextMax = 16;
		itemGreetings.pszText = "Greetings";
		TabCtrl_InsertItem(s_hTabCategory, 1, &itemGreetings);

		ListView_SetExtendedListViewStyleEx(s_hTitles, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
		s_pInterface = (EEInterface*)lParam;
		s_pTopicSystem = dynamic_cast<ITopicSystem*>(s_pInterface->GetPlugin("TopicSystem"));
		s_pEntityManager = dynamic_cast<IEntityManager*>(s_pInterface->GetPlugin("EntityManager"));
		if (!s_pTopicSystem)
			s_pTopicSystem = (ITopicSystem*)lParam;
		
		if (s_hTitles)
			InitTopicsWindow(hWnd, s_pTopicSystem->GetAllTopics());
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
					s_sSelectedTitle = GetCurrentTitle(hWnd);
					map<string, vector<ITopic*>>::iterator itTopic = GetCurrentTopics().find(s_sSelectedTitle);
					if (itTopic != GetCurrentTopics().end()) {
						RefreshTopics(s_sSelectedTitle);
						ClearEditTopicWindow();
					}
				}
				else if (pnmlv->uChanged & LVIF_TEXT) {
					string sNewTitle = GetCurrentTitle(hWnd);
					map<string, vector<ITopic*>>::iterator itTopic = GetCurrentTopics().find(s_sSelectedTitle);
					GetCurrentTopics()[sNewTitle] = itTopic->second;
					GetCurrentTopics().erase(itTopic);
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
				s_nCurrentTab = TabCtrl_GetCurSel(s_hTabCategory);
				if (s_nCurrentTab == 0) {
					InitTopicsWindow(hWnd, s_pTopicSystem->GetAllTopics());
				}
				else if (s_nCurrentTab == 1) {
					InitTopicsWindow(hWnd, s_pTopicSystem->GetAllGreetings());
				}
			}
		}
		break;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_EDIT_ACTIONS) {
			int hiword = HIWORD(wParam);
			if (HIWORD(wParam) == 1024) {
				char buffer[ACTIONS_BUFFER_SIZE];
				GetWindowText(s_hActions, buffer, ACTIONS_BUFFER_SIZE);
				string topicText;
				ITopic* pTopic = GetSelectedTopic(hWnd, topicText);
				vector<string> actions;
				GetActions(hWnd, actions);
				pTopic->SetActions(actions);
			}
		}
		else if (LOWORD(wParam) == IDC_COMBO_ID) {
			string characterId;
			GetCBSelection(s_hCharacterId, characterId);
			if (!characterId.empty()) {
				string itemName;
				ITopic* pSelectedTopic = GetSelectedTopic(hWnd, itemName);
				ICondition* pCondition = pSelectedTopic->GetCondition(10);
				if (!pCondition)
					pCondition = pSelectedTopic->AddCondition(10);
				pCondition->SetName("CharacterId");
				pCondition->SetComp("==");
				pCondition->SetValue(characterId);
			}
		}
		else if (LOWORD(wParam) == IDC_EDIT_TOPIC) {
			int hiword = HIWORD(wParam);
			if (hiword == 0x400) {
				char buffer[EDIT_TOPIC_BUFFER_SIZE];
				GetWindowTextA(s_hEditTopic, buffer, EDIT_TOPIC_BUFFER_SIZE);
				int topicIndex = SendMessageA(s_hTopics, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				if (topicIndex >= 0) {
					SendMessageA(s_hTopics, LB_DELETESTRING, (WPARAM)topicIndex, (LPARAM)0);
					SendMessageA(s_hTopics, LB_INSERTSTRING, (WPARAM)topicIndex, (LPARAM)buffer);
					SendMessageA(s_hTopics, LB_SETCURSEL, (WPARAM)topicIndex, (LPARAM)0);
					string sTitle = GetCurrentTitle(hWnd);
					map<string, vector<ITopic*>>::iterator itTitle = s_pTopicSystem->GetAllTopics().find(sTitle);
					if (itTitle != s_pTopicSystem->GetAllTopics().end()) {
						if (topicIndex < itTitle->second.size()) {
							string test = buffer;
							ITopic* pTopic = itTitle->second[topicIndex];
							pTopic->SetText(test);
						}
					}
				}
			}
			break;
		}
		else if (wParam == ID_TITLE_NEW) {
			string newTitlePrefix = "New_Title";
			string newTitleName = newTitlePrefix;
			int newNameIndex = 0;
			map<string, vector<ITopic*>>::iterator itTopic = s_pTopicSystem->GetAllTopics().find(newTitlePrefix);
			while (itTopic != s_pTopicSystem->GetAllTopics().end()) {
				newTitleName = newTitlePrefix + "_" + std::to_string(newNameIndex);
				itTopic = s_pTopicSystem->GetAllTopics().find(newTitleName);
				newNameIndex++;
			}
			string sNewTopicName = "New topic";
			InsertNewTitle(newTitleName, sNewTopicName);
			if(s_nCurrentTab == 0)
				s_pTopicSystem->AddTopic(newTitleName, sNewTopicName);
			else if (s_nCurrentTab == 1)
				s_pTopicSystem->AddGreetingTopic(newTitleName, sNewTopicName);
			return TRUE;
		}
		else if (wParam == ID_TITLE_RENAME) {
			int sel = GetSeletedTitleIndex();
			if (sel != -1) {
				SetFocus(s_hTitles);                  // le contrôle doit avoir le focus
				ListView_EditLabel(s_hTitles, sel);   // ouvre le champ d'édition en place
			}
			break;
		}
		else if (wParam == ID_TITLE_DELETE) {
			int sel = GetSeletedTitleIndex();
			if (sel != -1) {
				string sTitleName = GetCurrentTitle(hWnd);
				if (s_nCurrentTab == 0)
					s_pTopicSystem->DeleteTitle(sTitleName);
				else if (s_nCurrentTab == 1)
					s_pTopicSystem->DeleteTitleGreeting(sTitleName);
				ListView_DeleteItem(s_hTitles, sel);
				int count = ListView_GetItemCount(s_hTitles);
				SelectTitle(sel < count ? sel : sel - 1);
			}
			break;
		}
		else if (wParam == ID_TOPIC_NEW) {
			string sTopicName = "New topic";
			int index = SendMessage(s_hTopics, LB_ADDSTRING, 0, (LPARAM)sTopicName.c_str());
			string sCurrentTitle = CWindowEditor::GetCurrentTitle(hWnd);
			if (!sCurrentTitle.empty()) {
				if (s_nCurrentTab == 0)
					s_pTopicSystem->AddTopic(sCurrentTitle, sTopicName);
				else if (s_nCurrentTab == 1)
					s_pTopicSystem->AddGreetingTopic(sCurrentTitle, sTopicName);
			}
		}
		else if (wParam == ID_TOPIC_DELETE) {
			int sel = SendMessage(s_hTopics, LB_GETCURSEL, 0, 0);
			string title = CWindowEditor::GetCurrentTitle(hWnd);
			GetCurrentTopics()[title].erase(GetCurrentTopics()[title].begin() + sel);
			//RefreshTopics(title);
			SendMessage(s_hTopics, LB_DELETESTRING, sel, 0);
			int count = SendMessage(s_hTopics, LB_GETCOUNT, 0, 0);
			sel = min(sel, count - 1);
			SendMessage(s_hTopics, LB_SETCURSEL, sel, 0);
			return TRUE;
		}
		else if (LOWORD(wParam) == ID_OK) {
			s_pTopicSystem->SaveTopics("Topics.json");
			EndDialog(hWnd, IDCANCEL);
			return TRUE;
		}
		else if (LOWORD(wParam) == ID_CANCEL) {
			EndDialog(hWnd, IDCANCEL);
			return TRUE;
		}		
		else if (LOWORD(wParam) == IDC_TOPICS && HIWORD(wParam) == LBN_SELCHANGE) {
			string itemName;
			ITopic* pSelectedTopic = GetSelectedTopic(hWnd, itemName);
			SetWindowTextA(s_hEditTopic, itemName.data());
			if (pSelectedTopic) {
				ClearSpeakerConditions(hWnd);

				map<int, ICondition*> conditions = pSelectedTopic->GetConditions();
				for (pair<const int, ICondition*>& pairCondition : conditions) {
					ICondition* pCondition = pairCondition.second;
					if (pCondition->GetType().empty()) {
						if (pCondition->GetName() == "CharacterId") {
							HWND hID = GetDlgItem(hWnd, IDC_COMBO_ID);
							SendMessage(hID, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)pCondition->GetValue().c_str());
						}
					}
					else {
						HWND hConditionType = GetDlgItem(hWnd, IDC_CONDITION_TYPE01 + pairCondition.first);
						SendMessage(hConditionType, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)pCondition->GetType().c_str());
						FillConditionFromType(hWnd, IDC_CONDITION_TYPE01 + pairCondition.first, s_pInterface);
						HWND hConditionName = GetDlgItem(hWnd, IDC_CONDITION_NAME_01 + pairCondition.first);
						SendMessage(hConditionName, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)pCondition->GetName().c_str());
						HWND hConditionComp = GetDlgItem(hWnd, IDC_CONDITION_COMP_01 + pairCondition.first);
						SendMessage(hConditionComp, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)pCondition->GetCompStr().c_str());
						HWND hConditionValue = GetDlgItem(hWnd, IDC_CONDITION_VALUE_01 + pairCondition.first);
						SendMessage(hConditionValue, WM_SETTEXT, 0, (LPARAM)pCondition->GetValue().c_str());
					}
				}
				string actions;
				for (string s : pSelectedTopic->GetActions()) {
					actions += s + "\r\n";
				}
				SendMessage(s_hActions, WM_SETTEXT, 0, (LPARAM)actions.c_str());
			}
		}
		else if (LOWORD(wParam) >= IDC_CONDITION_TYPE01 && LOWORD(wParam) <= IDC_CONDITION_TYPE06) {
			int idcConditionType = LOWORD(wParam);
			int hiParam = HIWORD(wParam);
			if (hiParam == CBN_SELCHANGE) {
				FillConditionFromType(hWnd, idcConditionType, s_pInterface);
			}
		}
		else if (LOWORD(wParam) >= IDC_CONDITION_NAME_01 && LOWORD(wParam) <= IDC_CONDITION_NAME_06) {
			int hiParam = HIWORD(wParam);
			if (hiParam == CBN_SELCHANGE) {
				string itemName;
				ITopic* pSelectedTopic = GetSelectedTopic(hWnd, itemName);
				int index = LOWORD(wParam) - IDC_CONDITION_NAME_01;
				string type, name, comp, value;
				GetConditionFromControls(index, type, name, comp, value);
				if (!type.empty() && !name.empty() && !comp.empty() && !value.empty()) {
					ICondition* pCondition = pSelectedTopic->GetCondition(index);
					if (!pCondition)
						pCondition = pSelectedTopic->AddCondition(index);
					SetCondition(pCondition, type, name, comp, value);
				}
			}
		}
		else if (LOWORD(wParam) >= IDC_CONDITION_COMP_01 && LOWORD(wParam) <= IDC_CONDITION_COMP_06) {
			int hiParam = HIWORD(wParam);
			if (hiParam == CBN_SELCHANGE) {
				string itemName;
				ITopic* pSelectedTopic = GetSelectedTopic(hWnd, itemName);
				int index = LOWORD(wParam) - IDC_CONDITION_COMP_01;
				string type, name, comp, value;
				GetConditionFromControls(index, type, name, comp, value);
				if (!type.empty() && !name.empty() && !comp.empty() && !value.empty()) {
					ICondition* pCondition = pSelectedTopic->GetCondition(index);
					if (!pCondition)
						pCondition = pSelectedTopic->AddCondition(index);
					SetCondition(pCondition, type, name, comp, value);
				}
			}
		}
		else if (LOWORD(wParam) >= IDC_CONDITION_VALUE_01 && LOWORD(wParam) <= IDC_CONDITION_VALUE_06) {
			string itemName;
			ITopic* pSelectedTopic = GetSelectedTopic(hWnd, itemName);
			if (pSelectedTopic) {
				int hiParam = HIWORD(wParam);
				int index = LOWORD(wParam) - IDC_CONDITION_VALUE_01;
				string type, name, comp, value;
				GetConditionFromControls(index, type, name, comp, value);
				if (!type.empty() && !name.empty() && !comp.empty() && !value.empty()) {
					ICondition* pCondition = pSelectedTopic->GetCondition(index);
					if (!pCondition)
						pCondition = pSelectedTopic->AddCondition(index);
					SetCondition(pCondition, type, name, comp, value);
				}
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

void CWindowEditor::GetActions(HWND hWnd, vector<string>& actions)
{
	char buffer[ACTIONS_BUFFER_SIZE];
	GetWindowText(s_hActions, buffer, ACTIONS_BUFFER_SIZE);
	string actionsStr = buffer;
	int nPos = 0;
	while (nPos != -1) {
		nPos = actionsStr.find("\r\n");
		if (nPos != -1) {
			string s = actionsStr.substr(0, nPos);
			actions.push_back(s);
			actionsStr = actionsStr.substr(nPos + 2);
		}
		else {
			int sepPos = actionsStr.find("\t\n");
			if (sepPos != -1) {
				actionsStr = actionsStr.substr(0, sepPos);
			}
			if (!actionsStr.empty()) {
				actions.push_back(actionsStr);
			}
		}
	}
}

void CWindowEditor::GetConditionFromControls(int index, string& type, string& name, string& comp, string& value)
{
	GetCBSelection(s_hConditionTypes[index], type);
	GetCBSelection(s_hConditionNames[index], name);
	GetCBSelection(s_hConditionComps[index], comp);
	char buffer[8];
	GetWindowTextA(s_hConditionValues[index], buffer, 8);
	value = buffer;
}

void CWindowEditor::SetCondition(ICondition* pCondition, string& type, string& name, string& comp, string& value)
{
	pCondition->SetType(type);
	pCondition->SetName(name);
	pCondition->SetComp(comp);
	pCondition->SetValue(value);
}

void CWindowEditor::GetLBSelection(HWND hListBox, string& selectionText)
{
	int index = SendMessage(hListBox, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	int len = SendMessage(hListBox, LB_GETTEXTLEN, index, (LPARAM)0);
	string buf(len + 1, '\0');
	SendMessage(hListBox, LB_GETTEXT, index, (LPARAM)buf.data());
	selectionText = buf.data();
}

void CWindowEditor::GetCBSelection(HWND hListBox, string& selectionText)
{
	int index = SendMessage(hListBox, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	int len = SendMessage(hListBox, CB_GETCOUNT, index, (LPARAM)0);
	string buf(len + 1, '\0');
	SendMessage(hListBox, CB_GETLBTEXT, index, (LPARAM)buf.data());
	selectionText = buf.data();
}

ITopic* CWindowEditor::GetSelectedTopic(HWND hWnd, string& itemName)
{
	GetLBSelection(s_hTopics, itemName);
	string sTitle = GetCurrentTitle(hWnd);
	map<string, vector<ITopic*>>& topicsMap = GetCurrentTopics();
	ITopic* pSelectedTopic = nullptr;
	for (ITopic* pTopic : topicsMap[sTitle]) {
		if (pTopic->GetText() == itemName) {
			pSelectedTopic = pTopic;
			break;
		}
	}
	return pSelectedTopic;
}

void CWindowEditor::ClearTopicWindow()
{
	SendMessage(s_hTopics, LB_RESETCONTENT, 0, (LPARAM)0);
}

void CWindowEditor::ClearSpeakerConditions(HWND hWnd)
{
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
}

void CWindowEditor::FillConditionFromType(HWND hWnd, int idcConditionType, EEInterface* pInterface)
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