#pragma once

#include "EEPlugin.h"

class IScriptManager;

struct ICondition
{
	virtual string GetType() = 0;
	virtual string GetName() = 0;
	virtual string GetValue() = 0;
};

class ITopic
{
public:
	virtual string& GetName() = 0;
	virtual const string& GetText() const = 0;
	virtual vector<string>& GetActions() = 0;
	virtual vector<ICondition*>& GetConditions() = 0;
};

class ITopicSystem : public CPlugin
{
public:
	ITopicSystem(EEInterface& oInterface) : CPlugin(nullptr, "TopicSystem") {}
	virtual const ITopic*	SelectTopic(string sTopicName, string sSpeakerId) = 0;
	virtual ITopic*	SelectGreating(string sSpeakerId) = 0;
	virtual void GetCharacterTopics(string sCharacterID, vector<ITopic*>& topics) = 0;
	virtual map<string, vector<ITopic*>>& GetAllTopics() = 0;
	virtual bool ExecuteActions(ITopic* pTopic, string& error) const = 0;
};