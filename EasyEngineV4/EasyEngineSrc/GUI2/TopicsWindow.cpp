#include "Interface.h"
#include "IFileSystem.h"
#include "IScriptManager.h"
#include "TopicsWindow.h"
#include "Utils2/rectangle.h"
#include "Utils2/StringUtils.h"
#include "GUIManager.h"
#include "IRessource.h"
#include <sstream>
#include "IEntity.h"
#include "Interface.h"
#include "ITopicSystem.h"



CTopicsWindow::CTopicsWindow(EEInterface& oInterface, int width, int height) :
	CGUIWindow("Gui/TopicsWindow.bmp", oInterface, CDimension(width, height)),
	m_oInterface(oInterface),
	m_nMaxCharPerLine(95),
	m_nTopicTextPointer(0),
	m_oFileSystem(static_cast<IFileSystem&>(*oInterface.GetPlugin("FileSystem"))),
	m_oRenderer(static_cast<IRenderer&>(*oInterface.GetPlugin("Renderer"))),
	m_oRessourceManager(static_cast<IRessourceManager&>(*oInterface.GetPlugin("RessourceManager"))),
	m_pScriptManager(nullptr),
	m_bChoiceSet(false),
	m_oEntityManager(static_cast<IEntityManager&>(*oInterface.GetPlugin("EntityManager"))),
	m_bGoodbye(false)
{
	SetPosition(400, 200);
	m_pTopicFrame = new CTopicFrame(oInterface, 198, 759, m_oEntityManager);
	m_pTopicTextFrame = new CGUIWindow("Gui/TopicsTextWindow.bmp", oInterface, 660, 780);
	AddWidget(m_pTopicFrame);
	AddWidget(m_pTopicTextFrame);
	m_pTopicTextFrame->SetRelativePosition(10, 10);
	SetGUIMode(true);
	m_oInterface.HandlePluginCreation("GUIManager", [this](CPlugin* pGUIManager)
	{
		m_pGUIManager = static_cast<CGUIManager*>(pGUIManager);
	});
	m_oInterface.HandlePluginCreation("ScriptManager",[this](CPlugin* plugin)
	{
		m_pScriptManager = static_cast<IScriptManager*>(plugin);
	});

	m_oInterface.HandlePluginCreation("TopicSystem", [this] (CPlugin* pPlugin) {
		m_pTopicSystem = dynamic_cast<ITopicSystem*>(pPlugin);
	});
}


IScriptManager* CTopicsWindow::GetScriptManager()
{
	return m_pScriptManager;
}


void CTopicsWindow::OnShow(bool bShow)
{
	CGUIWindow::OnShow(bShow);
	if (!bShow) {
		m_sCurrentTopicName = "";
		m_sText.clear();
		m_pTopicFrame->DestroyTopicsWidgets();
		m_nTopicTextPointer = 0;
	}
	else {
		m_bGoodbye = false;
		m_pTopicFrame->UpdateTopics();
		DisplayGreeting();
	}
}

void CTopicsWindow::DisplayGreeting()
{
	ITopic* pTopic = m_pTopicSystem->SelectGreeting(m_sSpeakerId);
	if (pTopic) {
		AddTopicText(pTopic->GetText());
		string error;
		if (!m_pTopicSystem->ExecuteActions(pTopic, error))
			AddTopicText(error);
	}
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
				if(!text.empty())
					vText.push_back(pair<int, string>(0, text));
				vText.push_back(pair<int, string>(nChoiceNumer, choiceText));
				sRemainingString = sRemainingString.substr(closeTagIndex + sCloseTag.size());
			}
			else {
				string sOpenGoodbyeTag = "<goodbye>";
				string sCloseGoodbye = "</goodbye>";
				int openTagIndex = sRemainingString.find(sOpenGoodbyeTag);
				if (openTagIndex != -1) {
					int closeTagIndex = sRemainingString.find(sCloseGoodbye, openTagIndex + sOpenGoodbyeTag.size());
					string link = sRemainingString.substr(openTagIndex + sOpenGoodbyeTag.size(), closeTagIndex - openTagIndex - sCloseGoodbye.size() + 1);
					vText.push_back(pair<int, string>(256, link));
					sRemainingString = "";
				}
				else {
					vText.push_back(pair<bool, string>(false, sRemainingString));
					sRemainingString.clear();
				}
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

	int nChoiceNumber = 0;
	for (const pair<int, string>& choice : choices) {
		AddTopicText(string("<choice=") + std::to_string(choice.first) + ">" + choice.second + "</choice>", nChoiceNumber < choices.size() - 1 ? false : true);
		nChoiceNumber++;
	}
}

