#include "Interface.h"
#include "IFileSystem.h"
#include "IScriptManager.h"
#include "TopicsWindow.h"
#include "Utils2/rectangle.h"
#include "Utils2/StringUtils.h"
#include "GUIManager.h"
#include "IRessource.h"
#include <sstream>



using namespace rapidjson;


CTopicInfoWidgets::CTopicInfoWidgets(int nWidth, int nHeight) :
	CGUIWidget(nWidth, nHeight)
{

}

CTopicsWindow::CTopicsWindow(EEInterface& oInterface, int width, int height) :
	CGUIWindow("Gui/TopicsWindow.bmp", oInterface, CDimension(width, height)),
	m_oInterface(oInterface),
	m_nMaxCharPerLine(95),
	m_nTopicTextPointer(0),
	m_oFileSystem(static_cast<IFileSystem&>(*oInterface.GetPlugin("FileSystem"))),
	m_oRenderer(static_cast<IRenderer&>(*oInterface.GetPlugin("Renderer"))),
	m_oRessourceManager(static_cast<IRessourceManager&>(*oInterface.GetPlugin("RessourceManager"))),
	m_pScriptManager(nullptr)
{
	SetPosition(400, 200);
	LoadTopics("topics.json");
	m_pTopicFrame = new CTopicFrame(oInterface, 198, 759, m_mTopics);
	m_pTopicTextFrame = new CGUIWindow("Gui/TopicsTextWindow.bmp", oInterface, 660, 780);
	AddWidget(m_pTopicFrame);
	AddWidget(m_pTopicTextFrame);
	m_pTopicTextFrame->SetRelativePosition(10, 10);
	SetGUIMode(true);
	m_oInterface.HandlePluginCreation("GUIManager", OnGUIManagerCreated, this);
	m_oInterface.HandlePluginCreation("ScriptManager", OnScriptManagerCreated, this);
}

void CTopicsWindow::OnScriptManagerCreated(CPlugin* plugin, IBaseObject* pData)
{
	CTopicsWindow* pTopicWindow = dynamic_cast<CTopicsWindow*>(pData);
	pTopicWindow->m_pScriptManager = static_cast<IScriptManager*>(plugin);
}

IScriptManager* CTopicsWindow::GetScriptManager()
{
	return m_pScriptManager;
}

void CTopicsWindow::OnGUIManagerCreated(CPlugin* pGUIManager, IBaseObject* pData)
{
	CTopicsWindow* pTopicWindow = dynamic_cast<CTopicsWindow*>(pData);
	pTopicWindow->m_pGUIManager = static_cast<CGUIManager*>(pGUIManager);
}

void CTopicsWindow::OnShow(bool bShow)
{
	CGUIWindow::OnShow(bShow);
	if (!bShow) {
		m_sText.clear();
		m_pTopicFrame->DestroyTopicsWidgets();
		m_nTopicTextPointer = 0;
	}
	else {
		m_pTopicFrame->UpdateTopics();

		int index = SelectTopic(m_vGreatings, m_sSpeakerId);
		if(index != -1)
			AddTopicText(m_vGreatings[index].m_sText);
	}
}

void CTopicsWindow::AddTopic(string sTopicName, string sText, const vector<CCondition>& conditions, const vector<string>& vAction)
{
	CTopicInfo topic;
	topic.m_sText = sText;
	topic.m_vConditions = conditions;
	topic.m_vAction = vAction;
	m_mTopics[sTopicName].push_back(topic);
}

void CTopicsWindow::AddGreating(string sText, vector<CCondition>& conditions)
{
	CTopicInfo greating;
	greating.m_sText = sText;
	greating.m_vConditions = conditions;
	m_vGreatings.push_back(greating);
}

CTopicsWindow::~CTopicsWindow()
{
}

void CTopicsWindow::Display()
{
	CGUIWindow::Display();
}

void ExtractLink(string sText, vector<pair<int, string>>& vText)
{
	string sRemainingString = sText;
	while (!sRemainingString.empty()) {
		string sOpenTag = "<link>";
		string sCloseTag = "</link>";
		int openTagIndex = sRemainingString.find(sOpenTag);
		if (openTagIndex != -1) {
			string subString = sRemainingString.substr(0, openTagIndex);
			int closeTagIndex = sRemainingString.find(sCloseTag, openTagIndex + sOpenTag.size());
			string link = sRemainingString.substr(openTagIndex + sOpenTag.size(), closeTagIndex - openTagIndex - sCloseTag.size() + 1);
			if(!subString.empty())
				vText.push_back(pair<int, string>(0, subString));
			vText.push_back(pair<int, string>(-1, link));
			sRemainingString = sRemainingString.substr(closeTagIndex + sCloseTag.size());
		}
		else {
			string sOpenTag = "<choice=";
			string sCloseTag = "</choice>";
			int openTagIndex = sRemainingString.find(sOpenTag);
			if (openTagIndex != -1) {
				int openTagIndexEnd = sRemainingString.find('>');
				string sChoiceNumber = sRemainingString.substr(openTagIndex + sOpenTag.size(), openTagIndexEnd - openTagIndex - sOpenTag.size());
				int nChoiceNumer = atoi(sChoiceNumber.c_str());
				int closeTagIndex = sRemainingString.find(sCloseTag, openTagIndex + 1);
				string text = sRemainingString.substr(0, openTagIndex);
				string choiceText = sRemainingString.substr(openTagIndexEnd + 1, closeTagIndex - openTagIndexEnd - 1);
				vText.push_back(pair<int, string>(0, text));
				vText.push_back(pair<int, string>(nChoiceNumer, choiceText));
				sRemainingString = sRemainingString.substr(closeTagIndex + sCloseTag.size());
			}
			else {
				vText.push_back(pair<bool, string>(false, sRemainingString));
				sRemainingString.clear();
			}
		}
	}
}

