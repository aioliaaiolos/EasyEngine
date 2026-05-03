#ifndef IWINDOW_H
#define IWINDOW_H

#include "IWidget.h"
#include "IEventDispatcher.h"
#include <functional>

class CMenu2;

using WINDOWCALLBACK = std::function<LRESULT(IWidget*, UINT, WPARAM, LPARAM)>;

class IWindow : public IWidget
{
public:
	struct Desc : public IWidget::Desc
	{
		std::string				m_sClassName;
		WINDOWCALLBACK			m_pfnWindowCallback;
		void(*m_pfnOnUpdate)( const IWindow* );
		int						m_ColorBrush;
		DWORD					m_dwClassStyle;
		DWORD					m_dwExStyle;
		DWORD					m_dwStyle;
		bool					m_bCentered;
		CMenu2*					m_pMenu;
		bool					m_bFullscreen;
		int						m_nBits;
		IEventDispatcher&		m_oEventDispatcher;
		bool					m_bShowCursor;

		Desc( int nWidth, int nHeight, std::string sClassName, IEventDispatcher& oEventDispatcher ):
				IWidget::Desc( nWidth, nHeight, NULL ),
					m_sClassName( sClassName ),
					m_ColorBrush( WHITE_BRUSH ),
					m_dwClassStyle( -1 ),
					m_dwExStyle( 0 ),
					m_dwStyle( -1 ),
					m_pMenu( NULL ),
					m_bCentered(false),
					m_bFullscreen(false),
					m_pfnOnUpdate(NULL),
					m_nBits( 32 ),
					m_oEventDispatcher( oEventDispatcher ),
					m_pfnWindowCallback( NULL ),
					m_bShowCursor( false )
					{
					}	
	};

	virtual void			Close() = 0;
	virtual int				GetBits() const = 0;
	virtual void			Show() const = 0;
	virtual void			Setfocus() = 0;
	virtual void			SetForeground() = 0;
	virtual void			ShowModal() = 0;
	virtual void			SetFullScreenMode(bool fullscreen) = 0;
	virtual void			DisplayCursor(bool bShow) = 0;
};

class IWindowsGUISystem : public CPlugin
{
public:
	IWindowsGUISystem() : CPlugin(nullptr, "WindowsGUISystem") {}
	virtual IWindow* CreateWindow2(const IWindow::Desc& oWindowDesc) = 0;
	virtual IWindow* CreateWindowEditor(const IWindow::Desc& oWindowDesc) = 0;

	virtual IWindow* GetWindow2() = 0;
	virtual IWindow* GetWindowEditor() = 0;
};

#endif // IWINDOW_H