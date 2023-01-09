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
	m_nMaxCharPerLine(100),
	m_nTopicTextPointer(0),
	m_oFileSystem(static_cast<IFileSystem&>(*oInterface.GetPlugin("FileSystem"))),
	m_oRenderer(static_cast<IRenderer&>(*oInterface.GetPlugin("Renderer"))),
	m_oRessourceManager(static_cast<IRessourceManager&>(*oInterface.GetPlugin("RessourceManager"))),
	m_pScriptManager(nullptr)
{
	SetPosition(400, 200);
	m_pTopicFrame = new CTopicFrame(oInterface, 198, 759);
	m_pTopicTextFrame = new CGUIWindow("Gui/TopicsTextWindow.bmp", oInterface, 660, 780);
	AddWidget(m_pTopicFrame);
	AddWidget(m_pTopicTextFrame);
	m_pTopicTextFrame->SetRelativePosition(10, 10);
	SetGUIMode(true);

	LoadTopics("topics.json");
	m_oInterface.HandlePluginCreation("GUIManager", OnGUIManagerCreated, this);
	m_oInterface.HandlePluginCreation("ScriptManager", OnScriptManagerCreated, this);
}

void CTopicsWindow::OnScriptManagerCreated(CPlugin* plugin, void* pData)
{
	CTopicsWindow* pTopicWindow = (CTopicsWindow*)pData;
	pTopicWindow->m_pScriptManager = static_cast<IScriptManager*>(plugin);
}

IScriptManager* CTopicsWindow::GetScriptManager()
{
	return m_pScriptManager;
}

void CTopicsWindow::OnGUIManagerCreated(CPlugin* pGUIManager, void* pData)
{
	CTopicsWindow* pTopicWindow = (CTopicsWindow*)pData;
	pTopicWindow->m_pGUIManager = static_cast<CGUIManager*>(pGUIManager);
}

void CTopicsWindow::OnShow(bool bShow)
{
	if (!bShow) {
		m_sText.clear();
		m_pTopicFrame->DestroyTopicsWidgets();
		m_nTopicTextPointer = 0;
	}
	else {
		m_pTopicFrame->CreateTopicsWidgets();

		int index = SelectTopic(m_vGreatings, m_sSpeakerId);
		if(index != -1)
			AddTopicText(m_vGreatings[index].m_sText);
	}
}

