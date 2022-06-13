#define WINDOW2_CPP

#include "Window2.h"
#include <exception>
#include <sstream>
#include "Menu2.h"
#include "IEventDispatcher.h"
#include "exception.h"

using namespace std;

map< HWND, CWindow2* > CWindow2::s_mWindow;
map< int, string > CWindow2::s_mErrorString;



CWindow2::CallbackArgs::CallbackArgs()
{
}

CWindow2::CWindow2( const IWindow::Desc& desc ):
CWidget( desc ),
m_bShowCursor( desc.m_bShowCursor),
m_pfnWindowCallback( NULL ),
m_bDestroy(false),
m_bFullscreen( desc.m_bFullscreen ),
m_nBits( desc.m_nBits ),
m_oEventDispatcher( desc.m_oEventDispatcher )
{
	InitErrorString();
	string sErrorMessage;
	if ( TestDesc( desc, sErrorMessage ) == false )
	{
		exception e( sErrorMessage.c_str() );
		throw e;
	}
	m_pfnWindowCallback = desc.m_pfnWindowCallback;
	m_sClassName = desc.m_sClassName;
	m_pMenu = desc.m_pMenu;

	WNDCLASSA WndClass;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	if ( desc.m_ColorBrush != -1 )
		WndClass.hbrBackground = (HBRUSH)GetStockObject( desc.m_ColorBrush );// desc.m_hbrBackGround;
	else
		WndClass.hbrBackground = NULL;
	WndClass.hCursor = LoadCursor( NULL, IDC_ARROW );
	WndClass.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	WndClass.hInstance = GetModuleHandleA( NULL );
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = desc.m_sClassName.c_str();
	WndClass.lpszMenuName = "MainMenu";
	if ( desc.m_dwClassStyle == -1 )
		WndClass.style = CS_HREDRAW | CS_VREDRAW;
	else
		WndClass.style = desc.m_dwClassStyle;
	LRESULT lRegister = RegisterClassA( &WndClass );
	if( lRegister == 0 )
	{
		ostringstream ssErrorMessage;
		ssErrorMessage << "Impossible d'enregistrer la classe " << WndClass.lpszClassName << ", code d'erreur " << GetLastError();
		exception e( ssErrorMessage.str().c_str() );
		throw e;
	}
	DWORD dwStyle;
	if ( desc.m_dwStyle == -1 )
		dwStyle = WS_OVERLAPPEDWINDOW;
	else
		dwStyle = desc.m_dwStyle;


	DWORD dwExStyle = desc.m_dwExStyle;
	if ( desc.m_bFullscreen )
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth	= desc.m_nWidth;
		dmScreenSettings.dmPelsHeight	= desc.m_nHeight;
		dmScreenSettings.dmBitsPerPel	= desc.m_nBits;
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
		
		if ( ChangeDisplaySettingsA(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL )
		{
			exception e( "The Requested Fullscreen Mode Is Not Supported By Video Card" );
			throw e;
		}
		dwExStyle = WS_EX_APPWINDOW;								
		dwStyle=WS_POPUP;										
		RECT WindowRect;
		WindowRect.top = 0;
		WindowRect.left = 0;
		WindowRect.bottom = desc.m_nHeight;
		WindowRect.right = desc.m_nWidth;
		AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);
	}	

	m_oEventDispatcher.DispatchWindowEvent( IEventDispatcher::T_WINDOWCREATE, desc.m_nWidth, desc.m_nHeight );
	CreateWnd( dwExStyle, WndClass.lpszClassName, desc.m_sClassName, dwStyle, desc.m_nPosX, desc.m_nPosY, desc.m_nWidth, desc.m_nHeight, m_pMenu );
	s_mWindow[ m_hWnd ] = this;
	if ( desc.m_bCentered )
	{
		RECT WindowRect, oClientRect;
		GetWindowRect( m_hWnd, &WindowRect );
		GetClientRect( m_hWnd, &oClientRect );
		DEVMODEA oDevMode;
		memset( &oDevMode, 0, sizeof( DEVMODEA ) );
		oDevMode.dmSize = sizeof( DEVMODEA );
		EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &oDevMode );
		oDevMode.dmPelsWidth;
		int xPos = ( oDevMode.dmPelsWidth - WindowRect.right )/ 2;
		int yPos = ( oDevMode.dmPelsHeight - WindowRect.bottom )/ 2;
		int xClientPos = ( oDevMode.dmPelsWidth - oClientRect.right )/ 2;
		int yClientPos = ( oDevMode.dmPelsHeight - oClientRect.bottom )/ 2;
		SetWindowPos( m_hWnd, NULL, xPos, yPos, xClientPos, yClientPos, SWP_SHOWWINDOW );
	}
	if ( m_hWnd == NULL )
	{
		ostringstream ssErrorMessage;
		string sErrorString;
		GetStringByErrorCode( GetLastError(), sErrorString );
		if ( sErrorString.size() > 0 )
			ssErrorMessage << "Impossible de créer la fenêtre " << desc.m_sClassName << " : " << sErrorString ;
		else
			ssErrorMessage << "Impossible de créer la fenêtre " << desc.m_sClassName << " : code d'erreur " << GetLastError();
		exception e( ssErrorMessage.str().c_str() );
		throw e;
	}
}

