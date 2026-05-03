#include "TopicSystem.h"
#include "Interface.h"
#include "IEntity.h"
#include "IScriptManager.h"
#include "Utils2/StringUtils.h"


using namespace rapidjson;



string CCondition::GetType()
{
	return m_sType;
}

string CCondition::GetName()
{
	return m_sName;
}

string CCondition::GetValue()
{
	return m_sValue;
}

CCondition::TComp CCondition::GetComp()
{
	return m_eComp;
}



CTopic::CTopic()
{
}

CTopic::CTopic(const string& sText, const vector<ICondition*>& conditions, const vector<string>& actions) :
	m_sText(sText),
	m_vConditions(conditions),
	m_vAction(actions)
{
}

string& CTopic::GetName()
{
	return m_sName;
}

void CTopic::SetName(string& sName)
{
	m_sName = sName;
}

const string& CTopic::GetText() const
{
	return m_sText;
}

void CTopic::SetText(string& sText)
{
	m_sText = sText;
}

vector<string>& CTopic::GetActions()
{
	return m_vAction;
}

void CTopic::SetActions(const vector<string>& actions)
{
	m_vAction = actions;
}

vector<ICondition*>& CTopic::GetConditions()
{
	return m_vConditions;
}

void CTopic::SetConditions(vector<ICondition*>& conditions)
{
	m_vConditions = conditions;
}

CTopicSystem::CTopicSystem(EEInterface& oInterface) : ITopicSystem(oInterface)
{
	oInterface.HandlePluginCreation("EntityManager", [this](CPlugin* pPlugin) {
		m_pEntityManager = dynamic_cast<IEntityManager*>(pPlugin);
	});

	oInterface.HandlePluginCreation("FileSystem", [this](CPlugin* pPlugin) {
		m_pFileSystem = dynamic_cast<IFileSystem*>(pPlugin);
		LoadTopics("topics.json");
	});

	oInterface.HandlePluginCreation("ScriptManager", [this](CPlugin* pPlugin) {
		m_pScriptManager = dynamic_cast<IScriptManager*>(pPlugin);
	});
}

bool CTopicSystem::ExecuteActions(ITopic* pTopic, string& error) const
{
	try {
		for (const string& sAction : pTopic->GetActions())
			m_pScriptManager->ExecuteCommand(sAction + ";");
	}
	catch (CEException& e) {
		string sErrorMesage;
		e.GetErrorMessage(sErrorMesage);
		error = string("Script error : ") + sErrorMesage;
		return false;
	}
	return true;
}

void CTopicSystem::LoadTopics(string sFileName)
{
	m_mTopics.clear();
	FILE* pFile = m_pFileSystem->OpenFile(sFileName, "r");
	fclose(pFile);
	string sJsonDirectory;
	m_pFileSystem->GetLastDirectory(sJsonDirectory);
	string sFilePath = sJsonDirectory + "\\" + sFileName;

	ifstream ifs(sFilePath);
	IStreamWrapper isw(ifs);
	Document doc;

	doc.ParseStream(isw);
	if (doc.IsObject()) {
		if (doc.HasMember("Topics")) {
			rapidjson::Value& topics = doc["Topics"];
			if (topics.IsArray()) {
				int count = topics.Size();
				for (int iTopic = 0; iTopic < count; iTopic++) {
					string sTitle;
					string sText;
					vector<ICondition*> vConditions;
					vector<string> vAction;
					rapidjson::Value& topic = topics[iTopic];
					if (topic.IsObject()) {
						if (topic.HasMember("Title")) {
							rapidjson::Value& title = topic["Title"];
							if (title.IsString())
								sTitle = title.GetString();
						}
						if (topic.HasMember("Text")) {
							rapidjson::Value& text = topic["Text"];
							if (text.IsString())
								sText += text.GetString();
							else if (text.IsArray()) {
								for (int iLine = 0; iLine < text.GetArray().Size(); iLine++) {
									rapidjson::Value& line = text[iLine];
									if (line.IsString())
										sText += line.GetString();
								}
							}
						}
						LoadJsonConditions(topic, vConditions, sFileName);
						LoadJsonActions(topic, vAction);
					}
					else if (topic.IsString()) {
					}
					CStringUtils::DecodeString(sTitle, sTitle);
					CStringUtils::DecodeString(sText, sText);
					AddTopic(sTitle, sText, vConditions, vAction);
				}
			}
		}
		if (doc.HasMember("Greatings")) {
			rapidjson::Value& greatings = doc["Greatings"];
			if (greatings.IsArray()) {
				int count = greatings.Size();
				for (int iGreating = 0; iGreating < count; iGreating++) {
					string sText;
					vector<ICondition*> vConditions;
					vector<string> vAction;
					rapidjson::Value& greating = greatings[iGreating];
					if (greating.IsObject()) {
						if (greating.HasMember("Text")) {
							rapidjson::Value& text = greating["Text"];
							if (text.IsString())
								sText += text.GetString();
							else if (text.IsArray()) {
								for (int iLine = 0; iLine < text.GetArray().Size(); iLine++) {
									rapidjson::Value& line = text[iLine];
									if (line.IsString())
										sText += line.GetString();
								}
							}
						}
						LoadJsonConditions(greating, vConditions, sFileName);
						LoadJsonActions(greating, vAction);
					}
					CStringUtils::DecodeString(sText, sText);
					AddGreating(sText, vConditions, vAction);
				}
			}
		}
	}
	else {
		CTopicException e("Erreur lors de chargement de 'topics.json'");
		throw e;
	}
	ifs.close();
}