void CTopicsWindow::Truncate(string sText, int nMaxCharPerLine, vector<string>& output)
{
	string sOpenLink = "<link>";
	string sCloseLink = "</link>";
	string sOpenChoice = "<choice=";
	string sCloseChoice = "</choice>";	

	string sSubText;
	int nFinalTextSize = 0;
	while (!sText.empty()) {
		int nOpenChoiceIndex = sText.find(sOpenChoice);
		if (nOpenChoiceIndex != -1) {
			int nCloseChoiceIndex = sText.find(sCloseChoice);
			string subText = sText.substr(nOpenChoiceIndex, nCloseChoiceIndex + sCloseChoice.size());
			output.push_back(subText);
			sText = sText.substr(nCloseChoiceIndex + sCloseChoice.size());
		}
		else {

			bool isLineContainsLink = false;
			int nOpenLinkIndex = sText.find(sOpenLink);
			int nCloseLinkIndex = sText.find(sCloseLink);
			int nLinkSize = nCloseLinkIndex - nOpenLinkIndex - sOpenLink.size();
			if (nOpenLinkIndex > -1 && (nFinalTextSize + nOpenLinkIndex + nLinkSize < nMaxCharPerLine)) {
				if (nFinalTextSize + nOpenLinkIndex + nLinkSize < nMaxCharPerLine) {
					string sLinkTest = sText.substr(nOpenLinkIndex + sOpenLink.size(), nLinkSize);
					sSubText += sText.substr(0, nCloseLinkIndex + sCloseLink.size());
					sText = sText.substr(nCloseLinkIndex + sCloseLink.size());
					isLineContainsLink = true;
					nFinalTextSize += nOpenLinkIndex + nLinkSize;
				}
				else {
					sSubText += sText.substr(0, nOpenLinkIndex);
					output.push_back(sSubText);
					sSubText = "";
					sText = sText.substr(nOpenLinkIndex);
					nFinalTextSize += nOpenLinkIndex;
				}
			}
			else {
				if (nFinalTextSize + sText.size() > nMaxCharPerLine) {
					int nSpaceIndex = min(nMaxCharPerLine - nFinalTextSize, sText.size());
					while (nSpaceIndex > 0 && sText[nSpaceIndex--] != ' ');
					sSubText += sText.substr(0, nSpaceIndex + 1);
					sText = sText.substr(nSpaceIndex + 2);
					nFinalTextSize = 0;
				}
				else {
					sSubText += sText;
					sText = "";
				}
				output.push_back(sSubText);
				sSubText = "";
			}
		}
	}
}

int CTopicsWindow::GetTopicTextLineCount()
{
	int count = 0;
	for (CGUIWidget* pTopic : m_pTopicTextFrame->GetChildren()) {
		CGUIWindow* pTopicWindow = dynamic_cast<CGUIWindow*>(pTopic);
		if (pTopicWindow) {
			count += pTopicWindow->GetWidgetCount();
		}
		else {
			count++;
		}
	}
	return count;
}

void CTopicsWindow::OnChoiceCalled(string sChoices)
{
	map<int, string> choices;
	int lBracketIndex = 0, rBracketIndex = 0;
	while (true) {
		lBracketIndex = sChoices.find('{', rBracketIndex);
		if (lBracketIndex != -1) {
			if (sChoices.size() > lBracketIndex + 1) {
				rBracketIndex = sChoices.find('}', lBracketIndex + 1);
				if (rBracketIndex != -1) {
					string sChoiceNumber = sChoices.substr(lBracketIndex + 1, rBracketIndex - lBracketIndex - 1);
					int nChoiceNumber = atoi(sChoiceNumber.c_str());
					int nNextChoiceIndex = sChoices.find('{', rBracketIndex + 1);
					if (nNextChoiceIndex == -1)
						nNextChoiceIndex = sChoices.size();
					string sText = sChoices.substr(rBracketIndex + 1, nNextChoiceIndex - rBracketIndex - 1);
					choices[nChoiceNumber] = sText;
					if (nNextChoiceIndex == -1)
						break;
				}
			}
		}
		else
			break;
	}

	for (const pair<int, string>& choice : choices)
		AddTopicText(string("<choice=") + std::to_string(choice.first) + ">" + choice.second + "</choice>");
}


