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

class CTopicsWindow : public CGUIWindow, public ITopicWindow
{
public:
	CTopicsWindow(EEInterface& oInterface, int width, int height);
	virtual ~CTopicsWindow();
	void									AddTopic(string sTopicName, string sText, vector<CCondition>& conditions);
	void									Display();
	void									DisplayTopicInfos(string sTopic);
	void									SetSpeakerId(string sId) override;

private:

	void									LoadTopics(string sFileName);
	void									DecodeString(string& sIn, string& sOut);
	static									void OnGUIManagerCreated(CPlugin* pGUIManager, void* pData);
	void									OnShow(bool bShow) override;

	IRenderer&								m_oRenderer;
	IRessourceManager&						m_oRessourceManager;
	IFileSystem&							m_oFileSystem;
	CTopicFrame*							m_pTopicFrame;
	CGUIManager*							m_pGUIManager;
	string									m_sText;
	const int								m_nMaxCharPerLine;
};

class CTopicFrame : public CGUIWidget
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

private:

	CTopicsWindow*								GetParent();
	int											GetTopicIndexFromY(int y);
	void										OnItemSelected(int itemIndex);
	void										OnItemRelease(int itemIndex);
	void										OnItemHover(int itemIndex);
	int											SelectTopic(vector<CTopicInfo>& topics, string sSpeakerId);
	int											IsConditionChecked(vector<CTopicInfo>& topics, string sSpeakerId);
	static int									ConvertValueToInt(string sValue);
	static void									OnEventCallback(IGUIManager::ENUM_EVENT nEvent, CGUIWidget* pWidget, int x, int y);
	static void									OnGUIManagerCreated(CPlugin* plugin, void* pData);
	static void									OnScriptManagerCreated(CPlugin* plugin, void* pData);

	EEInterface&								m_oInterface;
	IGUIManager*								m_pGUIManager;
	IScriptManager*								m_pScriptManager;
	const int									m_nXTextMargin;
	const int									m_nYTextmargin;
	const int									m_nYmargin;
	const int									m_nTextHeight;
	map<string, vector<CTopicInfo>>				m_mTopics;
	map<string, vector<CTopicInfo>>				m_mDisplayedTopics;
	map<string, TTopicState>					m_mTopicsState;
	const int									m_nTopicBorderWidth;
	map<TTopicState, IGUIManager::TFontColor>	m_mFontColorFromTopicState;
	string										m_sSpeakerId;
};