void CTopicSystem::AddTopic(string sTopicName, string sText, vector<ICondition*>& conditions, const vector<string>& vAction)
{
	CTopic* topic = new CTopic;
	topic->SetText(sText);
	topic->SetConditions(conditions);
	topic->SetActions(vAction);
	m_mTopics[sTopicName].push_back(topic);
}

void CTopicSystem::AddGreating(string sText, vector<ICondition*>& conditions, vector<string>& actions)
{
	CTopic* greating = new CTopic;
	greating->SetText(sText);
	greating->SetConditions(conditions);
	greating->SetActions(actions);
	m_vGreatings.push_back(greating);
}

int CTopicSystem::IsConditionChecked(const vector<ITopic*>& topics, string sSpeakerId)
{
	vector<bool> checked;
	for (const ITopic* topic : topics)
		checked.push_back(true);
	int i = 0;
	for (ITopic* pTopic : topics) {
		CTopic* topic = static_cast<CTopic*>(pTopic);
		for (ICondition* pCondition : topic->GetConditions()) {
			CCondition& condition = *static_cast<CCondition*>(pCondition);
			if (condition.m_sName == "CharacterId") {
				if (condition.m_eComp == condition.eEqual) {
					if (!condition.m_sValue.empty() && condition.m_sValue != sSpeakerId) {
						checked[i] = false;
						break;
					}
					else {
						checked[i] = true;
					}
				}
			}
			else if (condition.m_sType == "Global") {
				float fValue = m_pScriptManager->GetVariableValue(condition.m_sName);
				if (condition.Evaluate(fValue)) {
					checked[i] = true;
				}
				else {
					checked[i] = false;
					break;
				}
			}
			else if (condition.m_sType == "PCItem") {
				int count = m_pEntityManager->GetPlayer()->GetItemCount(condition.m_sName);
				checked[i] = condition.Evaluate(count);
				if (!checked[i])
					break;
			}
			else if (condition.m_sType == "SpeakerLocal") {
				IEntity* pCharacter = m_pEntityManager->GetEntity(sSpeakerId);
				if (pCharacter) {
					if (CStringUtils::IsInteger(condition.m_sValue)) {
						int nCharacterValue;
						pCharacter->GetLocalVariableValue(condition.m_sName, nCharacterValue);
						checked[i] = condition.Evaluate(nCharacterValue);
						if (!checked[i])
							break;
					}
					else if (CStringUtils::IsFloat(condition.m_sValue)) {
						float fCharacterValue;
						pCharacter->GetLocalVariableValue(condition.m_sName, fCharacterValue);
						checked[i] = condition.Evaluate(fCharacterValue);
						if (!checked[i])
							break;
					}
					else {
						string sCharacterValue;
						pCharacter->GetLocalVariableValue(condition.m_sName, sCharacterValue);
						checked[i] = condition.Evaluate(sCharacterValue);
						if (!checked[i])
							break;
					}
				}
				else
					break;
			}
		}
		i++;
	}
	int count;
	for (bool check : checked) {
		if (check)
			return true;
	}
	return false;
}

string CTopicSystem::GetName()
{
	return "TopicSystem";
}