void CTopicsWindow::OnGoodbyeCalled()
{
	AddTopicText("<goodbye>Au revoir</goodbye>");
	m_bGoodbye = true;
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
			CStringUtils::Truncate(oText.second, m_nMaxCharPerLine, vLines, false);
			for (string s : vLines) {
				vRearrangedSubString.back().push_back(pair<int, string>(0, s));
			}
		}
		else {
			vRearrangedSubString.back().push_back(pair<int, string>(oText.first, oText.second));
		}
	}
}

void CTopicsWindow::ConvertTextArrayToWidgetArray(const vector<vector<pair<int, string>>>& vRearrangedSubString, vector<vector<pair<int, CGUIWidget*>>>& vTopicWidgets, int& nLineCount)
{
	nLineCount = 0;
	for (const vector<pair<int, string>>& topicLine : vRearrangedSubString) {
		vTopicWidgets.resize(vTopicWidgets.size() + 1);
		for (const pair<int, string>& elem : topicLine) {
			CGUIWidget* pTextWidget = nullptr;
			if (elem.first == -1) {
				pTextWidget = new CLink(m_oInterface, elem.second);
				CLink* pLink = static_cast<CLink*>(pTextWidget);
				pLink->SetClickedCallback(OnLinkClicked);
				nLineCount = pLink->GetLineCount();
			}
			else if (elem.first == 0)
				pTextWidget = m_pGUIManager->CreateStaticText(elem.second, nLineCount);
			else if (elem.first > 0 && elem.first < 255) {
				pTextWidget = new CTopicLink(m_oInterface, elem.second, elem.first, m_pTopicTextFrame->GetDimension().GetWidth());
				CTopicLink* pLink = static_cast<CTopicLink*>(pTextWidget);
				pLink->SetClickedCallback(OnChoiceClicked);
				nLineCount = pLink->GetLineCount();
				pLink->SetColorByState(CLink::TState::eNormal, IGUIManager::TFontColor::eRed);
				pLink->SetColorByState(CLink::TState::eHover, IGUIManager::TFontColor::eTurquoise);
				pLink->SetColorByState(CLink::TState::eClick, IGUIManager::TFontColor::eYellow);
			}
			else if (elem.first > 255) {
				pTextWidget = new CTopicLink(m_oInterface, elem.second, elem.first, m_pTopicTextFrame->GetDimension().GetWidth());
				CTopicLink* pLink = static_cast<CTopicLink*>(pTextWidget);
				pLink->SetClickedCallback(OnGoodbyeClicked);
				nLineCount = pLink->GetLineCount();
				pLink->SetColorByState(CLink::TState::eNormal, IGUIManager::TFontColor::eRed);
				pLink->SetColorByState(CLink::TState::eHover, IGUIManager::TFontColor::eTurquoise);
				pLink->SetColorByState(CLink::TState::eClick, IGUIManager::TFontColor::eYellow);
			}

			pTextWidget->m_sUserData = elem.second;
			vTopicWidgets.back().push_back(pair<int, CGUIWidget*>(elem.first, pTextWidget));
		}
	}
}

float CTopicsWindow::PutWidgetArrayIntoTopicWidget(vector<vector<pair<int, CGUIWidget*>>>& vTopicWidgets, CGUIWindow* pTopicWidget)
{
	m_pTopicTextFrame->AddWidget(pTopicWidget);
	int y = 0;
	pTopicWidget->SetRelativePosition(0, m_nTopicTextPointer * m_pGUIManager->GetCurrentFontEspacementY());
	for (vector<pair<int, CGUIWidget*>>& line : vTopicWidgets) {
		int x = 0;
		CGUIWindow* pLineWidget = new CGUIWindow(m_oInterface);
		for (pair<int, CGUIWidget*>& elem : line) {
			pLineWidget->AddWidget(elem.second);
			elem.second->SetRelativePosition(x, 0);
			x += elem.second->GetDimension().GetWidth();
		}
		pTopicWidget->AddWidget(pLineWidget);
		pLineWidget->SetRelativePosition(0, y);
		
		m_nTopicTextPointer++;
		y += m_pGUIManager->GetCurrentFontEspacementY();
	}
	return y;
}

