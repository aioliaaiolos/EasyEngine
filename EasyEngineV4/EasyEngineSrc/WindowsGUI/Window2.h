#ifndef WINDOW2_CPP
#error
#endif

#ifndef BASE_WINDOW_H
#define BASE_WINDOW_H

#include <windows.h>
#include <string>
#include <map>
#include "Widget.h"
#include "IWindow.h"
#include "EEPlugin.h"

class CMenu2;
class IEventDispatcher;

class CWindow2 : public IWindow, public CWidget
{
public:

private:
	bool									m_bFullscreen;
	WINDOWCALLBACK							m_pfnWindowCallback;
	std::string								m_sClassName;
	CMenu2*									m_pMenu;
	static std::map< int, std::string >		s_mErrorString;
	static									std::map< HWND, CWindow2* >	s_mWindow;
	bool									TestDesc( const IWindow::Desc&, std::string& sErrorMessage )const;
	int										m_nBits;
	bool									m_bShowCursor;

	static void								InitErrorString();
	static									LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

protected:
	static void								GetStringByErrorCode( int nErrorCode, std::string& sString );
	static CWindow2*						GetWindow( HWND hWnd );	
	bool									m_bDestroy;
	IEventDispatcher&						m_oEventDispatcher;

public:

	CWindow2( const IWindow::Desc& );
	virtual 								~CWindow2();
	int										GetBits() const;
	void									Show()const;
	void									ShowModal();
	void									Close()const;
	void									CallCallback( CallbackArgs& args );
	bool									IsFullscreenMode();
	void									Setfocus();
	void									SetForeground();
	HWND									GetHandle();
	void									GetDimension(unsigned int& nWidth, unsigned int& nHeight )const;
	void									AddChild( IWidget* pChild );
	string									GetName() override;
	void									SetFullScreenMode(bool fullscreen) override;
};

extern "C" _declspec(dllexport) IWindow* CreateWindow2( const IWindow::Desc& );

#endif // BASE_WINDOW_H