LRESULT CALLBACK CWindow2::WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	CWindow2* pCurrentWindow = NULL;
	map< HWND, CWindow2* >::iterator itWindow = s_mWindow.find( hWnd );
	if ( itWindow != s_mWindow.end() )
		pCurrentWindow = itWindow->second;

	try
	{
		switch (msg)
		{
		case WM_DESTROY:
			pCurrentWindow->m_bDestroy = true;
			PostQuitMessage(0);
			break;
		case WM_COMMAND:
			if (pCurrentWindow->m_pMenu)
			{
				if (pCurrentWindow->m_pMenu->m_pfnCallback)
					pCurrentWindow->m_pMenu->m_pfnCallback((int)wParam);
			}
			break;
		}

		if (pCurrentWindow)
		{
			IWidget* pWindow = reinterpret_cast<IWidget*> (pCurrentWindow);
			pCurrentWindow->CallCallback(CallbackArgs(pWindow, msg, wParam, lParam));
			for (int i = 0; i < pCurrentWindow->GetChildCount(); i++)
				pCurrentWindow->GetChild(i)->CallCallback(CallbackArgs(pWindow, msg, wParam, lParam));
		}
	}
	catch( CTerminateException& e )
	{
		e = e;
		pCurrentWindow->m_bDestroy = true;
		PostQuitMessage( 0 );
	}
	return DefWindowProcA( hWnd, msg, wParam, lParam );
}

CWindow2::~CWindow2(void)
{
}


void CWindow2::Show()const
{
	ShowWindow( m_hWnd, SW_SHOW );
	if ( UpdateWindow( m_hWnd ) == FALSE )
	{
		ostringstream ssErrorMessage;
		ssErrorMessage << "Erreur à l'update de la fenêtre " << m_sClassName << ", code d'erreur " << GetLastError();
		exception e( ssErrorMessage.str().c_str() );
		throw e;
	}
}

void CWindow2::ShowModal()
{
	m_bDestroy = false;
	Show();
	ShowCursor( m_bShowCursor );
	MSG msg;
	while( !m_bDestroy )
	{
		if ( PeekMessage(&msg,NULL,0,0,PM_REMOVE) )	
		{
			TranslateMessage(&msg);				
			DispatchMessage(&msg);							
		}
		else	
		{
			unsigned int nWidth, nHeight;
			GetDimension( nWidth, nHeight );
			m_oEventDispatcher.DispatchWindowEvent( IEventDispatcher::T_WINDOWUPDATE, nWidth, nHeight );
		}
	}
}

bool CWindow2::TestDesc( const IWindow::Desc& desc, string& sErrorMessage )const
{
	bool bReturn = true;
	if ( desc.m_sClassName == "" )
	{
		sErrorMessage = "La fenêtre doit avoir un nom";
		bReturn = false;
	}
	return bReturn;
}

void CWindow2::InitErrorString()
{
	s_mErrorString[ ERROR_RESOURCE_TYPE_NOT_FOUND ] = "The specified resource type cannot be found in the image file";
}

void CWindow2::GetStringByErrorCode( int nErrorCode, string& sString )
{
	map< int, string >::iterator itError = s_mErrorString.find( nErrorCode );
	if ( itError != s_mErrorString.end() )
		sString = s_mErrorString[ nErrorCode ];
}

CWindow2* CWindow2::GetWindow( HWND hWnd )
{
	return s_mWindow[ hWnd ];
}

