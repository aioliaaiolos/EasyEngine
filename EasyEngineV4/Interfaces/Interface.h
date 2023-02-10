#ifndef EE_INTERFACE_H
#define EE_INTERFACE_H

#include "EEPlugin.h"
#include <map>
#include <functional>


using namespace std;


class EEInterface
{
public:
	using PluginCreationProc = std::function<void(CPlugin*)>;


	void RegisterPlugin(CPlugin* plugin)
	{
		m_mPlugins.insert(map<string, CPlugin*>::value_type(plugin->GetName(), plugin));
		map<string, vector<PluginCreationProc>>::iterator itCallback = s_vPluginCreationCallback.find(plugin->GetName());
		if (itCallback != s_vPluginCreationCallback.end())
			for (int i = 0; i < itCallback->second.size(); i++)
				itCallback->second[i](plugin);
	}

	CPlugin* GetPlugin(string sName)
	{
		map<string, CPlugin*>::iterator it = m_mPlugins.find(sName);
		if (it != m_mPlugins.end())
			return it->second;
		return nullptr;
	}
	
	void HandlePluginCreation(string pluginName, PluginCreationProc callback)
	{
		map<string, CPlugin*>::iterator itPlugin = m_mPlugins.find(pluginName);
		if (itPlugin != m_mPlugins.end()) {
			map<string, vector<PluginCreationProc>>::iterator itPluginCallback = s_vPluginCreationCallback.find(pluginName);
			callback(itPlugin->second);
		}
		else
			s_vPluginCreationCallback[pluginName].push_back(callback);
	}

private:
	map<string, CPlugin*>											m_mPlugins;
	map<string, vector<PluginCreationProc>>							s_vPluginCreationCallback;
};

#endif // EE_INTERFACE_H