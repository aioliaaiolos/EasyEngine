#pragma once
#include "Window2.h"


class ITopicSystem;

class CWindowEditor : public CWindow2
{
public:
	struct EditorDesc : public IWindow::Desc
	{
		//ITopicSystem* m_pTopicSystem = nullptr;
		EEInterface&	m_oInterface;

		EditorDesc(const IWindow::Desc& desc, EEInterface&	oInterface) : IWindow::Desc(desc), m_oInterface(oInterface)
		{
		}
	};

	CWindowEditor(const CWindowEditor::EditorDesc& desc);

	static INT_PTR CALLBACK OnTopicCallback(HWND, UINT, WPARAM, LPARAM);

private:
	static void FillFieldsFromType(HWND hWnd, int idcConditionType, EEInterface* pInterface);
	static ITopicSystem* s_pTopicSystem;
	static string s_LastSelectedTitle;
};