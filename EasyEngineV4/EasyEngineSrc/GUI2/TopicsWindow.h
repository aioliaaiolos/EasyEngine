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
class CTopicsWindow;
class IPlayer;

struct CCondition
{
	enum TComp
	{
		eEqual = 0,
		eDifferent,
		eSup,
		eSupEqual,
		eInf,
		eInfEqual,
		eIs,
		eIsNot
	};

	string	m_sType;
	string	m_sName;
	string	m_sValue;
	TComp	m_eComp;
	bool Evaluate(int val) const
	{
		int value = atoi(m_sValue.c_str());
		switch (m_eComp) {
		case eEqual:
			return val == value;
			break;
		case eSup:
			return val > value;
			break;
		case eSupEqual:
			return val >= value;
			break;
		case eInf:
			return val < value;
			break;
		case eInfEqual:
			return val <= value;
			break;
		case eDifferent:
			return val != value;
			break;
		default:
			throw CEException("Error in CCondition::Evaluate() : Comparaison operator not managed.");
			break;
		}
	}
	bool Evaluate(float val) const
	{
		float value = atof(m_sValue.c_str());
		switch (m_eComp) {
		case eEqual:
			return val == value;
			break;
		case eSup:
			return val > value;
			break;
		case eSupEqual:
			return val >= value;
			break;
		case eInf:
			return val < value;
			break;
		case eInfEqual:
			return val <= value;
			break;
		case eDifferent:
			return val != value;
			break;
		default:
			throw CEException("Error in CCondition::Evaluate() : Comparaison operator not managed.");
			break;
		}
	}
	bool Evaluate(string val) const
	{
		m_sValue;
		switch (m_eComp) {
		case eEqual:
			return val == m_sValue;
			break;
		case eSup:
			return val > m_sValue;
			break;
		case eInf:
			return val < m_sValue;
			break;
		case eDifferent:
			return val != m_sValue;
			break;
		default:
			throw CEException("Error in CCondition::Evaluate() : Comparaison operator not managed.");
			break;
		}
	}
};

struct CTopicInfo
{
	CTopicInfo();
	CTopicInfo(const string& sText, const vector<CCondition>& conditions, const vector<string>& actions);
	void ExecuteActions(IScriptManager* pScriptManager, CTopicsWindow* pTopicsWindow);

	string m_sName;
	string m_sText;
	vector<CCondition> m_vConditions;
	vector<string> m_vAction;
};

class CTopicLink : public CLink
{
public:
	
	CTopicLink(EEInterface& oInterface, string sName, int nMaxChar = -1) : 
		CLink(oInterface, sName, nMaxChar), m_nChoiceNumber(-1)
	{
		m_oTopicInfos.m_sName = sName;
	}

	
	CTopicLink(EEInterface& oInterface, string sName, int nChoiceNumber, int nMaxWidth) :
		CLink(oInterface, sName, nMaxWidth),
		m_nChoiceNumber(nChoiceNumber)
	{
		m_oTopicInfos.m_sName = sName;
	}

	CTopicInfo&	GetTopicInfos();
	void		SetTopicInfos(const CTopicInfo& oTopicInfos);
	int			GetChoiceNumber();

private:
	CTopicInfo	m_oTopicInfos;
	int			m_nChoiceNumber;
};


class CTopicsWindow : public CGUIWindow, public ITopicWindow
{
public:
	CTopicsWindow(EEInterface& oInterface, int width, int height);
	virtual ~CTopicsWindow();
	void									AddTopic(string sTopicName, string sText, const vector<CCondition>& vConditions, const vector<string>& vAction);
	void									AddGreating(string sText, vector<CCondition>& conditions, vector<string>& actions);
	void									AddTopicText(const string& sTopicText, bool bNewParagraph = true);
	void									Display();
	void									SetSpeakerId(string sId) override;
	string									GetSpeakerId();
	void									RemoveTopicTexts();
	IScriptManager*							GetScriptManager();
	int										SelectTopic(const vector<CTopicInfo>& topics, string sSpeakerId);
	int										SelectTopic(string sTopicName, string sSpeakerId);
	void									SetCurrentTopicName(string sTopicName);
	bool									IsGoodbye();
	IValue*									GetSpeakerLocalVar(string sVarName) override;
	void									SetSpeakerLocalVar(string sLocalVar, string sValue) override;
	void									SetSpeakerLocalVar(string sLocalVar, int nValue) override;
	const string&							GetSpeakerID() override;

private:

	void									LoadTopics(string sFileName);
	void									LoadJsonActions(rapidjson::Value& oParentNode, vector<string>& vAction);
	void									LoadJsonConditions(rapidjson::Value& oParentNode, vector<CCondition>& vConditions, string sFileName);
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
	vector<CTopicInfo>						m_vGreatings;
	IScriptManager*							m_pScriptManager;
	string									m_sSpeakerId;
	string									m_sCurrentTopicName;
	map<string, vector<CTopicInfo>>			m_mTopics;
	bool									m_bChoiceSet;
	IEntityManager&							m_oEntityManager;
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

	CTopicFrame(EEInterface& oInterface, int width, int height, const map<string, vector<CTopicInfo>>& mTopics, IEntityManager& oEntityManager);
	virtual ~CTopicFrame();
	void										Display();
	CTopicLink*									GetTopicLink(string sTopicTitle);
	void										SetParent(CGUIWidget* parent);
	int											GetTextHeight();
	void										UpdateTopics();
	int											AddTopicToWindow(const pair<string, vector<CTopicInfo>>& topic, string sSpeakerId);
	void										DestroyTopicsWidgets();

private:

	CTopicsWindow*								GetParent();
	int											GetTopicIndexFromY(int y);
	void										OnItemSelected(CTopicLink* pTitle);
	int											IsConditionChecked(const vector<CTopicInfo>& topics, string sSpeakerId);
	static int									ConvertValueToInt(string sValue);
	static void									OnClickTopic(CLink* pLink);
	static void									Format(string sTopicText, string sSpeakerId, string& sFormatedText);
	static void									GetVarValue(string sVarName, string sCharacterId, string& sValue);

	EEInterface&											m_oInterface;
	CGUIManager*											m_pGUIManager;
	const int												m_nXTextMargin;
	const int												m_nYTextmargin;
	const int												m_nYmargin;
	const int												m_nTextHeight;
	map<string, TTopicState>								m_mTopicsState;
	const int												m_nTopicBorderWidth;
	map<TTopicState, IGUIManager::TFontColor>				m_mFontColorFromTopicState;
	const map<string, vector<CTopicInfo>>&					m_mTopics;
	IEntityManager&											m_oEntityManager;
};