void CTopicsWindow::DecomposeTopicText(string sTopicText, vector<vector<pair<int, string>>>& vRearrangedSubString)
{
	vRearrangedSubString.resize(vRearrangedSubString.size() + 1);
	vector<pair<int, string>> vSubString;
	ExtractLink(sTopicText, vSubString);
	vector<string> vTruncatedSubstring;
	for (pair<int, string>& oText : vSubString) {
		if (oText.first == 0) {
			vector<string> vLines;
			CStringUtils::Truncate(oText.second, m_nMaxCharPerLine, vLines);
			for (string s : vLines) {
				vRearrangedSubString.back().push_back(pair<int, string>(0, s));
			}
		}
		else {
			vRearrangedSubString.back().push_back(pair<int, string>(oText.first, oText.second));
		}
	}
}

void CTopicsWindow::ConvertTextArrayToWidgetArray(const vector<vector<pair<int, string>>>& vRearrangedSubString, vector<vector<pair<int, CGUIWidget*>>>& vTopicWidgets)
{
	for (const vector<pair<int, string>>& topicLine : vRearrangedSubString) {
		vTopicWidgets.resize(vTopicWidgets.size() + 1);
		for (const pair<int, string>& elem : topicLine) {
			//vTopicWidgets.push_back(vector<pair<int, CGUIWidget*>>());
			CGUIWidget* pTextWidget = nullptr;
			if (elem.first == -1) {
				pTextWidget = new CLink(m_oInterface, elem.second);
				CLink* pLink = static_cast<CLink*>(pTextWidget);
				pLink->SetClickedCallback(OnLinkClicked);
			}
			else if (elem.first == 0)
				pTextWidget = m_pGUIManager->CreateStaticText(elem.second);
			else if (elem.first > 0) {
				pTextWidget = new CTopicLink(m_oInterface, elem.second, elem.first);
				CTopicLink* pLink = static_cast<CTopicLink*>(pTextWidget);
				pLink->SetClickedCallback(OnChoiceClicked);
			}
			pTextWidget->m_sUserData = elem.second;
			vTopicWidgets.back().push_back(pair<int, CGUIWidget*>(elem.first, pTextWidget));
		}
	}
}

float CTopicsWindow::PutWidgetArrayIntoTopicWidget(vector<vector<pair<int, CGUIWidget*>>>& vTopicWidgets, CGUIWindow* pTopicWidget)
{
#if 0
	m_pTopicTextFrame->AddWidget(pTopicWidget);
	int y = 0;
	pTopicWidget->SetRelativePosition(0, m_nTopicTextPointer * m_pGUIManager->GetCurrentFontEspacementY());

	int nElemIndex = 0;
	int nLastElemType = vTopicWidgets[0].first;
	while (nElemIndex < vTopicWidgets.size()) {
		int x = 0;
		CGUIWindow* pLineWidget = new CGUIWindow;
		while (nElemIndex < vTopicWidgets.size() && (x + vTopicWidgets[nElemIndex].second->GetDimension().GetWidth() < 700)) {
			pair<int, CGUIWidget*>& lineWidgets = vTopicWidgets[nElemIndex];
			pLineWidget->AddWidget(lineWidgets.second);
			lineWidgets.second->SetRelativePosition(x, y);
			int nAdditionalWidth = nLastElemType != lineWidgets.first ? m_pGUIManager->GetCurrentFontWidth(' ') : 0;
			x += lineWidgets.second->GetDimension().GetWidth() + nAdditionalWidth;
			nLastElemType = lineWidgets.first;
			nElemIndex++;
		}
		pTopicWidget->AddWidget(pLineWidget);
		m_nTopicTextPointer++;
		y += m_pGUIManager->GetCurrentFontEspacementY();
	}
	return y;
#else
	m_pTopicTextFrame->AddWidget(pTopicWidget);
	int y = 0;
	pTopicWidget->SetRelativePosition(0, m_nTopicTextPointer * m_pGUIManager->GetCurrentFontEspacementY());
	for (vector<pair<int, CGUIWidget*>>& line : vTopicWidgets) {
		int x = 0;
		CGUIWindow* pLineWidget = new CGUIWindow;
		for (pair<int, CGUIWidget*>& elem : line) {
			pLineWidget->AddWidget(elem.second);
			elem.second->SetRelativePosition(x, 0);
			//int nAdditionalWidth = nLastElemType != lineWidgets.first ? m_pGUIManager->GetCurrentFontWidth(' ') : 0;
			x += elem.second->GetDimension().GetWidth(); // +nAdditionalWidth;
		}
		pTopicWidget->AddWidget(pLineWidget);
		pLineWidget->SetRelativePosition(0, y);
		
		m_nTopicTextPointer++;
		y += m_pGUIManager->GetCurrentFontEspacementY();
	}
	return y;

#endif // 0
}

