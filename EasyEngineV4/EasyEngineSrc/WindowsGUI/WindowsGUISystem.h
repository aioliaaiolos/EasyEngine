#pragma once

#include "IWindow.h"

class CWindowsGUISystem : public IWindowsGUISystem
{
public:
	CWindowsGUISystem(EEInterface& oInterface);

	IWindow* CreateWindow2(const IWindow::Desc& oWindowDesc) override;
	IWindow* CreateWindowEditor(const IWindow::Desc& oWindowDesc) override;

	IWindow* GetWindow2() override;
	IWindow* GetWindowEditor() override;

	string	GetName();

private:
	IWindow*	m_pWindow2 = nullptr;
	IWindow*	m_pWindowEditor = nullptr;
	EEInterface&	m_oInterface;
};

extern "C" _declspec(dllexport) IWindowsGUISystem* CreateWindowsGUISystem(EEInterface& oInterface);