void CTopicsWindow::AddTopic(string sTopicName, string sText, vector<CCondition>& conditions)
{
	m_pTopicFrame->AddTopic(sTopicName, sText, conditions);
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

void ExtractLink(string sText, vector<pair<bool, string>>& vText)
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
			vText.push_back(pair<bool, string>(false, subString));
			vText.push_back(pair<bool, string>(true, link));
			sRemainingString = sRemainingString.substr(closeTagIndex + sCloseTag.size());
		}
		else {
			vText.push_back(pair<bool, string>(false, sRemainingString));
			sRemainingString.clear();
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

void CTopicsWindow::AddTopicText(const string& sTopicText)
{
	vector<string> vLines;
	CStringUtils::Truncate(sTopicText, m_nMaxCharPerLine, vLines);

	vector<vector<pair<bool, CGUIWidget*>>> vTopicWidgets;
	int nIndex = 0;
	for (string& sTopicText : vLines) {
		vTopicWidgets.push_back(vector<pair<bool, CGUIWidget*>>());
		vector<pair<bool, string>> vSubString;
		ExtractLink(sTopicText, vSubString);
		for (pair<bool, string>& topicLine : vSubString) {
			CGUIWidget* pTextWidget = nullptr;
			if (topicLine.first) {
				pTextWidget = new CLink(m_oInterface, topicLine.second);
				CLink* pLink = static_cast<CLink*>(pTextWidget);
				pLink->SetClickedCallback(OnLinkClicked);
			}
			else
				pTextWidget = m_pGUIManager->CreateStaticText(topicLine.second);
			pTextWidget->m_sUserData = topicLine.second;
			vTopicWidgets.back().push_back(pair<bool, CGUIWidget*>(topicLine.first, pTextWidget));
		}
	}

	CGUIWindow* pTopicWidget = new CGUIWindow;
	m_pTopicTextFrame->AddWidget(pTopicWidget);
	int y = 0;
	pTopicWidget->SetRelativePosition(0, m_nTopicTextPointer * m_pGUIManager->GetCurrentFontEspacementY());
	for (vector<pair<bool, CGUIWidget*>> lineWidgets : vTopicWidgets) {
		int x = 0;
		CGUIWindow* pLineWidget = new CGUIWindow;
		for (pair<bool, CGUIWidget*>& p : lineWidgets) {
			pLineWidget->AddWidget(p.second);
			p.second->SetRelativePosition(x, y);
			x += p.second->GetDimension().GetWidth();
		}
		pTopicWidget->AddWidget(pLineWidget);
		m_nTopicTextPointer++;
		y += m_pGUIManager->GetCurrentFontEspacementY();
	}

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

void CTopicsWindow::OnLinkClicked(CLink* pLink)
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
				if (pTopicWindow) {
					CTopicFrame* pTopicFrame = nullptr;
					for (CGUIWidget* pWidget : pTopicWindow->GetChildren()) {
						pTopicFrame = dynamic_cast<CTopicFrame*>(pWidget);
						if (pTopicFrame)
							break;
					}
					string sTopicText;
					if (sLinkText[0] >= 'a' && sLinkText[0] <= 'z') {
						sLinkText[0] += 'A' - 'a';
					}
					pTopicFrame->GetTopicText(sLinkText, sTopicText);
					pTopicWindow->m_sNextTopicTextToAdd = sTopicText;
					IEventDispatcher* pEventDispatcher = static_cast<IEventDispatcher*>(pTopicWindow->m_oInterface.GetPlugin("EventDispatcher"));
					pEventDispatcher->AbonneToWindowEvent(pTopicWindow->m_pGUIManager, CTopicsWindow::OnAddTopic);
				}
			}
		}
	}
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
					}
					else if (topic.IsString()) {
					}
					DecodeString(sTitle, sTitle);
					DecodeString(sText, sText);
					AddTopic(sTitle, sText, vConditions);
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


