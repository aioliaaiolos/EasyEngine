#include <string>
#include <vector>
#include "ITopicSystem.h"
#include "Exception.h"

// rapidjson
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/filereadstream.h"
#include <fstream>


class IEntityManager;
class IScriptManager;
class IFileSystem;

struct CCondition : public ICondition
{
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

	string GetType() override;
	string GetName() override;
	string GetValue() override;
	TComp GetComp() override;
	string GetCompStr() override;
	TComp GetCompFromString(string comp);
	int GetNum() override;

	void SetType(const string& type) override;
	void SetName(const string& name) override;
	void SetValue(const string& value) override;
	void SetComp(TComp comp) override;
	void SetComp(string comp) override;

	string	m_sType;
	string	m_sName;
	string	m_sValue;
	TComp	m_eComp;
	int m_nNum = 0;
};

class CTopic : public ITopic
{
public:
	CTopic();
	CTopic(const string& sText, const map<int, ICondition*>& conditions, const vector<string>& actions);
	string& GetName() override;
	void SetName(const string& sName) override;
	void SetText(const string& sText) override;
	const string& GetText() const override;	
	vector<string>& GetActions() override;
	void SetActions(const vector<string>& actions);
	map<int, ICondition*>& GetConditions() override;
	ICondition* GetCondition(int index) override;
	ICondition* AddCondition(int conditionIndex) override;
	void SetConditions(map<int, ICondition*>& conditions);
	void SetActions(vector<string>& actions) override;
	

private:
	map<int, ICondition*> m_mConditions;
	vector<string> m_vAction;
	string m_sName;
	string m_sText;
};

class CTopicSystem : public ITopicSystem
{
public:


	CTopicSystem(EEInterface& oInterface);

protected:
	void										LoadTopics(string sFileName);
	void										SaveTopics(const string& sFileName) override;
	void										AddTopic(string sTitleName, string topicName) override;
	void										AddGreetingTopic(string sTitleName, string sTopicName) override;
	void										DeleteTitle(string sTitleName) override;
	void										DeleteTitleGreeting(string sTitleName) override;
	void										LoadJsonConditions(rapidjson::Value& oParentNode, map<int, ICondition*>& vConditions);
	void										LoadJsonCondition(rapidjson::Value& condition, CCondition* pCondition);
	int											SelectTopic(const vector<ITopic*>& topics, string sSpeakerId);
	ITopic*										SelectGreeting(string sSpeakerId) override;
	const ITopic*								SelectTopic(string sTopicName, string sSpeakerId);
	void										LoadJsonActions(rapidjson::Value& oParentNode, vector<string>& vAction);
	int											IsConditionChecked(const vector<ITopic*>& topics, string sSpeakerId);
	string										GetName() override;
	void										GetCharacterTopics(string sCharacterID, vector<ITopic*>& topics) override;
	map<string, vector<ITopic*>>&				GetAllTopics() override;
	map<string, vector<ITopic*>>&				GetAllGreetings() override;
	void										Format(string sTopicText, string sSpeakerId, string& sFormatedText);
	void										GetVarValue(string sVarName, string sCharacterId, string& sValue);
	bool										ExecuteActions(ITopic* pTopic, string& error) const;
	ITopic*										GetTopic(string sTitle, string sText) override;
	ITopic*										GetGreeting(string sTitle, string sText) override;

private:

	string										LoadTopic(rapidjson::Value& topic, CTopic* pTopic);

	void										SaveJsonTopics(string title, vector<ITopic*>& vTopic, rapidjson::Value& topics, rapidjson::Document& doc);
	void										SaveJsonTopic(string titleStr, ITopic* pTopic, rapidjson::Document& doc, rapidjson::Value& topic);
	void										SaveJsonConditions(const map<int, ICondition*>& conditionArray, rapidjson::Document& doc, rapidjson::Value& conditions);
	void										SaveJsonActions(const vector<string> actionArray, rapidjson::Document& doc, rapidjson::Value& actions);

	map<string, vector<ITopic*>>				m_mTopics;
	map<string, vector<ITopic*>>				m_mGreetings;
	IEntityManager*								m_pEntityManager = nullptr;
	IFileSystem*								m_pFileSystem = nullptr;
	IScriptManager*								m_pScriptManager = nullptr;
};

extern "C" _declspec(dllexport) ITopicSystem* CreateTopicSystem(EEInterface& oInterface);