void CTopicsWindow::AddTopicText(const string& sTopicText)
{
	int y = 0;
	vector<string> vTopicLines;
	Truncate(sTopicText, m_nMaxCharPerLine, vTopicLines);
	vector<vector<pair<int, string>>> vSubString;
	for (string sLine : vTopicLines)
		DecomposeTopicText(sLine, vSubString);
	vector<vector<pair<int, CGUIWidget*>>> vTopicWidgets;
	ConvertTextArrayToWidgetArray(vSubString, vTopicWidgets);
	CGUIWindow* pTopicWidget = new CGUIWindow;
	y = PutWidgetArrayIntoTopicWidget(vTopicWidgets, pTopicWidget);

	CGUIWidget* pBlank = m_pGUIManager->CreateStaticText("      ");
	m_pTopicTextFrame->AddWidget(pBlank);
	pBlank->SetRelativePosition(0, y);
	m_nTopicTextPointer++;

	y = 0;
	int maxLineCount = (GetDimension().GetHeight() / m_pGUIManager->GetCurrentFontEspacementY()) - 1;
	int numberOflineToRemove = GetTopicTextLineCount() - maxLineCount;
	if (numberOflineToRemove > 0) {
		m_nTopicTextPointer -= numberOflineToRemove;
		bool bContinue = true;
		for (int k = 0; k < m_pTopicTextFrame->GetChildren().size();) {
			CGUIWidget* pTopicText = m_pTopicTextFrame->GetWidget(k);
			CGUIWindow* topicTextWindow = dynamic_cast<CGUIWindow*>(pTopicText);
			if (topicTextWindow) {
				numberOflineToRemove -= topicTextWindow->GetChildren().size();
				topicTextWindow->Clear();
			}
			else {
				numberOflineToRemove--;
			}
			m_pTopicTextFrame->GetChildren().pop_front();
			if (numberOflineToRemove <= 0)
				break;
		}

		int y = 0;
		for (CGUIWidget* pTopic : m_pTopicTextFrame->GetChildren()) {
			CGUIWindow* pTopicWindow = dynamic_cast<CGUIWindow*>(pTopic);
			if (pTopicWindow) {
				pTopicWindow->SetRelativePosition(0, y);
				y += pTopicWindow->GetChildren().size() * m_pGUIManager->GetCurrentFontEspacementY();
			}
			else {
				pTopic->SetRelativePosition(0, y);
				y += m_pGUIManager->GetCurrentFontEspacementY();
			}
		}
	}
}

CTopicsWindow* CTopicsWindow::GetTopicsWindowFromLink(CLink* pLink)
{
	string sLinkText;
	pLink->GetText(sLinkText);
	CGUIWindow* pLine = dynamic_cast<CGUIWindow*>(pLink->GetParent());
	if (pLine) {
		CGUIWindow* pTopicText = dynamic_cast<CGUIWindow*>(pLine->GetParent());
		if (pTopicText) {
			CGUIWindow* pTopicTextFrame = dynamic_cast<CGUIWindow*>(pTopicText->GetParent());
			if (pTopicTextFrame) {
				CTopicsWindow* pTopicWindow = dynamic_cast<CTopicsWindow*>(pTopicTextFrame->GetParent());
				return pTopicWindow;
			}
		}
	}
}

void CTopicsWindow::OnLinkClicked(CLink* pLink)
{
	string sLinkText;
	pLink->GetText(sLinkText);
	CTopicsWindow* pTopicWindow = GetTopicsWindowFromLink(pLink);
	if (pTopicWindow) {
		pTopicWindow->m_pTopicFrame->UpdateTopics();
		string sTopicText;
		if (sLinkText[0] >= 'a' && sLinkText[0] <= 'z') {
			sLinkText[0] += 'A' - 'a';
		}
		pTopicWindow->AddTopicTextFromTopicName(sLinkText);
	}			
}

void CTopicsWindow::AddTopicTextFromTopicName(string sName)
{
	string sTopicText;
	CTopicLink* pLink = m_pTopicFrame->GetTopicLink(sName);
	if (pLink) {
		sTopicText = pLink->GetTopicInfos().m_sText;
		pLink->GetTopicInfos().ExecuteActions(GetScriptManager());
	}
	/*
	else {
		map<string, vector<CTopicInfo>>::iterator itTopic = m_mTopics.find(sName);
		if (itTopic != m_mTopics.end()) {
			int topicIndex = m_pTopicFrame->AddTopicToWindow(*itTopic, GetSpeakerId());
			if (topicIndex != -1) {
				itTopic->second[topicIndex].ExecuteActions(GetScriptManager());
				sTopicText = itTopic->second[topicIndex].m_sText;
			}
			else
				sTopicText = string("Error, topic '") + sName + "' : no condition verified";
		}
		else
			sTopicText = string("Error, topic '") + sName + "' not found";
	}*/
	m_sNextTopicTextToAdd = sTopicText;
	IEventDispatcher* pEventDispatcher = static_cast<IEventDispatcher*>(m_oInterface.GetPlugin("EventDispatcher"));
	pEventDispatcher->AbonneToWindowEvent(m_pGUIManager, CTopicsWindow::OnAddTopic);
}