void CTopicSystem::GetCharacterTopics(string sCharacterID, vector<ITopic*>& topics)
{
	for (map<string, vector<ITopic*>>::const_iterator itTopic = m_mTopics.begin(); itTopic != m_mTopics.end(); itTopic++) {
		const pair<string, vector<ITopic*>>& topic = *itTopic;
		if (IsConditionChecked(topic.second, sCharacterID)) {
			string sTopic = topic.first;
			int nIdx = SelectTopic(topic.second, sCharacterID);
			if (nIdx >= 0) {
				const CTopic* pTopic = static_cast<const CTopic*>(topic.second[nIdx]);
				string sFormatedText;
				Format(pTopic->GetText(), sCharacterID, sFormatedText);
				CTopic* newTopic = new CTopic(*pTopic);
				newTopic->SetText(sFormatedText);
				newTopic->SetName(sTopic);
				topics.push_back(newTopic);
			}
		}
	}
}

map<string, vector<ITopic*>>& CTopicSystem::GetAllTopics()
{
	return m_mTopics;
}

void CTopicSystem::Format(string sTopicText, string sSpeakerId, string& sFormatedText)
{
	int nIndex = 0;
	int nLastIndex = 0;
	while (nIndex < sTopicText.size()) {
		string varTag = "<var>";
		nIndex = sTopicText.find(varTag, nIndex);
		sFormatedText += sTopicText.substr(nLastIndex, nIndex);
		if (nIndex == -1)
			break;
		int nEndVarIndex = sTopicText.find("</var>");
		if (nEndVarIndex == -1) {
			ostringstream oss;
			oss << "'var' tag not closed";
			throw CEException(oss.str());
		}
		int nVarNameSize = nEndVarIndex - nIndex - varTag.size();
		string sVarName = sTopicText.substr(nIndex + varTag.size(), nVarNameSize);
		string sVarValue;
		GetVarValue(sVarName, sSpeakerId, sVarValue);
		sFormatedText += sVarValue;

		nIndex += varTag.size() + sVarName.size() + varTag.size() + 1;
		nLastIndex = nIndex;
	}
}

void CTopicSystem::GetVarValue(string sVarName, string sCharacterId, string& sValue)
{
	if (sVarName == "CharacterName") {
		sValue = sCharacterId;
	}
}

void CTopicSystem::LoadJsonActions(rapidjson::Value& oParentNode, vector<string>& vAction)
{
	if (oParentNode.HasMember("Actions")) {
		rapidjson::Value& actions = oParentNode["Actions"];
		if (actions.IsArray()) {
			for (int iAction = 0; iAction < actions.GetArray().Size(); iAction++) {
				rapidjson::Value& action = actions[iAction];
				if (action.IsString()) {
					string sAction = action.GetString();
					CStringUtils::DecodeString(sAction, sAction);
					vAction.push_back(sAction);
				}
			}
		}
	}
}

void CTopicSystem::LoadJsonConditions(rapidjson::Value& oParentNode, vector<ICondition*>& vConditions, string sFileName)
{
	if (oParentNode.HasMember("Conditions")) {
		rapidjson::Value& conditions = oParentNode["Conditions"];
		if (conditions.IsArray()) {
			for (int iCondition = 0; iCondition < conditions.GetArray().Size(); iCondition++) {
				rapidjson::Value& condition = conditions[iCondition];
				CCondition* pCondition = new CCondition;
				string sTestVariableName, sTestVariableValue;
				if (condition.IsObject()) {
					if (condition.HasMember("Name")) {
						if (condition.HasMember("Type")) {
							rapidjson::Value& type = condition["Type"];
							if (type.IsString()) {
								pCondition->m_sType = type.GetString();
							}
						}
						rapidjson::Value& varName = condition["Name"];
						if (varName.IsString())
							pCondition->m_sName = varName.GetString();
						rapidjson::Value& value = condition["Value"];
						if (value.IsString())
							pCondition->m_sValue = value.GetString();
						if (condition.HasMember("Comp")) {
							rapidjson::Value& comp = condition["Comp"];
							if (value.IsString()) {
								string sComp = comp.GetString();
								if (sComp == "==")
									pCondition->m_eComp = CCondition::eEqual;
								else if (sComp == "!=")
									pCondition->m_eComp = CCondition::eDifferent;
								else if (sComp == "<")
									pCondition->m_eComp = CCondition::eInf;
								else if (sComp == "<=")
									pCondition->m_eComp = CCondition::eInfEqual;
								else if (sComp == ">")
									pCondition->m_eComp = CCondition::eSup;
								else if (sComp == ">=")
									pCondition->m_eComp = CCondition::eSupEqual;
								else if (sComp == "is")
									pCondition->m_eComp = CCondition::eIs;
								else if (sComp == "isNot")
									pCondition->m_eComp = CCondition::eIsNot;
							}
						}
						else
							throw CEException(string("Error during parsing '") + sFileName + "' : 'Comp' is missing in condition for topic '" + oParentNode.GetString());
					}
				}
				vConditions.push_back(pCondition);
			}
		}
	}
}

