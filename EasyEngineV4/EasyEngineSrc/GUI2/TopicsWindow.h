#pragma once
#include "GUIWindow.h"

// rapidjson
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/filereadstream.h"
#include <fstream>


class CGUIManager;
class CTopicFrame;
class IScriptManager;


struct CCondition
{
	enum TComp
	{
		eEqual = 0,
		eDifferent,
		eSup,
		eInf,
		eIs,
		eIsNot
	};

	string	m_sVariableName;
	string	m_sValue;
	TComp	m_eComp;
};

struct CTopicInfo
{
	string m_sText;
	vector<CCondition> m_vConditions;
	vector<string> m_vAction;

	CTopicInfo() {}
};


struct CTopicInfoWidgets : public CGUIWidget
{
	CTopicInfoWidgets(int nWidth, int nHeight);

	CGUIWidget* m_pTitle;
	CGUIWidget*	m_pText;
};

class CTopicsWindow : public CGUIWindow, public ITopicWindow
{
public:
	CTopicsWindow(EEInterface& oInterface, int width, int height);
	virtual ~CTopicsWindow();
	void									AddTopic(string sTopicName, string sText, const vector<CCondition>& vConditions, const vector<string>& vAction);
	void									AddGreating(string sText, vector<CCondition>& conditions);
	void									AddTopicText(const string& sTopicText);
	void									Display();
	void									SetSpeakerId(string sId) override;
	string									GetSpeakerId();
	void									RemoveTopicTexts();
	IScriptManager*							GetScriptManager();
	int										SelectTopic(const vector<CTopicInfo>& topics, string sSpeakerId);

private:

	void									LoadTopics(string sFileName);
	void									LoadJsonConditions(rapidjson::Value& oParentNode, vector<CCondition>& vConditions, string sFileName);
	void									DecodeString(string& sIn, string& sOut);
	void									OnShow(bool bShow) override;
	void									DestroyTopicsWidgets();
	int										GetTopicTextLineCount();
	static void								OnLinkClicked(CLink* pLink);
	static									void OnGUIManagerCreated(CPlugin* pGUIManager, IObject* pData);
	static void								OnAddTopic(CPlugin* pPlugin, IEventDispatcher::TWindowEvent e, int, int);	
	static void								OnScriptManagerCreated(CPlugin* plugin, IObject* pData);

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
	vector<CTopicInfo>						m_vGreatings;
	IScriptManager*							m_pScriptManager;
	string									m_sSpeakerId;
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

	CTopicFrame(EEInterface& oInterface, int width, int height);
	virtual ~CTopicFrame();
	void										Display();
	void										AddTopic(string sTopicName, string sText, const vector<CCondition>& conditions, const vector<string>& vAction);
	void										GetTopicText(string sTopicTitle, string& sTopicText);
	void										SetParent(CGUIWidget* parent);
	int											GetTextHeight();
	void										AddTopicsToWindow();
	void										AddTopicToWindow(const pair<string, vector<CTopicInfo>>& topic, string sSpeakerId);
	void										DestroyTopicsWidgets();

private:

	CTopicsWindow*								GetParent();
	int											GetTopicIndexFromY(int y);
	void										OnItemSelected(CLink* pTitle);
	void										OnItemHover(CGUIWidget* pTitle);
	int											IsConditionChecked(const vector<CTopicInfo>& topics, string sSpeakerId);
	static int									ConvertValueToInt(string sValue);
	static void									OnGUIManagerCreated(CPlugin* plugin, IObject* pData);
	static void									OnClickTopic(CLink* pLink);
	static void									Format(string sTopicText, string sSpeakerId, string& sFormatedText);
	static void									GetVarValue(string sVarName, string sCharacterId, string& sValue);

	EEInterface&											m_oInterface;
	CGUIManager*											m_pGUIManager;
	//IScriptManager*											m_pScriptManager;
	const int												m_nXTextMargin;
	const int												m_nYTextmargin;
	const int												m_nYmargin;
	const int												m_nTextHeight;
	map<string, vector<CTopicInfo>>							m_mTopics;
	map<CLink*, string>										m_mDisplayedTopicWidgets;
	map<string, TTopicState>								m_mTopicsState;
	const int												m_nTopicBorderWidth;
	map<TTopicState, IGUIManager::TFontColor>				m_mFontColorFromTopicState;
};

class CTopicLink : public CLink
{
public:
	string m_sTitle;
	vector<CCondition> m_vCondition;
	vector<string>	m_vAction;
};