void CTopicsWindow::OnChoiceClicked(CLink* pTopicLink)
{
	CTopicLink* pChoice = dynamic_cast<CTopicLink*>(pTopicLink);
	CTopicsWindow* pTopicWindow = GetTopicsWindowFromLink(pTopicLink);
	pTopicWindow->GetScriptManager()->ExecuteCommand("choice=" + std::to_string(pChoice->GetChoiceNumber()) + ";");
	pTopicWindow->m_pTopicFrame->UpdateTopics();
	//pTopicWindow->AddTopicText(pTopicWindow->m_sCurrentTopicName);	
	//string sTopicText;
	//pTopicWindow->m_pTopicFrame->GetTopicText(pTopicWindow->m_sCurrentTopicName, sTopicText);
	pTopicWindow->AddTopicTextFromTopicName(pTopicWindow->m_sCurrentTopicName);

}

void CTopicsWindow::OnAddTopic(CPlugin* pPlugin, IEventDispatcher::TWindowEvent e, int, int)
{
	if (e == IEventDispatcher::TWindowEvent::T_WINDOWUPDATE) {
		CGUIManager* pGUIManager = static_cast<CGUIManager*>(pPlugin);
		CTopicsWindow* pTopicWindow = static_cast<CTopicsWindow*>(pGUIManager->GetTopicsWindow());
		pTopicWindow->AddTopicText(pTopicWindow->m_sNextTopicTextToAdd);
		IEventDispatcher* pEventDispatcher = static_cast<IEventDispatcher*>(pTopicWindow->m_oInterface.GetPlugin("EventDispatcher"));
		pEventDispatcher->DesabonneToWindowEvent(CTopicsWindow::OnAddTopic);
	}
}

void CTopicsWindow::RemoveTopicTexts()
{
	m_pTopicTextFrame->Clear();
}

void CTopicsWindow::DecodeString(string& sIn, string& sOut)
{
	int idx = sIn.find("Ã©");
	while (idx != -1) {
		sOut = sIn.replace(sIn.begin() + idx, sIn.begin() + idx + 2, "é");
		idx = sOut.find("Ã©");
	}
	idx = sIn.find("Ã");
	while (idx != -1) {
		sOut = sIn.replace(sIn.begin() + idx, sIn.begin() + idx + 2, "à");
		idx = sOut.find("Ã");
	}
}

void CTopicsWindow::LoadTopics(string sFileName)
{
	FILE* pFile = m_oFileSystem.OpenFile(sFileName, "r");
	fclose(pFile);
	string sJsonDirectory;
	m_oFileSystem.GetLastDirectory(sJsonDirectory);
	string sFilePath = sJsonDirectory + "\\" + sFileName;

	ifstream ifs(sFilePath);
	IStreamWrapper isw(ifs);
	Document doc;

	doc.ParseStream(isw);
	if (doc.IsObject()) {
		if (doc.HasMember("Topics")) {
			rapidjson::Value& topics = doc["Topics"];
			if (topics.IsArray()) {
				int count = topics.Size();
				for (int iTopic = 0; iTopic < count; iTopic++) {
					string sTitle;
					string sText;
					vector<CCondition> vConditions;
					vector<string> vAction;
					rapidjson::Value& topic = topics[iTopic];
					if (topic.IsObject()) {
						if (topic.HasMember("Title")) {
							rapidjson::Value& title = topic["Title"];
							if (title.IsString())
								sTitle = title.GetString();
						}
						if (topic.HasMember("Text")) {
							rapidjson::Value& text = topic["Text"];
							if (text.IsString())
								sText += text.GetString();
							else if (text.IsArray()) {
								for (int iLine = 0; iLine < text.GetArray().Size(); iLine++) {
									rapidjson::Value& line = text[iLine];
									if (line.IsString())
										sText += line.GetString();
								}
							}
						}
						if (topic.HasMember("Conditions")) {
							rapidjson::Value& conditions = topic["Conditions"];
							if (conditions.IsArray()) {
								for (int iCondition = 0; iCondition < conditions.GetArray().Size(); iCondition++) {
									rapidjson::Value& condition = conditions[iCondition];
									CCondition c;
									string sTestVariableName, sTestVariableValue;
									if (condition.IsObject()) {
										if (condition.HasMember("VariableName")) {
											rapidjson::Value& varName = condition["VariableName"];
											if (varName.IsString())
												c.m_sVariableName = varName.GetString();
											rapidjson::Value& value = condition["Value"];
											if (value.IsString())
												c.m_sValue = value.GetString();
											if (condition.HasMember("Comp")) {
												rapidjson::Value& comp = condition["Comp"];
												if (value.IsString()) {
													string sComp = comp.GetString();
													if (sComp == "==")
														c.m_eComp = CCondition::eEqual;
													else if (sComp == "!=")
														c.m_eComp = CCondition::eDifferent;
													else if (sComp == "<")
														c.m_eComp = CCondition::eInf;
													else if (sComp == ">")
														c.m_eComp = CCondition::eSup;
													else if (sComp == "is")
														c.m_eComp = CCondition::eIs;
													else if (sComp == "isNot")
														c.m_eComp = CCondition::eIsNot;
												}
											}
											else
												throw CEException(string("Error during parsing '") + sFileName + "' : 'Comp' is missing in condition for topic '" + topic.GetString());
										}
									}
									vConditions.push_back(c);
								}
							}
						}
						if (topic.HasMember("Action")) {
							rapidjson::Value& actions = topic["Action"];
							if (actions.IsArray()) {
								for (int iAction = 0; iAction < actions.GetArray().Size(); iAction++) {
									rapidjson::Value& action = actions[iAction];
									if (action.IsString()) {
										string sAction = action.GetString();
										DecodeString(sAction, sAction);
										vAction.push_back(sAction);
									}
								}
							}
						}
					}
					else if (topic.IsString()) {
					}
					DecodeString(sTitle, sTitle);
					DecodeString(sText, sText);
					AddTopic(sTitle, sText, vConditions, vAction);
				}
			}
		}
		if (doc.HasMember("Greatings")) {
			rapidjson::Value& greatings = doc["Greatings"];
			if (greatings.IsArray()) {
				int count = greatings.Size();
				for (int iGreating = 0; iGreating < count; iGreating++) {
					string sText;
					vector<CCondition> vConditions;
					rapidjson::Value& greating = greatings[iGreating];
					if (greating.IsObject()) {
						if (greating.HasMember("Text")) {
							rapidjson::Value& text = greating["Text"];
							if (text.IsString())
								sText += text.GetString();
							else if (text.IsArray()) {
								for (int iLine = 0; iLine < text.GetArray().Size(); iLine++) {
									rapidjson::Value& line = text[iLine];
									if (line.IsString())
										sText += line.GetString();
								}
							}
						}
						LoadJsonConditions(greating, vConditions, sFileName);						
					}
					DecodeString(sText, sText);
					AddGreating(sText, vConditions);
				}
			}
		}
	}
	else {
		CTopicException e("Erreur de chargement de topics.json");
		throw e;
	}
	ifs.close();
}


