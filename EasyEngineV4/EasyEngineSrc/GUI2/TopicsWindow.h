#pragma once
#include "GUIWindow.h"


class CGUIManager;
class CTopicFrame;
class IScriptManager;
class CTopicsWindow;
class IPlayer;
class ITopicSystem;
class ITopic;

class CTopicLink : public CLink
{
public:
	
	CTopicLink(EEInterface& oInterface, string sName, int nMaxChar = -1) : 
		CLink(oInterface, sName, nMaxChar), m_nChoiceNumber(-1)
	{
		//m_pTopic.m_sName = sName;
	}

	
	CTopicLink(EEInterface& oInterface, string sName, int nChoiceNumber, int nMaxWidth) :
		CLink(oInterface, sName, nMaxWidth),
		m_nChoiceNumber(nChoiceNumber)
	{
		//m_pTopic->SetName(sName);
	}

	ITopic*		GetTopic();
	void		SetTopicInfos(ITopic* pTopic);
	int			GetChoiceNumber();

private:
	ITopic*		m_pTopic;
	int			m_nChoiceNumber;
};


class CTopicsWindow : public CGUIWindow, public ITopicWindow
{
public:
	CTopicsWindow(EEInterface& oInterface, int width, int height);
	virtual ~CTopicsWindow();
	//void									AddTopic(string sTopicName, string sText, const vector<CCondition>& vConditions, const vector<string>& vAction);
	//void									AddGreating(string sText, vector<CCondition>& conditions, vector<string>& actions);
	void									AddTopicText(const string& sTopicText, bool bNewParagraph = true);
	void									Display();
	void									SetSpeakerId(string sId) override;
	string									GetSpeakerId();
	void									RemoveTopicTexts();
	IScriptManager*							GetScriptManager();
	void									SetCurrentTopicName(string sTopicName);
	bool									IsGoodbye();
	IValue*									GetSpeakerLocalVar(string sVarName) override;
	void									SetSpeakerLocalVar(string sLocalVar, string sValue) override;
	void									SetSpeakerLocalVar(string sLocalVar, int nValue) override;
	const string&							GetSpeakerID() override;

private:

	void									OnShow(bool bShow) override;
	void									DisplayGreating();
	void									DestroyTopicsWidgets();
	int										GetTopicTextLineCount();
	void									OnChoiceCalled(string sChoices) override;
	void									OnGoodbyeCalled() override;
	void									AddTopicWidgets();
	void									Truncate(string sText, int nMaxCharPerLine, vector<string>& output);
	void									DecomposeTopicText(string sTopicText, vector<vector<pair<int, string>>>& vRearangedSubString);
	void									ConvertTextArrayToWidgetArray(const vector<vector<pair<int, string>>>& vRearrangedSubString, vector<vector<pair<int, CGUIWidget*>>>& vTopicWidgets, int& nLineCount);
	float									PutWidgetArrayIntoTopicWidget(vector<vector<pair<int, CGUIWidget*>>>& vTopicWidgets, CGUIWindow* pTopicWidget);
	void									AddTopicTextFromTopicName(string sName);
	static CTopicsWindow*					GetTopicsWindowFromLink(CLink* pLink);
	static void								OnLinkClicked(CLink* pLink);
	static void								OnChoiceClicked(CLink* pTopicLink);
	static void								OnGoodbyeClicked(CLink* pTopicLink);

	EEInterface&							m_oInterface;
	IRenderer&								m_oRenderer;
	IRessourceManager&						m_oRessourceManager;
	IFileSystem&							m_oFileSystem;
	CTopicFrame*							m_pTopicFrame;
	CGUIWindow*								m_pTopicTextFrame;
	CGUIManager*							m_pGUIManager;
	string									m_sText;
	const int								m_nMaxCharPerLine;
	int										m_nTopicTextPointer;
	string									m_sNextTopicTextToAdd;
	IScriptManager*							m_pScriptManager;
	string									m_sSpeakerId;
	string									m_sCurrentTopicName;
	bool									m_bChoiceSet;
	IEntityManager&							m_oEntityManager;
	ITopicSystem*							m_pTopicSystem = nullptr;
	bool									m_bGoodbye;
};

class CTopicFrame : public CGUIWindow
{
public:
	enum TTopicState {
		eNormal = 0,
		eHover,
		ePressed,
		eReleased
	};

	CTopicFrame(EEInterface& oInterface, int width, int height, IEntityManager& oEntityManager);
	virtual ~CTopicFrame();
	void										Display();
	CTopicLink*									GetTopicLink(string sTopicTitle);
	void										SetParent(CGUIWidget* parent);
	int											GetTextHeight();
	void										UpdateTopics();
	int											AddTopicToWindow(const pair<string, vector<ITopic*>>& topic, string sSpeakerId);
	void										DestroyTopicsWidgets();

private:

	CTopicsWindow*								GetParent();
	int											GetTopicIndexFromY(int y);
	void										OnItemSelected(CTopicLink* pTitle);
	static int									ConvertValueToInt(string sValue);
	static void									OnClickTopic(CLink* pLink);
	//static void									Format(string sTopicText, string sSpeakerId, string& sFormatedText);
	//static void									GetVarValue(string sVarName, string sCharacterId, string& sValue);

	EEInterface&											m_oInterface;
	CGUIManager*											m_pGUIManager;
	const int												m_nXTextMargin;
	const int												m_nYTextmargin;
	const int												m_nYmargin;
	const int												m_nTextHeight;
	map<string, TTopicState>								m_mTopicsState;
	const int												m_nTopicBorderWidth;
	map<TTopicState, IGUIManager::TFontColor>				m_mFontColorFromTopicState;
	IEntityManager&											m_oEntityManager;
	ITopicSystem*											m_pTopicSystem;
};

