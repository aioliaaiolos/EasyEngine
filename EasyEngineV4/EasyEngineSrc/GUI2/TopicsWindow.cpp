#include "Interface.h"
#include "IFileSystem.h"
#include "IScriptManager.h"
#include "TopicsWindow.h"
#include "Utils2/rectangle.h"
#include "GUIManager.h"
#include "IRessource.h"
#include <sstream>

// rapidjson
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/filereadstream.h"
#include <fstream>

using namespace rapidjson;

CTopicsWindow::CTopicsWindow(EEInterface& oInterface, int width, int height) :
	CGUIWindow("Gui/TopicsWindow.bmp", oInterface, CDimension(width, height)),
	m_nMaxCharPerLine(96),
	m_oFileSystem(static_cast<IFileSystem&>(*oInterface.GetPlugin("FileSystem"))),
	m_oRenderer(static_cast<IRenderer&>(*oInterface.GetPlugin("Renderer"))),
	m_oRessourceManager(static_cast<IRessourceManager&>(*oInterface.GetPlugin("RessourceManager")))
{
	SetPosition(400, 200);
	m_pTopicFrame = new CTopicFrame(oInterface, 198, 759);
	AddWidget(m_pTopicFrame);
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
	if(!bShow)
		m_sText.clear();
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
	m_pGUIManager->Print(m_sText, GetPosition().GetX(), GetPosition().GetY());
}

void CTopicsWindow::DisplayTopicInfos(string sTopicInfo)
{
	vector<int> returnIndices;
	int textIndex = 0;
	while (textIndex < sTopicInfo.size()) {
		int lineIndex = 0;
		int lastWordIndex = 0;
		while ( (textIndex < sTopicInfo.size()) && (lineIndex < m_nMaxCharPerLine) ) {
			if (sTopicInfo[textIndex] == ' ') {
				lastWordIndex = textIndex;
			}
			lineIndex++;
			textIndex++;
		}
		if(lineIndex >= m_nMaxCharPerLine)
			returnIndices.push_back(lastWordIndex);
	}

	string sTruncatedString = sTopicInfo;
	for (int i = 0; i < returnIndices.size(); i++) {
		string::iterator itIndex = sTruncatedString.begin() + returnIndices[i];
		itIndex = sTruncatedString.erase(itIndex);
		sTruncatedString.insert(itIndex, '\n');
	}
	m_sText += "\n" + sTruncatedString + "\n";

	int indexLast = 0;
	int indexNext = 0;
	int lineCount = 0;
	do {
		indexNext = m_sText.find('\n', indexLast);
		indexLast = indexNext + 1;
		lineCount++;
	} while (indexNext != -1);
	
	int maxLineCount = (GetDimension().GetHeight() / m_pGUIManager->GetCurrentFontEspacementY()) - 1;
	int numberOflineToRemove = lineCount - maxLineCount;
	bool bTruncateLine = numberOflineToRemove > 0;
	
	while (numberOflineToRemove-- > 0) {
		int index = m_sText.find('\n');
		m_sText = m_sText.substr(index + 1);
	}
	if(bTruncateLine)
		m_sText.insert(0, "\n");
}