void CTopicsWindow::LoadJsonConditions(rapidjson::Value& oParentNode, vector<CCondition>& vConditions, string sFileName)
{
	if (oParentNode.HasMember("Conditions")) {
		rapidjson::Value& conditions = oParentNode["Conditions"];
		if (conditions.IsArray()) {
			for (int iCondition = 0; iCondition < conditions.GetArray().Size(); iCondition++) {
				rapidjson::Value& condition = conditions[iCondition];
				CCondition c;
				string sTestVariableName, sTestVariableValue;
				if (condition.IsObject()) {
					if (condition.HasMember("VariableName")) {
						rapidjson::Value& varName = condition["VariableName"];
						if (varName.IsString())
							c.m_sVariableName = varName.GetString();
						rapidjson::Value& value = condition["Value"];
						if (value.IsString())
							c.m_sValue = value.GetString();
						if (condition.HasMember("Comp")) {
							rapidjson::Value& comp = condition["Comp"];
							if (value.IsString()) {
								string sComp = comp.GetString();
								if (sComp == "==")
									c.m_eComp = CCondition::eEqual;
								else if (sComp == "!=")
									c.m_eComp = CCondition::eDifferent;
								else if (sComp == "<")
									c.m_eComp = CCondition::eInf;
								else if (sComp == ">")
									c.m_eComp = CCondition::eSup;
								else if (sComp == "is")
									c.m_eComp = CCondition::eIs;
								else if (sComp == "isNot")
									c.m_eComp = CCondition::eIsNot;
							}
						}
						else
							throw CEException(string("Error during parsing '") + sFileName + "' : 'Comp' is missing in condition for topic '" + oParentNode.GetString());
					}
				}
				vConditions.push_back(c);
			}
		}
	}
}


int CTopicsWindow::SelectTopic(string sTopicName, string sSpeakerId)
{
	//map<string, vector<CTopicInfo>>::iterator itTopic = m_pTopicFrame->gettop
	map<string, vector<CTopicInfo>>::iterator itTopic = m_mTopics.find(sTopicName);
	const vector<CTopicInfo>& topics = itTopic->second;
	vector<int> topicVerifiedConditionCount;
	for (int k = 0; k < topics.size(); k++)
		topicVerifiedConditionCount.push_back(0);
	int i = 0;
	for (const CTopicInfo& topic : topics) {
		for (const CCondition& condition : topic.m_vConditions) {
			if (condition.m_sVariableName == "CharacterId") {
				if (condition.m_eComp == condition.eEqual) {
					if (!condition.m_sValue.empty() && condition.m_sValue != sSpeakerId) {
						topicVerifiedConditionCount[i] = -1;
						break;
					}
					else {
						topicVerifiedConditionCount[i] += 1;
					}
				}
			}
			else {
				float fValue = m_pScriptManager->GetVariableValue(condition.m_sVariableName);
				int nValue = atoi(condition.m_sValue.c_str());
				if (float(nValue) == fValue) {
					topicVerifiedConditionCount[i] += 1;
				}
				else {
					topicVerifiedConditionCount[i] = -1;
					break;
				}
			}
		}
		i++;
	}
	int max = -1;
	int higherIndex = -1;
	for (int i = 0; i < topicVerifiedConditionCount.size(); i++) {
		if (max < topicVerifiedConditionCount[i]) {
			max = topicVerifiedConditionCount[i];
			higherIndex = i;
		}
	}
	return higherIndex;
}

