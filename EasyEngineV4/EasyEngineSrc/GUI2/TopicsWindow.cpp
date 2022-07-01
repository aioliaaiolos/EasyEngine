#include "Interface.h"
#include "IFileSystem.h"
#include "IScriptManager.h"
#include "TopicsWindow.h"
#include "Utils2/rectangle.h"
#include "Utils2/StringUtils.h"
#include "GUIManager.h"
#include "IRessource.h"
#include <sstream>

// rapidjson
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/filereadstream.h"
#include <fstream>

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
	m_oRessourceManager(static_cast<IRessourceManager&>(*oInterface.GetPlugin("RessourceManager")))
{
	SetPosition(400, 200);
	m_pTopicFrame = new CTopicFrame(oInterface, 198, 759);
	m_pTopicTextFrame = new CGUIWindow("Gui/TopicsTextWindow.bmp", oInterface, 660, 780);
	AddWidget(m_pTopicFrame);
	AddWidget(m_pTopicTextFrame);
	m_pTopicTextFrame->SetRelativePosition(10, 10);
	SetGUIMode(true);

	LoadTopics("topics.json");
	oInterface.HandlePluginCreation("GUIManager", OnGUIManagerCreated, this);
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
	}
}

void CTopicsWindow::AddTopic(string sTopicName, string sText, vector<CCondition>& conditions)
{
	m_pTopicFrame->AddTopic(sTopicName, sText, conditions);
}

CTopicsWindow::~CTopicsWindow()
{
}

void CTopicsWindow::Display()
{
	CGUIWindow::Display();
}

