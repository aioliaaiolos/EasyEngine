#pragma once
#include "GUIWindow.h"

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
	void									AddTopic(string sTopicName, string sText, vector<CCondition>& conditions);
	void									Display();
	void									SetSpeakerId(string sId) override;
	void									AddTopicText(const string& sTopicText);
	void									RemoveTopicTexts();

private:

	void									LoadTopics(string sFileName);
	void									DecodeString(string& sIn, string& sOut);
	static									void OnGUIManagerCreated(CPlugin* pGUIManager, void* pData);
	void									OnShow(bool bShow) override;
	void									DestroyTopicsWidgets();

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
	void										Display();
	void										AddTopic(string sTopicName, string sText, vector<CCondition>& conditions);
	void										SetParent(CGUIWidget* parent);
	int											GetTextHeight();
	void										SetSpeakerId(string sId);
	void										CreateTopicsWidgets();
	void										DestroyTopicsWidgets();

private:

	CTopicsWindow*								GetParent();
	int											GetTopicIndexFromY(int y);
	void										OnItemSelected(CGUIWidget* pTitle);
	void										OnItemHover(CGUIWidget* pTitle);
	int											SelectTopic(vector<CTopicInfo>& topics, string sSpeakerId);
	int											IsConditionChecked(vector<CTopicInfo>& topics, string sSpeakerId);
	static int									ConvertValueToInt(string sValue);
	static void									OnGUIManagerCreated(CPlugin* plugin, void* pData);
	static void									OnScriptManagerCreated(CPlugin* plugin, void* pData);
	static void									OnTopicEvent(IGUIManager::ENUM_EVENT nEvent, CGUIWidget* pWidget, int x, int y);
	static void									Format(string sTopicText, string sSpeakerId, string& sFormatedText);
	static void									GetVarValue(string sVarName, string sCharacterId, string& sValue);

	EEInterface&											m_oInterface;
	CGUIManager*											m_pGUIManager;
	IScriptManager*											m_pScriptManager;
	const int												m_nXTextMargin;
	const int												m_nYTextmargin;
	const int												m_nYmargin;
	const int												m_nTextHeight;
	map<string, vector<CTopicInfo>>							m_mTopics;
	map<string, vector<CTopicInfo>>							m_mDisplayedTopics;
	map<CGUIWidget*, string>								m_mDisplayedTopicWidgets;
	map<string, TTopicState>								m_mTopicsState;
	const int												m_nTopicBorderWidth;
	map<TTopicState, IGUIManager::TFontColor>				m_mFontColorFromTopicState;
	string													m_sSpeakerId;
};