const ITopic* CTopicSystem::SelectTopic(string sTopicName, string sSpeakerId)
{
	map<string, vector<ITopic*>>::const_iterator itTopic = m_mTopics.find(sTopicName);
	const vector<ITopic*>& topics = itTopic->second;
	int selectedTopicIndex = SelectTopic(topics, sSpeakerId);
	const ITopic* pTopic = topics.at(selectedTopicIndex);
	return pTopic;
}

ITopic* CTopicSystem::SelectGreating(string sSpeakerId)
{
	int selectedTopicIndex = SelectTopic(m_vGreatings, sSpeakerId);
	ITopic* pTopic = m_vGreatings.at(selectedTopicIndex);
	return pTopic;
}

int CTopicSystem::SelectTopic(const vector<ITopic*>& topics, string sSpeakerId)
{
	vector<int> topicVerifiedConditionCount;
	for (int k = 0; k < topics.size(); k++)
		topicVerifiedConditionCount.push_back(0);
	int i = 0;
	for (ITopic* pTopic : topics) {
		CTopic* topic = static_cast<CTopic*>(pTopic);
		for (ICondition* pCondition : topic->GetConditions()) {
			CCondition& condition = *static_cast<CCondition*>(pCondition);
			if (condition.m_sName == "CharacterId") {
				if (condition.m_eComp == condition.eEqual) {
					if (!condition.m_sValue.empty() && condition.m_sValue != sSpeakerId) {
						topicVerifiedConditionCount[i] = -1;
						break;
					}
					else {
						topicVerifiedConditionCount[i] += 1;
					}
				}
			}
			else if (condition.m_sType == "Global") {
				float fValue = m_pScriptManager->GetVariableValue(condition.m_sName);
				if (condition.Evaluate(fValue)) {
					topicVerifiedConditionCount[i] += 1;
				}
				else {
					topicVerifiedConditionCount[i] = -1;
					break;
				}
			}
			else if (condition.m_sType == "PCItem") {
				int count = m_pEntityManager->GetPlayer()->GetItemCount(condition.m_sName);
				if (condition.Evaluate(count))
					topicVerifiedConditionCount[i] += 1;
				else {
					topicVerifiedConditionCount[i] = -1;
					break;
				}
			}
			else if (condition.m_sType == "SpeakerLocal") {
				IEntity* pCharacter = m_pEntityManager->GetEntity(sSpeakerId);
				if (pCharacter) {
					if (CStringUtils::IsInteger(condition.m_sValue)) {
						int nCharacterValue;
						if (pCharacter->GetLocalVariableValue(condition.m_sName, nCharacterValue)) {
							if (condition.Evaluate(nCharacterValue))
								topicVerifiedConditionCount[i] += 1;
							else {
								topicVerifiedConditionCount[i] -= 1;
								break;
							}
						}
					}
					else if (CStringUtils::IsFloat(condition.m_sValue)) {
						float fCharacterValue;
						if (pCharacter->GetLocalVariableValue(condition.m_sName, fCharacterValue) && condition.Evaluate(fCharacterValue))
							topicVerifiedConditionCount[i] += 1;
						else {
							topicVerifiedConditionCount[i] -= 1;
							break;
						}
					}
					else {
						string sCharacterValue;
						if (pCharacter->GetLocalVariableValue(condition.m_sName, sCharacterValue) && condition.Evaluate(sCharacterValue))
							topicVerifiedConditionCount[i] += 1;
						else {
							topicVerifiedConditionCount[i] -= 1;
							break;
						}
					}
				}
				else {
					topicVerifiedConditionCount[i] -= 1;
					break;
				}
			}
			else if (condition.m_sName == "SpeakerClass") {
				ICharacter* pCharacter = dynamic_cast<ICharacter*>(m_pEntityManager->GetEntity(sSpeakerId));
				if (condition.m_sValue == pCharacter->GetClass())
					topicVerifiedConditionCount[i] += 1;
				else {
					topicVerifiedConditionCount[i] -= 1;
					break;
				}
			}
		}
		i++;
	}
	int max = -1;
	int higherIndex = -1;
	for (int i = 0; i < topicVerifiedConditionCount.size(); i++) {
		if (max < topicVerifiedConditionCount[i]) {
			max = topicVerifiedConditionCount[i];
			higherIndex = i;
		}
	}
	return higherIndex;
}

extern "C" _declspec(dllexport) ITopicSystem* CreateTopicSystem(EEInterface& oInterface)
{
	return new CTopicSystem(oInterface);
}