void CTopicsWindow::SetSpeakerId(string sId)
{
	m_pTopicFrame->SetSpeakerId(sId);
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

void CTopicsWindow::AddTopicText(const string& sTopicText)
{
	vector<string> vLines;
	CStringUtils::Truncate(sTopicText, m_nMaxCharPerLine, vLines);

	vector<vector<pair<bool, CGUIWidget*>>> vText;
	int nIndex = 0;
	for (string& line : vLines) {
		vText.push_back(vector<pair<bool, CGUIWidget*>>());
		vector<pair<bool, string>> vSubString;
		ExtractLink(line, vSubString);
		for (pair<bool, string>& p : vSubString) {
			CGUIWidget* pTextWidget = nullptr;
			if (p.first)
				pTextWidget = new CLink(m_oInterface, p.second);
			else
				pTextWidget = m_pGUIManager->CreateStaticText(p.second);
			vText.back().push_back(pair<bool, CGUIWidget*>(p.first, pTextWidget));
		}
	}

	int y = m_nTopicTextPointer * m_pGUIManager->GetCurrentFontEspacementY();
	for (vector<pair<bool, CGUIWidget*>> text : vText) {
		int x = 0;
		for (pair<bool, CGUIWidget*>& p : text) {
			m_pTopicTextFrame->AddWidget(p.second);
			p.second->SetRelativePosition(x, y);
			x += p.second->GetDimension().GetWidth();
		}
		m_nTopicTextPointer++;
		y += m_pGUIManager->GetCurrentFontEspacementY();
	}

	CGUIWidget* pBlank = m_pGUIManager->CreateStaticText("      ");
	m_pTopicTextFrame->AddWidget(pBlank);
	pBlank->SetRelativePosition(0, y);
	m_nTopicTextPointer++;

	int maxLineCount = (GetDimension().GetHeight() / m_pGUIManager->GetCurrentFontEspacementY()) - 1;
	int numberOflineToRemove = m_nTopicTextPointer - maxLineCount;
	if (numberOflineToRemove > 0) {
		m_nTopicTextPointer -= numberOflineToRemove;
		for (int i = 0; i < numberOflineToRemove; i++) {
			m_pTopicTextFrame->GetChildren().pop_front();
		}
		int offsetY = numberOflineToRemove * m_pGUIManager->GetCurrentFontEspacementY();
		for (CGUIWidget* pText : m_pTopicTextFrame->GetChildren())
			pText->SetPosition(pText->GetPosition().GetX(), pText->GetPosition().GetY() - offsetY);
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
	}
	ifs.close();
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
	m_oInterface.HandlePluginCreation("ScriptManager", OnScriptManagerCreated, this);
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

void CTopicFrame::OnScriptManagerCreated(CPlugin* plugin, void* pData)
{
	CTopicFrame* pTopicFrame = (CTopicFrame*)pData;
	pTopicFrame->m_pScriptManager = static_cast<IScriptManager*>(plugin);
}

int CTopicFrame::GetTextHeight()
{
	return m_nTextHeight;
}

void CTopicFrame::SetSpeakerId(string sId)
{
	m_sSpeakerId = sId;
}

void CTopicFrame::AddTopic(string sTopicName, string sText, vector<CCondition>& conditions)
{
	CTopicInfo topic;
	topic.m_sText = sText;
	topic.m_vConditions = conditions;
	m_mTopics[sTopicName].push_back(topic);
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
	m_mDisplayedTopics.clear();
	Clear();
	for (map<string, vector<CTopicInfo>>::iterator itTopic = m_mTopics.begin(); itTopic != m_mTopics.end(); itTopic++) {
		if (IsConditionChecked(itTopic->second, m_sSpeakerId)) {
			int y = iLine * m_nTextHeight + m_nYTextmargin;
			string sTopic = itTopic->first;
			CGUIWidget* pTitle = m_pGUIManager->CreateStaticText(sTopic);
			
			int nIdx = SelectTopic(itTopic->second, m_sSpeakerId);
			if (nIdx < 0)
				nIdx = 0;
			
			string sFormatedText;
			Format(itTopic->second[nIdx].m_sText, m_sSpeakerId, sFormatedText);

			m_mDisplayedTopicWidgets[pTitle] = sFormatedText;
			CListener* pListener = new CListener;
			pListener->SetEventCallBack(OnTopicEvent);
			pTitle->SetListener(pListener);
			AddWidget(pTitle);
			pTitle->SetRelativePosition(m_nXTextMargin, iLine * m_nTextHeight + m_nYTextmargin);
			iLine++;
		}
	}
}

void CTopicFrame::DestroyTopicsWidgets()
{
	CTopicsWindow* pTopicWindow = dynamic_cast<CTopicsWindow*>(GetParent());
	pTopicWindow->RemoveTopicTexts();
	for (map<CGUIWidget*, string>::iterator itTopic = m_mDisplayedTopicWidgets.begin(); itTopic != m_mDisplayedTopicWidgets.end(); itTopic++)
		delete itTopic->first;	
	m_mDisplayedTopicWidgets.clear();
}

void CTopicFrame::OnTopicEvent(IGUIManager::ENUM_EVENT nEvent, CGUIWidget* pWidget, int x, int y)
{
	int i = 0;
	CTopicFrame* pTopicFrame = nullptr;
	switch (nEvent) {
	case IGUIManager::EVENT_LMOUSECLICK:
		i = i;
		break;
	case IGUIManager::EVENT_LMOUSERELEASED:
		pTopicFrame = dynamic_cast<CTopicFrame*>(pWidget->GetParent());
		if (pTopicFrame) {
			pTopicFrame->OnItemSelected(pWidget);
		}
	case IGUIManager::EVENT_MOUSEENTERED:
		pTopicFrame = dynamic_cast<CTopicFrame*>(pWidget->GetParent());
		if (pTopicFrame) {
			pTopicFrame->OnItemHover(pWidget);
		}
		break;
		
	default:
		break;

	}
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

void CTopicFrame::OnItemSelected(CGUIWidget* pTitle)
{
	CTopicsWindow* pTopicWindow = dynamic_cast<CTopicsWindow*>(GetParent());
	map<CGUIWidget*, string>::iterator itTopic = m_mDisplayedTopicWidgets.find(pTitle);
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

int CTopicFrame::SelectTopic(vector<CTopicInfo>& topics, string sSpeakerId)
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
			else{ 
				float fValue = m_pScriptManager->GetVariableValue(condition.m_sVariableName);
				int nValue = ConvertValueToInt(condition.m_sValue);
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