void CTopicsWindow::SetSpeakerId(string sId)
{
	m_pTopicFrame->SetSpeakerId(sId);
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
	CGUIWidget(oInterface, "Gui/topic-frame.bmp", width, height),
	m_oInterface(oInterface),
	m_pGUIManager(nullptr),
	m_nXTextMargin(5),
	m_nYTextmargin(5),
	m_nYmargin(21),
	m_nTextHeight(25),
	m_nTopicBorderWidth(218),
	m_pScriptManager(nullptr)
{
	CListener* pEventListener = new CListener;
	pEventListener->SetEventCallBack(OnEventCallback);
	SetListener(pEventListener);
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
	pTopicFrame->m_pGUIManager = static_cast<IGUIManager*>(plugin);
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

void CTopicFrame::Display()
{
	CGUIWidget::Display();
	int iLine = 0;
	m_mDisplayedTopics.clear();
	for(map<string, vector<CTopicInfo>>::iterator itTopic = m_mTopics.begin(); itTopic != m_mTopics.end(); itTopic++){
		if (IsConditionChecked(itTopic->second, m_sSpeakerId)) {
			m_mDisplayedTopics[itTopic->first] = itTopic->second;
			int y = m_oPosition.GetY() + iLine * m_nTextHeight + m_nYTextmargin;
			m_pGUIManager->Print(itTopic->first, GetPosition().GetX() + m_nXTextMargin, y, m_mFontColorFromTopicState[m_mTopicsState[itTopic->first]]);
			iLine++;
		}
	}
}

int CTopicFrame::GetTopicIndexFromY(int y)
{
	int i = (int)(((float)y - m_oPosition.GetY() - m_nYTextmargin) / (float)m_nTextHeight);
	return i;
}

void CTopicFrame::SetParent(CGUIWidget* parent)
{
	CGUIWidget::SetParent(parent);
	SetPosition(GetPosition().GetX() + m_pParent->GetDimension().GetWidth() - m_nTopicBorderWidth, GetPosition().GetY() + m_nYmargin);
}

void CTopicFrame::OnItemSelected(int itemIndex) 
{
	map<string, vector<CTopicInfo>>::iterator itTopic = m_mDisplayedTopics.begin();
	std::advance(itTopic, itemIndex);
	m_mTopicsState[itTopic->first] = ePressed;
}

void CTopicFrame::OnItemRelease(int itemIndex)
{
	map<string, vector<CTopicInfo>>::iterator itTopic = m_mDisplayedTopics.begin();
	std::advance(itTopic, itemIndex);
	m_mTopicsState[itTopic->first] = eReleased;
	int idx = SelectTopic(itTopic->second, m_sSpeakerId);
	if (idx < 0) 
		idx = 0;
	GetParent()->DisplayTopicInfos(itTopic->second[idx].m_sText);
}

void CTopicFrame::OnItemHover(int itemIndex)
{
	map<string, vector<CTopicInfo>>::iterator itTopic = m_mDisplayedTopics.begin();
	std::advance(itTopic, itemIndex);
	if(itTopic != m_mDisplayedTopics.end())
		m_mTopicsState[itTopic->first] = eHover;
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

void CTopicFrame::OnEventCallback(IGUIManager::ENUM_EVENT nEvent, CGUIWidget* pWidget, int x, int y)
{
	switch (nEvent) {
	case IGUIManager::EVENT_OUTSIDE:
	case IGUIManager::EVENT_NONE:
	case IGUIManager::EVENT_MOUSEMOVE:
	{
		CTopicFrame* pTopicFrame = dynamic_cast<CTopicFrame*>(pWidget);
		if (pTopicFrame) {
			int index = pTopicFrame->GetTopicIndexFromY(y);
			if(index < pTopicFrame->m_mDisplayedTopics.size())
				pTopicFrame->OnItemHover(pTopicFrame->GetTopicIndexFromY(y));
		}
	}
	break;
	case IGUIManager::EVENT_LMOUSECLICK: {
		CTopicFrame* pTopicFrame = dynamic_cast<CTopicFrame*>(pWidget);
		if (pTopicFrame) {
			int index = pTopicFrame->GetTopicIndexFromY(y);
			if (index < pTopicFrame->m_mDisplayedTopics.size())
				pTopicFrame->OnItemSelected(index);
		}
			
		break;	
	}
	case IGUIManager::EVENT_LMOUSERELEASED: {
		CTopicFrame* pTopicFrame = dynamic_cast<CTopicFrame*>(pWidget);
		if (pTopicFrame) {
			int index = pTopicFrame->GetTopicIndexFromY(y);
			if (index < pTopicFrame->m_mDisplayedTopics.size())
				pTopicFrame->OnItemRelease(index);
		}

		break;
	}
	default:
		break;
	}
}
