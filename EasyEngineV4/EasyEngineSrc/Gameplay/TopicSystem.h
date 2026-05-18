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

	string	m_sType;
	string	m_sName;
	string	m_sValue;
	TComp	m_eComp;
};

class CTopic : public ITopic
{
public:
	CTopic();
	CTopic(const string& sText, const vector<ICondition*>& conditions, const vector<string>& actions);
	string& GetName() override;
	void SetName(string& sName);
	void SetText(string& sText);
	const string& GetText() const override;	
	vector<string>& GetActions() override;
	void SetActions(const vector<string>& actions);
	vector<ICondition*>& GetConditions() override;
	void SetConditions(vector<ICondition*>& conditions);
	

private:
	vector<ICondition*> m_vConditions;
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
	void										SaveTopics(const string& sFileName, map<string, vector<ITopic*>>& mTopics, vector<ITopic*>& vGreatings) override;
	void										AddTopic(string sTopicName, string sText, vector<ICondition*>& conditions, const vector<string>& vAction) override;
	void										AddGreating(string sText, vector<ICondition*>& conditions, vector<string>& actions);
	void										LoadJsonConditions(rapidjson::Value& oParentNode, vector<ICondition*>& vConditions, string sFileName);
	int											SelectTopic(const vector<ITopic*>& topics, string sSpeakerId);
	ITopic*										SelectGreating(string sSpeakerId) override;
	const ITopic*								SelectTopic(string sTopicName, string sSpeakerId) override;
	void										LoadJsonActions(rapidjson::Value& oParentNode, vector<string>& vAction);
	int											IsConditionChecked(const vector<ITopic*>& topics, string sSpeakerId);
	string										GetName() override;
	void										GetCharacterTopics(string sCharacterID, vector<ITopic*>& topics) override;
	//map<string, vector<CTopic>>&	GetAllTopics() override;
	map<string, vector<ITopic*>>&				GetAllTopics() override;
	vector<ITopic*>&							GetAllGreatings() override;
	void										Format(string sTopicText, string sSpeakerId, string& sFormatedText);
	void										GetVarValue(string sVarName, string sCharacterId, string& sValue);
	bool										ExecuteActions(ITopic* pTopic, string& error) const;

private:

	void										SaveJsonTopics(string title, vector<ITopic*>& vTopic, rapidjson::Value& topics, rapidjson::Document& doc);
	void										SaveJsonTopic(string titleStr, ITopic* pTopic, rapidjson::Document& doc, rapidjson::Value& topic);
	void										SaveJsonConditions(const vector<ICondition*>& conditionArray, rapidjson::Document& doc, rapidjson::Value& conditions);
	void										SaveJsonActions(const vector<string> actionArray, rapidjson::Document& doc, rapidjson::Value& actions);

	map<string, vector<ITopic*>>				m_mTopics;
	vector<ITopic*>								m_vGreatings;
	IEntityManager*								m_pEntityManager = nullptr;
	IFileSystem*								m_pFileSystem = nullptr;
	IScriptManager*								m_pScriptManager = nullptr;
};

extern "C" _declspec(dllexport) ITopicSystem* CreateTopicSystem(EEInterface& oInterface);