int CTopicsWindow::SelectTopic(const vector<CTopicInfo>& topics, string sSpeakerId)
{
	vector<int> topicVerifiedConditionCount;
	for (int k = 0; k < topics.size(); k++)
		topicVerifiedConditionCount.push_back(0);
	int i = 0;
	for (const CTopicInfo& topic : topics) {
		for (const CCondition& condition : topic.m_vConditions) {
			if (condition.m_sVariableName == "CharacterId") {
				if (condition.m_eComp == condition.eEqual) {
					if (!condition.m_sValue.empty() && condition.m_sValue != sSpeakerId) {
						topicVerifiedConditionCount[i] = -1;
						break;
					}
					else {
						topicVerifiedConditionCount[i] += 1;
					}
				}
			}
			else {
				float fValue = m_pScriptManager->GetVariableValue(condition.m_sVariableName);
				int nValue = atoi(condition.m_sValue.c_str());
				if (float(nValue) == fValue) {
					topicVerifiedConditionCount[i] += 1;
				}
				else {
					topicVerifiedConditionCount[i] = -1;
					break;
				}
			}
		}
		i++;
	}
	int max = -1;
	int higherIndex = -1;
	for (int i = 0; i < topicVerifiedConditionCount.size(); i++) {
		if (max < topicVerifiedConditionCount[i]) {
			max = topicVerifiedConditionCount[i];
			higherIndex = i;
		}
	}
	return higherIndex;
}

void CTopicsWindow::SetCurrentTopicName(string sTopicName)
{
	m_sCurrentTopicName = sTopicName;
}

void CTopicsWindow::SetSpeakerId(string sId)
{
	m_sSpeakerId = sId;
}

string CTopicsWindow::GetSpeakerId()
{
	return m_sSpeakerId;
}

CTopicFrame::CTopicFrame(EEInterface& oInterface, int width, int height, map<string, vector<CTopicInfo>>& mTopics) :
	CGUIWindow("Gui/topic-frame.bmp", oInterface, width, height),
	m_oInterface(oInterface),
	m_pGUIManager(nullptr),
	m_nXTextMargin(5),
	m_nYTextmargin(5),
	m_nYmargin(21),
	m_nTextHeight(25),
	m_nTopicBorderWidth(218),
	m_mTopics(mTopics)
{
	m_mFontColorFromTopicState[eNormal] = IGUIManager::eWhite;
	m_mFontColorFromTopicState[ePressed] = IGUIManager::eTurquoise;
	m_oInterface.HandlePluginCreation("GUIManager", OnGUIManagerCreated, this);

	CTopicsWindow* pTopicsWindow = GetParent();
}

CTopicFrame::~CTopicFrame()
{

}

CTopicsWindow* CTopicFrame::GetParent()
{
	return (CTopicsWindow*)m_pParent;
}

void CTopicFrame::OnGUIManagerCreated(CPlugin* plugin, IBaseObject* pData)
{
	CTopicFrame* pTopicFrame = dynamic_cast<CTopicFrame*>(pData);
	pTopicFrame->m_pGUIManager = static_cast<CGUIManager*>(plugin);
}

int CTopicFrame::GetTextHeight()
{
	return m_nTextHeight;
}

CTopicLink* CTopicFrame::GetTopicLink(string sTopicName)
{
	deque<CGUIWidget*>::iterator itTopic = std::find_if(m_vWidget.begin(), m_vWidget.end(), [sTopicName](CGUIWidget* pWidget) 
	{
		CTopicLink* pTopicLink = static_cast<CTopicLink*>(pWidget);
		return pTopicLink->GetText() == sTopicName;
	});

	if (itTopic != m_vWidget.end()) {
		CTopicLink* pLink = static_cast<CTopicLink*>(*itTopic);
		return pLink;
	}
	return nullptr;
}

void CTopicFrame::GetVarValue(string sVarName, string sCharacterId, string& sValue)
{
	if (sVarName == "CharacterName") {
		sValue = sCharacterId;
	}
}

void CTopicFrame::Format(string sTopicText, string sSpeakerId, string& sFormatedText)
{
	int nIndex = 0;
	int nLastIndex = 0;
	while (nIndex < sTopicText.size()) {
		string varTag = "<var>";
		nIndex = sTopicText.find(varTag, nIndex);
		sFormatedText += sTopicText.substr(nLastIndex, nIndex);
		if (nIndex == -1)
			break;
		int nEndVarIndex = sTopicText.find("</var>");
		if (nEndVarIndex == -1) {
			ostringstream oss;
			oss << "'var' tag not closed";
			throw CEException(oss.str());
		}
		int nVarNameSize = nEndVarIndex - nIndex - varTag.size();
		string sVarName = sTopicText.substr(nIndex + varTag.size(), nVarNameSize);
		string sVarValue;
		GetVarValue(sVarName, sSpeakerId, sVarValue);
		sFormatedText += sVarValue;

		nIndex += varTag.size() + sVarName.size() + varTag.size() + 1;
		nLastIndex = nIndex;
	}
}

void CTopicFrame::UpdateTopics()
{
	int iLine = 0;
	Clear();
	for (map<string, vector<CTopicInfo>>::const_iterator itTopic = m_mTopics.begin(); itTopic != m_mTopics.end(); itTopic++) {
		AddTopicToWindow(*itTopic, GetParent()->GetSpeakerId());
	}
}