void CTopicsWindow::AddTopicText(const string& sTopicText, bool bNewParagraph)
{
	int y = 0;
	vector<string> vTopicLines;
	Truncate(sTopicText, m_nMaxCharPerLine, vTopicLines);
	vector<vector<pair<int, string>>> vSubString;
	for (string sLine : vTopicLines)
		DecomposeTopicText(sLine, vSubString);
	vector<vector<pair<int, CGUIWidget*>>> vTopicWidgets;
	int nLineCount;
	ConvertTextArrayToWidgetArray(vSubString, vTopicWidgets, nLineCount);
	CGUIWindow* pTopicWidget = new CGUIWindow(m_oInterface);
	y = PutWidgetArrayIntoTopicWidget(vTopicWidgets, pTopicWidget);

	int nAdditionalParagraphes = nLineCount + bNewParagraph ? 1 : 0;
	for (int i = 0; i < nAdditionalParagraphes; i++) {
		int nLineCount;
		CGUIWidget* pBlank = m_pGUIManager->CreateStaticText("      ", nLineCount);
		m_pTopicTextFrame->AddWidget(pBlank);
		pBlank->SetRelativePosition(0, y + i * m_pGUIManager->GetCurrentFontEspacementY());
		m_nTopicTextPointer++;
	}

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
	CTopicsWindow* pTopicWindow = GetTopicsWindowFromLink(pLink);
	if (pTopicWindow) {
		if (!pTopicWindow->m_bGoodbye) {
			string sLinkText;
			pLink->GetText(sLinkText);
			pTopicWindow->SetCurrentTopicName(sLinkText);
			pTopicWindow->m_pTopicFrame->UpdateTopics();
			string sTopicText;
			if (sLinkText[0] >= 'a' && sLinkText[0] <= 'z') {
				sLinkText[0] += 'A' - 'a';
			}
			pTopicWindow->AddTopicTextFromTopicName(sLinkText);
		}
	}
}

void CTopicsWindow::AddTopicTextFromTopicName(string sName)
{
	string sTopicText;
	CTopicLink* pLink = m_pTopicFrame->GetTopicLink(sName);
	if (pLink) {
		sTopicText = pLink->GetTopic()->GetText();
		m_sNextTopicTextToAdd = sTopicText;
		IEventDispatcher* pEventDispatcher = static_cast<IEventDispatcher*>(m_oInterface.GetPlugin("EventDispatcher"));
		AddTopicText(sTopicText);
		string error;
		if (!m_pTopicSystem->ExecuteActions(pLink->GetTopic(), error))
			AddTopicText(error);
		if (m_bChoiceSet) {
			m_pScriptManager->ExecuteCommand("choice=0;");
			m_bChoiceSet = false;
		}
	}
	else {
		AddTopicText(string("Error : no topic found for '" + sName + "' with these conditions"));
	}
}

void CTopicsWindow::OnChoiceClicked(CLink* pTopicLink)
{
	CTopicsWindow* pTopicWindow = GetTopicsWindowFromLink(pTopicLink);
	if (!pTopicWindow->m_bGoodbye) {
		CTopicLink* pChoice = dynamic_cast<CTopicLink*>(pTopicLink);
		pTopicWindow->GetScriptManager()->ExecuteCommand("choice=" + std::to_string(pChoice->GetChoiceNumber()) + ";");
		pTopicWindow->m_pTopicFrame->UpdateTopics();
		pTopicWindow->m_bChoiceSet = true;
		if (pTopicWindow->m_sCurrentTopicName[0] > 'Z')
			pTopicWindow->m_sCurrentTopicName[0] -= 'a' - 'A';
		if(!pTopicWindow->m_sCurrentTopicName.empty())
			pTopicWindow->AddTopicTextFromTopicName(pTopicWindow->m_sCurrentTopicName);
		else {
			pTopicWindow->DisplayGreeting();
		}
		pTopicWindow->m_pScriptManager->SetVariableValue("choice", 0);
	}
}

void CTopicsWindow::OnGoodbyeClicked(CLink* pTopicLink)
{
	CTopicsWindow* pTopicWindow = GetTopicsWindowFromLink(pTopicLink);
	pTopicWindow->Close();
	pTopicWindow->m_bGoodbye = false;
}

void CTopicsWindow::RemoveTopicTexts()
{
	m_pTopicTextFrame->Clear();
}

void CTopicsWindow::SetCurrentTopicName(string sTopicName)
{
	m_sCurrentTopicName = sTopicName;
}

bool CTopicsWindow::IsGoodbye()
{
	return m_bGoodbye;
}

IValue* CTopicsWindow::GetSpeakerLocalVar(string sVarName)
{
	IEntity* pCharacter = m_oEntityManager.GetEntity(m_sSpeakerId);
	if (pCharacter) {
		IValue* pValue = pCharacter->GetLocalVariableValue(sVarName);
		return pValue;
	}
	return nullptr;
}

