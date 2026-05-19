#pragma once

#include "EEPlugin.h"

class IScriptManager;

struct ICondition
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

	virtual string GetType() = 0;
	virtual string GetName() = 0;
	virtual string GetValue() = 0;
	virtual TComp GetComp() = 0;
	virtual string GetCompStr() = 0;
};

class ITopic
{
public:
	virtual string& GetName() = 0;
	virtual const string& GetText() const = 0;
	virtual vector<string>& GetActions() = 0;
	virtual vector<ICondition*>& GetConditions() = 0;
	virtual void SetName(const string& sName) = 0;
	virtual void SetText(const string& sText) = 0;
};

class ITopicSystem : public CPlugin
{
public:
	ITopicSystem(EEInterface& oInterface) : CPlugin(nullptr, "TopicSystem") {}
	virtual const ITopic*	SelectTopic(string sTopicName, string sSpeakerId) = 0;
	virtual ITopic*	SelectGreeting(string sSpeakerId) = 0;
	virtual void GetCharacterTopics(string sCharacterID, vector<ITopic*>& topics) = 0;
	virtual map<string, vector<ITopic*>>& GetAllTopics() = 0;
	virtual map<string, vector<ITopic*>>& GetAllGreetings() = 0;
	virtual bool ExecuteActions(ITopic* pTopic, string& error) const = 0;
	virtual void SaveTopics(const string& sFileName) = 0;
	virtual void AddTopic(string sTitleName, string topicName) = 0;
	virtual void AddGreetingTopic(string sTitleName, string sTopicName) = 0;
	virtual void DeleteTitle(string sTitleName) = 0;
	virtual void DeleteTitleGreeting(string sTitleName) = 0;
};