int CTopicFrame::AddTopicToWindow(const pair<string, vector<CTopicInfo>>& topic, string sSpeakerId)
{
	if (IsConditionChecked(topic.second, GetParent()->GetSpeakerId())) {
		int iLine = GetWidgetCount();
		int y = iLine * m_nTextHeight + m_nYTextmargin;
		string sTopic = topic.first;
		CTopicLink* pTitle = new CTopicLink(m_oInterface, sTopic);
		int nIdx = GetParent()->SelectTopic(topic.second, GetParent()->GetSpeakerId());
		if (nIdx < 0)
			return -1;
		string sFormatedText;
		Format(topic.second[nIdx].m_sText, GetParent()->GetSpeakerId(), sFormatedText);
		CTopicInfo ti(topic.second.at(nIdx));
		ti.m_sText = sFormatedText;
		ti.m_sName = sTopic;
		pTitle->SetTopicInfos(ti);

		pTitle->SetClickedCallback(OnClickTopic);
		AddWidget(pTitle);
		pTitle->SetRelativePosition(m_nXTextMargin, iLine * m_nTextHeight + m_nYTextmargin);
		pTitle->SetColorByState(CLink::TState::eNormal, IGUIManager::TFontColor::eWhite);
		pTitle->SetColorByState(CLink::TState::eHover, IGUIManager::TFontColor::eYellow);
		pTitle->SetColorByState(CLink::TState::eClick, IGUIManager::TFontColor::eTurquoise);
		iLine++;
		return nIdx;
	}
	return -1;
}

void CTopicFrame::OnClickTopic(CLink* pLink)
{
	CTopicFrame* pTopicFrame = dynamic_cast<CTopicFrame*>(pLink->GetParent());
	if (pTopicFrame) {
		CTopicLink* pTopicLink = static_cast<CTopicLink*>(pLink);
		pTopicFrame->OnItemSelected(pTopicLink);
	}
}

void CTopicFrame::DestroyTopicsWidgets()
{
	CTopicsWindow* pTopicWindow = dynamic_cast<CTopicsWindow*>(GetParent());
	pTopicWindow->RemoveTopicTexts();
	//m_mDisplayedTopicWidgets.clear();
}

void CTopicFrame::Display()
{
	CGUIWindow::Display();
}

int CTopicFrame::GetTopicIndexFromY(int y)
{
	int i = (int)(((float)y - m_oPosition.GetY() - m_nYTextmargin) / (float)m_nTextHeight);
	return i;
}

void CTopicFrame::SetParent(CGUIWidget* parent)
{
	CGUIWidget::SetParent(parent);
	SetRelativePosition(m_pParent->GetDimension().GetWidth() - m_nTopicBorderWidth, m_nYmargin);
}

void CTopicFrame::OnItemSelected(CTopicLink* pTitle)
{
	CTopicsWindow* pTopicWindow = dynamic_cast<CTopicsWindow*>(GetParent());
	pTopicWindow->SetCurrentTopicName(pTitle->GetTopicInfos().m_sName);
	pTopicWindow->AddTopicText(pTitle->GetTopicInfos().m_sText);
	pTitle->GetTopicInfos().ExecuteActions(GetParent()->GetScriptManager());
}

void CTopicFrame::OnItemHover(CGUIWidget* pTitle)
{
}

int CTopicFrame::ConvertValueToInt(string sValue)
{
	return atoi(sValue.c_str());
}

int CTopicFrame::IsConditionChecked(const vector<CTopicInfo>& topics, string sSpeakerId)
{
	vector<bool> checked;
	for(const CTopicInfo& topic : topics)
		checked.push_back(true);
	int i = 0;
	for (const CTopicInfo& topic : topics) {
		for (const CCondition& condition : topic.m_vConditions) {
			if (condition.m_sVariableName == "CharacterId") {
				if (condition.m_eComp == condition.eEqual) {
					if (!condition.m_sValue.empty() && condition.m_sValue != sSpeakerId) {
						checked[i] = false;
						break;
					}
					else {
						checked[i] = true;
					}
				}
			}
			else {
				float fValue = GetParent()->GetScriptManager()->GetVariableValue(condition.m_sVariableName);
				int nValue = ConvertValueToInt(condition.m_sValue);
				if (float(nValue) == fValue) {
					checked[i] = true;
				}
				else {
					checked[i] = false;
					break;
				}
			}
		}
		i++;
	}
	int count;
	for (bool check : checked) {
		if (check)
			return true;
	}
	return false;
}

void CTopicInfo::ExecuteActions(IScriptManager* pScriptManager)
{
	for (const string& sAction : m_vAction)
		pScriptManager->ExecuteCommand(sAction);
}

CTopicInfo&	CTopicLink::GetTopicInfos()
{
	return m_oTopicInfos;
}

void CTopicLink::SetTopicInfos(const CTopicInfo& oTopicInfos)
{
	m_oTopicInfos = oTopicInfos;
}

int CTopicLink::GetChoiceNumber()
{
	return m_nChoiceNumber;
}