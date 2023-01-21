#ifndef EE_INTERFACE_H
#define EE_INTERFACE_H

#include "EEPlugin.h"
#include <map>

class IObject;

using namespace std;

typedef void(*PluginCreationProc)(CPlugin* plugin, IObject* pData);

class EEInterface
{
public:
	void RegisterPlugin(CPlugin* plugin)
	{
		m_mPlugins.insert(map<string, CPlugin*>::value_type(plugin->GetName(), plugin));
		map<string, vector<pair<PluginCreationProc, IObject*>>>::iterator itCallback = s_vPluginCreationCallback.find(plugin->GetName());
		if (itCallback != s_vPluginCreationCallback.end())
			for (int i = 0; i < itCallback->second.size(); i++)
				itCallback->second[i].first(plugin, itCallback->second[i].second);
	}

	CPlugin* GetPlugin(string sName)
	{
		map<string, CPlugin*>::iterator it = m_mPlugins.find(sName);
		if (it != m_mPlugins.end())
			return it->second;
		return nullptr;
	}
	
	void HandlePluginCreation(string pluginName, PluginCreationProc callback, IObject* pData)
	{
		s_vPluginCreationCallback[pluginName].push_back(pair<PluginCreationProc, IObject*>(callback, pData));
	}

private:
	map<string, CPlugin*>									m_mPlugins;
	map<string, vector<pair<PluginCreationProc, IObject*>>>	s_vPluginCreationCallback;
};

#endif // EE_INTERFACE_H