void CWindow2::Close() const
{
	if ( m_bFullscreen )
	{
		ChangeDisplaySettingsA( NULL, 0 );
		ShowCursor( TRUE );
	}
	DestroyWindow( m_hWnd );
	UnregisterClassA( m_sClassName.c_str(), GetModuleHandleA(NULL) );
}

void CWindow2::CallCallback( CallbackArgs& args )
{
	switch( args.m_msg )
	{
	case WM_SHOWWINDOW:
		m_oEventDispatcher.DispatchWindowEvent( IEventDispatcher::T_WINDOWSHOW, LOWORD( args.m_lParam ), HIWORD( args.m_lParam ) );
		break;
	case WM_KEYUP:
		m_oEventDispatcher.DispatchKeyEvent( IEventDispatcher::T_KEYUP, (int)args.m_wParam );
		break;
	case WM_KEYDOWN:
		m_oEventDispatcher.DispatchKeyEvent( IEventDispatcher::T_KEYDOWN, (int)args.m_wParam );
		break;
	case WM_MOUSEMOVE:
		{
			POINT p;
			GetCursorPos( &p );
			m_oEventDispatcher.DispatchMouseEvent( IEventDispatcher::T_MOVE, p.x, p.y );
			break;
		}
	case WM_MOUSEWHEEL: 
		{
			int value = GET_WHEEL_DELTA_WPARAM(args.m_wParam) / WHEEL_DELTA;
			m_oEventDispatcher.DispatchMouseEvent(IEventDispatcher::T_WHEEL, value, 0);
		}
		break;
	case WM_CLOSE:
		m_oEventDispatcher.DispatchWindowEvent( IEventDispatcher::T_WINDOWCLOSE, LOWORD( args.m_lParam ), HIWORD( args.m_lParam ) );
		break;
	case WM_DESTROY:
		m_oEventDispatcher.DispatchWindowEvent( IEventDispatcher::T_WINDOWDESTROY, LOWORD( args.m_lParam ), HIWORD( args.m_lParam ) );
		break;
	case WM_SIZE:
		m_oEventDispatcher.DispatchWindowEvent( IEventDispatcher::T_WINDOWRESIZE, LOWORD( args.m_lParam ), HIWORD( args.m_lParam ) );
		break;
	case WM_LBUTTONDOWN:
		POINT p;
		GetCursorPos( &p );
		m_oEventDispatcher.DispatchMouseEvent( IEventDispatcher::T_LBUTTONDOWN, p.x, p.y );
		break;
	case WM_LBUTTONUP:
		GetCursorPos( &p );
		m_oEventDispatcher.DispatchMouseEvent( IEventDispatcher::T_LBUTTONUP, p.x, p.y );
		break;
	case WM_RBUTTONDOWN:
		GetCursorPos( &p );
		m_oEventDispatcher.DispatchMouseEvent( IEventDispatcher::T_RBUTTONDOWN, p.x, p.y );
		break;
	case WM_RBUTTONUP:
		GetCursorPos( &p );
		m_oEventDispatcher.DispatchMouseEvent( IEventDispatcher::T_RBUTTONUP, p.x, p.y );
		break;
		
	}
	
	if ( m_pfnWindowCallback )
		m_pfnWindowCallback( args.m_pParent, args.m_msg, args.m_wParam, args.m_lParam );
}

bool CWindow2::IsFullscreenMode()
{
	return m_bFullscreen;
}

void CWindow2::Setfocus()
{
	SetFocus( m_hWnd );
}

void CWindow2::SetForeground()
{
	SetForegroundWindow( m_hWnd );
}

int CWindow2::GetBits() const
{
	return m_nBits;
}

HWND CWindow2::GetHandle()
{
	return CWidget::GetHandle();
}

void CWindow2::GetDimension(unsigned int& nWidth, unsigned int& nHeight )const
{
	CWidget::GetDimension( nWidth, nHeight );
}

void CWindow2::AddChild( IWidget* pChild )
{
	CWidget::AddChild( pChild );
}

string CWindow2::GetName()
{
	return "Window";
}

void CWindow2::SetFullScreenMode(bool fullscreen)
{
	m_bFullscreen = fullscreen;
}

extern "C" _declspec(dllexport) IWindow* CreateWindow2( const IWindow::Desc& oDesc )
{
	return new CWindow2( oDesc );
}