void CTopicsWindow::SetSpeakerLocalVar(string sLocalVar, string sValue)
{
	IEntity* pCharacter = m_oEntityManager.GetEntity(m_sSpeakerId);
	if (pCharacter) {
		if (CStringUtils::IsInteger(sValue)) {
			int nValue = atoi(sValue.c_str());
			pCharacter->SetLocalVariableValue(sLocalVar, nValue);
		}
		else if (CStringUtils::IsFloat(sValue)) {
			float fValue = atof(sValue.c_str());
			pCharacter->SetLocalVariableValue(sLocalVar, fValue);
		}
		else
			pCharacter->GetLocalVariableValue(sLocalVar, sValue);
	}
}

void CTopicsWindow::SetSpeakerLocalVar(string sLocalVar, int nValue)
{
	IEntity* pCharacter = m_oEntityManager.GetEntity(m_sSpeakerId);
	if (pCharacter)
		pCharacter->SetLocalVariableValue(sLocalVar, nValue);
	else
		throw CEException("Error in CTopicsWindow::SetSpeakerLocalVar() : character '" + m_sSpeakerId + "' not found");
}

const string& CTopicsWindow::GetSpeakerID()
{
	return m_sSpeakerId;
}

void CTopicsWindow::SetSpeakerId(string sId)
{
	m_sSpeakerId = sId;
}

string CTopicsWindow::GetSpeakerId()
{
	return m_sSpeakerId;
}

CTopicFrame::CTopicFrame(EEInterface& oInterface, int width, int height, IEntityManager& oEntityManager) :
	CGUIWindow("Gui/topic-frame.bmp", oInterface, width, height),
	m_oInterface(oInterface),
	m_pGUIManager(nullptr),
	m_nXTextMargin(5),
	m_nYTextmargin(5),
	m_nYmargin(21),
	m_nTextHeight(25),
	m_nTopicBorderWidth(218),
	//m_mTopics(mTopics),
	m_oEntityManager(oEntityManager)
{
	m_mFontColorFromTopicState[eNormal] = IGUIManager::eWhite;
	m_mFontColorFromTopicState[ePressed] = IGUIManager::eTurquoise;
	m_oInterface.HandlePluginCreation("GUIManager",	[this](CPlugin* plugin)
	{
		m_pGUIManager = static_cast<CGUIManager*>(plugin);
	});

	m_oInterface.HandlePluginCreation("TopicSystem", [this](CPlugin* plugin) {
		m_pTopicSystem = dynamic_cast<ITopicSystem*>(plugin);
	});

	CTopicsWindow* pTopicsWindow = GetParent();
}

CTopicFrame::~CTopicFrame()
{

}

CTopicsWindow* CTopicFrame::GetParent()
{
	return (CTopicsWindow*)m_pParent;
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



void CTopicFrame::UpdateTopics()
{
	int iLine = 0;
	Clear();

	vector<ITopic*> vTopics;
	m_pTopicSystem->GetCharacterTopics(GetParent()->GetSpeakerId(), vTopics);
	for (ITopic* pTopic : vTopics) {
		int iLine = GetWidgetCount();
		int y = iLine * m_nTextHeight + m_nYTextmargin;
		
		CTopicLink* pTitle = new CTopicLink(m_oInterface, pTopic->GetName());
		pTitle->SetTopicInfos(pTopic);
		pTitle->SetClickedCallback(OnClickTopic);
		AddWidget(pTitle);
		pTitle->SetRelativePosition(m_nXTextMargin, iLine * m_nTextHeight + m_nYTextmargin);
		pTitle->SetColorByState(CLink::TState::eNormal, IGUIManager::TFontColor::eWhite);
		pTitle->SetColorByState(CLink::TState::eHover, IGUIManager::TFontColor::eYellow);
		pTitle->SetColorByState(CLink::TState::eClick, IGUIManager::TFontColor::eTurquoise);
		iLine++;
	}
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
	if (!pTopicWindow->IsGoodbye()) {
		pTopicWindow->SetCurrentTopicName(pTitle->GetTopic()->GetName());
		pTopicWindow->AddTopicText(pTitle->GetTopic()->GetText());
		string error;
		//if(!pTitle->GetTopic()->ExecuteActions(GetParent()->GetScriptManager(), error))
		if(!m_pTopicSystem->ExecuteActions(pTitle->GetTopic(), error))
			pTopicWindow->AddTopicText(error);
		UpdateTopics();
	}
}

int CTopicFrame::ConvertValueToInt(string sValue)
{
	return atoi(sValue.c_str());
}


ITopic*	CTopicLink::GetTopic()
{
	return m_pTopic;
}

void CTopicLink::SetTopicInfos(ITopic* pTopic)
{
	m_pTopic = pTopic;
}

int CTopicLink::GetChoiceNumber()
{
	return m_nChoiceNumber;
}
