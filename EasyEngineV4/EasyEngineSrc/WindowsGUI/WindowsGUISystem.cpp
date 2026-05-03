#include "WindowsGUISystem.h"
#include "Window2.h"
#include "WindowEditor.h"
#include "Interface.h"
#include "ITopicSystem.h"

CWindowsGUISystem::CWindowsGUISystem(EEInterface& oInterface) : m_oInterface(oInterface)
{

}


IWindow* CWindowsGUISystem::CreateWindow2(const IWindow::Desc& oWindowDesc)
{
	m_pWindow2 = new CWindow2(oWindowDesc);
	return m_pWindow2;
}

IWindow* CWindowsGUISystem::CreateWindowEditor(const IWindow::Desc& oWindowDesc)
{
	CWindowEditor::EditorDesc descEditor(oWindowDesc, m_oInterface);
	m_pWindowEditor = new CWindowEditor(descEditor);
	return m_pWindowEditor;
}

IWindow* CWindowsGUISystem::GetWindow2()
{
	return m_pWindow2;
}

IWindow* CWindowsGUISystem::GetWindowEditor()
{
	return m_pWindowEditor;
}

string CWindowsGUISystem::GetName()
{
	return "WindowsGUISystem";
}

extern "C" _declspec(dllexport) IWindowsGUISystem* CreateWindowsGUISystem(EEInterface& oInterface)
{
	return new CWindowsGUISystem(oInterface);
}