int CTopicsWindow::SelectTopic(vector<CTopicInfo>& topics, string sSpeakerId)
{
	vector<int> topicVerifiedConditionCount;
	for (int k = 0; k < topics.size(); k++)
		topicVerifiedConditionCount.push_back(0);
	int i = 0;
	for (CTopicInfo& topic : topics) {
		for (CCondition& condition : topic.m_vConditions) {
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

void CTopicsWindow::SetSpeakerId(string sId)
{
	m_sSpeakerId = sId;
}

string CTopicsWindow::GetSpeakerId()
{
	return m_sSpeakerId;
}

CTopicFrame::CTopicFrame(EEInterface& oInterface, int width, int height) :
	CGUIWindow("Gui/topic-frame.bmp", oInterface, width, height),
	m_oInterface(oInterface),
	m_pGUIManager(nullptr),
	m_nXTextMargin(5),
	m_nYTextmargin(5),
	m_nYmargin(21),
	m_nTextHeight(25),
	m_nTopicBorderWidth(218),
	m_pScriptManager(nullptr)
{
	m_mFontColorFromTopicState[eNormal] = IGUIManager::eWhite;
	m_mFontColorFromTopicState[ePressed] = IGUIManager::eTurquoise;
	m_oInterface.HandlePluginCreation("GUIManager", OnGUIManagerCreated, this);
}

CTopicFrame::~CTopicFrame()
{

}

CTopicsWindow* CTopicFrame::GetParent()
{
	return (CTopicsWindow*)m_pParent;
}

void CTopicFrame::OnGUIManagerCreated(CPlugin* plugin, void* pData)
{
	CTopicFrame* pTopicFrame = (CTopicFrame*)pData;
	pTopicFrame->m_pGUIManager = static_cast<CGUIManager*>(plugin);
}

int CTopicFrame::GetTextHeight()
{
	return m_nTextHeight;
}

void CTopicFrame::AddTopic(string sTopicName, string sText, vector<CCondition>& conditions)
{
	CTopicInfo topic;
	topic.m_sText = sText;
	topic.m_vConditions = conditions;
	m_mTopics[sTopicName].push_back(topic);
}

void CTopicFrame::GetTopicText(string sTopicTitle, string& sTopicText)
{
	map<CLink*, string>::iterator itTopic = std::find_if(m_mDisplayedTopicWidgets.begin(), m_mDisplayedTopicWidgets.end(),
		[sTopicTitle](pair<CGUIWidget*, string> const& p) {return p.first->m_sUserData == sTopicTitle; });
	sTopicText = itTopic->second;
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

void CTopicFrame::CreateTopicsWidgets()
{
	int iLine = 0;
	Clear();
	for (map<string, vector<CTopicInfo>>::iterator itTopic = m_mTopics.begin(); itTopic != m_mTopics.end(); itTopic++) {
		if (IsConditionChecked(itTopic->second, GetParent()->GetSpeakerId())) {
			int y = iLine * m_nTextHeight + m_nYTextmargin;
			string sTopic = itTopic->first;
			CLink* pTitle = new CLink(m_oInterface, sTopic);
			pTitle->m_sUserData = sTopic;
			int nIdx = GetParent()->SelectTopic(itTopic->second, GetParent()->GetSpeakerId());
			if (nIdx < 0)
				nIdx = 0;
			
			string sFormatedText;
			Format(itTopic->second[nIdx].m_sText, GetParent()->GetSpeakerId(), sFormatedText);
			m_mDisplayedTopicWidgets[pTitle] = sFormatedText;
			pTitle->SetClickedCallback(OnClickTopic);
			AddWidget(pTitle);
			pTitle->SetRelativePosition(m_nXTextMargin, iLine * m_nTextHeight + m_nYTextmargin);
			pTitle->SetColorByState(CLink::TState::eNormal, IGUIManager::TFontColor::eWhite);
			pTitle->SetColorByState(CLink::TState::eHover, IGUIManager::TFontColor::eYellow);
			pTitle->SetColorByState(CLink::TState::eClick, IGUIManager::TFontColor::eTurquoise);
			iLine++;
		}
	}
}

void CTopicFrame::OnClickTopic(CLink* pLink)
{
	CTopicFrame* pTopicFrame = dynamic_cast<CTopicFrame*>(pLink->GetParent());
	if (pTopicFrame) {
		pTopicFrame->OnItemSelected(pLink);
	}
}

void CTopicFrame::DestroyTopicsWidgets()
{
	CTopicsWindow* pTopicWindow = dynamic_cast<CTopicsWindow*>(GetParent());
	pTopicWindow->RemoveTopicTexts();
	m_mDisplayedTopicWidgets.clear();
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

void CTopicFrame::OnItemSelected(CLink* pTitle)
{
	CTopicsWindow* pTopicWindow = dynamic_cast<CTopicsWindow*>(GetParent());
	map<CLink*, string>::iterator itTopic = m_mDisplayedTopicWidgets.find(pTitle);
	if (itTopic != m_mDisplayedTopicWidgets.end())
		pTopicWindow->AddTopicText(itTopic->second);
}

void CTopicFrame::OnItemHover(CGUIWidget* pTitle)
{
}

int CTopicFrame::ConvertValueToInt(string sValue)
{
	return atoi(sValue.c_str());
}

int CTopicFrame::IsConditionChecked(vector<CTopicInfo>& topics, string sSpeakerId)
{
	vector<bool> checked;
	for(CTopicInfo& topic : topics)
		checked.push_back(true);
	int i = 0;
	for (CTopicInfo& topic : topics) {
		for (CCondition& condition : topic.m_vConditions) {
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
				float fValue = m_pScriptManager->GetVariableValue(condition.m_sVariableName);
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
