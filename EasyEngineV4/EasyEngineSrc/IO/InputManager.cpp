#include "inputmanager.h"

#define INPUT_METHODE_3

using namespace std;

CInputManager::CInputManager( const IInputManager::Desc& oDesc ):
IInputManager( oDesc ),
m_oEventDispatcher( oDesc.m_oEventDispatcher ),
m_ButtonState( 0 ),
m_Left( 0 ),
m_Right( oDesc.m_nResX ),
m_Top( 0 ),
m_Bottom( oDesc.m_nResY ),
m_fMouseSensitivity( 0.05f ),
m_nVirtualCursorPosx( 0 ),
m_nVirtualCursorPosy( 0 ),
m_bEditionMode( false ),
m_bWheelConsumed(false),
m_bUpLButtonConsumed(false)
{
	m_OffsetMousePos.x = 0;
	m_OffsetMousePos.y = 0;
	m_OldMousePos.x = 0;
	m_OldMousePos.y  =0;

	m_oEventDispatcher.AbonneToKeyEvent( static_cast< IInputManager* > (this), OnKeyEventCallback );
	m_oEventDispatcher.AbonneToMouseEvent( static_cast< IInputManager* > (this), OnMouseEventCallback );
}


CInputManager::~CInputManager(void)
{
}

void CInputManager::OnUpdate()
{
	for( unsigned int i = 0; i < m_vLastKeyEvent.size(); i++ )
	{
		int key = m_vLastKeyEvent[ i ];
		switch( m_mKeyboardState[ key ] )
		{
		case JUST_PRESSED:
			m_mKeyboardState[ key ] = PRESSED;
			m_mKeyPressed[ key ] = true;
			break;
		case JUST_RELEASED:
			m_mKeyboardState[ key ] = RELEASED;
			break;
		}
	}
	m_vLastKeyEvent.clear();

	for( map< int, bool >::iterator itKey = m_mKeyPressed.begin(); itKey != m_mKeyPressed.end(); itKey++ )
	{
		unsigned short state = GetAsyncKeyState( itKey->first );
		if( state == 0 )
		{
			m_mKeyboardState[ itKey->first ] = JUST_RELEASED;
			itKey = m_mKeyPressed.erase( itKey );
			if( itKey == m_mKeyPressed.end() )
				break;
		}
	}

	if (m_bWheelConsumed) {
		map< TMouseButton, TMouseButtonState >::iterator itButton = m_mMouseButtonState.find(eMouseWheel);
		if (itButton != m_mMouseButtonState.end())
		{
			if (itButton->second == eMouseWheelUp || itButton->second == eMouseWheelDown) {
				itButton->second = eMouseWheelNone;
				m_bWheelConsumed = false;
			}
		}
	}

	if (m_bUpLButtonConsumed) {
		map< TMouseButton, TMouseButtonState >::iterator itButton = m_mMouseButtonState.find(eMouseButtonLeft);
		if (itButton != m_mMouseButtonState.end())
		{
			if (itButton->second == eMouseButtonStateJustUp) {
				itButton->second = eMouseButtonStateNone;
				m_bUpLButtonConsumed = false;
			}
		}
	}
}

string CInputManager::GetName()
{
	return "InputManager";
}

void CInputManager::SetEditionMode( bool bEdition )
{
	m_bEditionMode = bEdition;
	int ret = 0;
	bool condition = false; 
	do {
		ret = ShowCursor(bEdition);
		condition = bEdition ? ret >= 0 : ret < 0;
	} while (!condition);
}

void CInputManager::OnKeyEventCallback( CPlugin* pPlugin, IEventDispatcher::TKeyEvent e, int nKeyCode )
{ 
	CInputManager* pInputManager = static_cast< CInputManager* >( pPlugin );
	if ( e == IEventDispatcher::T_KEYDOWN )
		pInputManager->OnKeyPress( nKeyCode );
	if ( e == IEventDispatcher::T_KEYUP )
		pInputManager->OnKeyRelease( nKeyCode );
	pInputManager->m_vLastKeyEvent.push_back( nKeyCode );
}

void CInputManager::OnMouseEventCallback( CPlugin* pPlugin, IEventDispatcher::TMouseEvent e, int x, int y )
{
	CInputManager* pInputManager = static_cast< CInputManager* >( pPlugin );
	TMouseButton b = eMouseButtonNone;
	TMouseButtonState s = eMouseButtonStateNone;
	switch( e )
	{
	case IEventDispatcher::T_MOVE:
		pInputManager->OnMouseMove( x, y );
		break;
	case IEventDispatcher::T_LBUTTONDOWN:
		b = eMouseButtonLeft;
		if( s == eMouseButtonStateJustUp || s == eMouseButtonNone )
			s = eMouseButtonStateJustDown;
		else
			s = eMouseButtonStateDown;
		break;
	case IEventDispatcher::T_RBUTTONDOWN:
		b = eMouseButtonRight;
		if (s == eMouseButtonStateJustUp || s == eMouseButtonNone)
			s = eMouseButtonStateJustDown;
		else
			s = eMouseButtonStateDown;
		break;
	case IEventDispatcher::T_LBUTTONUP:
		b = eMouseButtonLeft;
		s = eMouseButtonStateJustUp;
		break;
	case IEventDispatcher::T_RBUTTONUP:
		b = eMouseButtonRight;
		s = eMouseButtonStateJustUp;
		break;
	case IEventDispatcher::T_WHEEL:
		b = eMouseWheel;
		break;
	}
	if (b != eMouseButtonNone && (b != eMouseWheel)) {
		pInputManager->OnMouseClick(b, s);
	}
	else if (b == eMouseWheel)
		pInputManager->OnMouseWheel(x);
}

void CInputManager::OnMouseClick( TMouseButton b, TMouseButtonState s )
{
	m_mMouseButtonState[ b ] = s;
}

void CInputManager::OnMouseWheel(int value) 
{
	m_mMouseButtonState[eMouseWheel] = value > 0 ? eMouseWheelDown : (value < 0 ? eMouseWheelUp : eMouseWheelNone);
}

void CInputManager::OnKeyPress( unsigned int key )
{
	KEY_STATE state = NONE;
	if ( ( m_mKeyboardState[ key ] == RELEASED ) || ( m_mKeyboardState[ key ] == JUST_RELEASED ) )
		state = JUST_PRESSED;
	else
	{
		state = PRESSED;
		m_mKeyPressed[ key ] = true;
	}
	m_mKeyboardState[ key ] = state;
	for ( unsigned int i = 0; i < m_vKeyCallback.size(); i++ )
	{
		TKeyCallback callback = m_vKeyCallback[ i ].second;
		callback( m_vKeyCallback[ i ].first, key, state );
	}
}

void CInputManager::OnKeyRelease( unsigned int key )
{
	KEY_STATE state = NONE;
	if ( ( m_mKeyboardState[ key ] == PRESSED ) || ( m_mKeyboardState[ key ] == JUST_PRESSED ) )
		state = JUST_RELEASED;
	else
		state = RELEASED;
	m_mKeyboardState[ key ] = state;
	for ( unsigned int i = 0; i < m_vKeyCallback.size(); i++ )
	{
		TKeyCallback callback = m_vKeyCallback[ i ].second;
		callback( m_vKeyCallback[ i ].first, key, state );
	}
}

void CInputManager::GetOffsetMouse( int& x, int& y )
{
	x = m_OffsetMousePos.x;
	y = m_OffsetMousePos.y;
	m_OffsetMousePos.x = 0;
	m_OffsetMousePos.y = 0;
}

void CInputManager::SetMouseCorner( int left, int right, int top, int bottom )
{
	m_Left = left;
	m_Right = right;
	m_Top = top;
	m_Bottom = bottom;
}

void CInputManager::OnMouseMove( int x, int y )
{	
	m_nPhysicalCursorPosx = x;
	m_nPhysicalCursorPosy = y;
	bool bMustChange = false;

	POINT OffsetMousePos;

	if (!m_bEditionMode) {
		m_OffsetMousePos.x += (x - m_OldMousePos.x);
		m_OffsetMousePos.y += (y - m_OldMousePos.y);
		OffsetMousePos.x = (x - m_OldMousePos.x);
		OffsetMousePos.y = (y - m_OldMousePos.y);
	}

	m_OldMousePos.x = x;		
	m_OldMousePos.y = y;
	
	if (!m_bEditionMode) {
		m_nVirtualCursorPosx += OffsetMousePos.x;
		m_nVirtualCursorPosy += OffsetMousePos.y;
	}

	if( !m_bEditionMode )
	{	
		POINT newPos;
		newPos.x = x;
		newPos.y = y;
		if ( x < m_Left + 10 )
		{
 			newPos.x += 180;
			bMustChange = true;
		}
		if ( x > m_Right - 10 )
		{
			newPos.x -= 180;		
			bMustChange = true;
		}

		if ( y < m_Top + 10 )
		{
			newPos.y += 180;
			bMustChange = true;
		}
		if ( y > m_Bottom - 10 )
		{
			newPos.y -= 180;
			bMustChange = true;
		}
		if ( bMustChange )
		{
			m_OldMousePos.x = newPos.x;
			m_OldMousePos.y = newPos.y;
			SetCursorPos( static_cast< int >( newPos.x ), static_cast< int >( newPos.y ) );
		}
	}
}
//
//void CInputManager::OnMouseButton( CPlugin* pPlugin, IEventDispatcher::TMouseEvent e, int x, int y )
//{
//	//CActionManager* pActionManager = static_cast< CActionManager* > ( pPlugin );
//	//TMouseButton b;
//	//TMouseButtonState s;
//	//switch( e )
//	//{
//	//case IEventDispatcher::T_LBUTTONDOWN:
//	//	b = eMouseButtonLeft;
//	//	s = eMouseButtonStateDown;
//	//	break;
//	//case IEventDispatcher::T_RBUTTONDOWN:
//	//	b = eMouseButtonRight;
//	//	s = eMouseButtonStateDown;
//	//	break;
//	//case IEventDispatcher::T_LBUTTONUP:
//	//	b = eMouseButtonLeft;
//	//	s = eMouseButtonStateUp;
//	//	break;
//	//case IEventDispatcher::T_RBUTTONUP:
//	//	b = eMouseButtonRight;
//	//	s = eMouseButtonStateUp;
//	//	break;
//	//}
//	//pActionManager->m_mMouseButtonState[ b ] = s;
//}

void CInputManager::GetPhysicalCursorPos(int& x, int& y)
{
	x = m_nPhysicalCursorPosx;
	y = m_nPhysicalCursorPosy;
}

void CInputManager::GetVirtualCursorPos( int& x, int& y )
{
	x = m_nVirtualCursorPosx;
	y = m_nVirtualCursorPosy;
}

void CInputManager::ShowMouseCursor(bool bShow)
{
	ShowCursor( bShow );
}

void CInputManager::SetMouseCursorPos(int nx, int ny)
{
	m_nVirtualCursorPosx = nx;
	m_nVirtualCursorPosy = ny;
}

void CInputManager::SetMouseCursorXPos(int nx)
{
	m_nVirtualCursorPosx = nx;
}


void CInputManager::SetMouseCursorYPos(int ny)
{
	m_nVirtualCursorPosy = ny;
}


void CInputManager::SetMouseButtonState(unsigned int ButtonState)
{
	m_ButtonState = ButtonState;
}

IInputManager::TMouseButtonState CInputManager::GetMouseButtonState( IInputManager::TMouseButton b )
{
	map< TMouseButton, TMouseButtonState >::iterator itButton = m_mMouseButtonState.find( b );
	if( itButton != m_mMouseButtonState.end() )
	{
		TMouseButton b = itButton->first;
		TMouseButtonState s = itButton->second;
		if( s == eMouseButtonStateJustDown )
			itButton->second = eMouseButtonStateDown;
		if (s == eMouseWheelUp || s == eMouseWheelDown)
			m_bWheelConsumed = true;
		if ((b == TMouseButton::eMouseButtonLeft) && (s == eMouseButtonStateJustUp))
			m_bUpLButtonConsumed = true;
		return s;
	}
	return IInputManager::eMouseButtonStateNone;
}


//unsigned int CInputManager::GetMouseButtonState()
//{
//	return m_ButtonState;
//}

void CInputManager::AbonneToKeyEvent( CPlugin* pPlugin, TKeyCallback pfnCallback )
{
	m_vKeyCallback.push_back( pair< CPlugin*, TKeyCallback >::pair( pPlugin, pfnCallback ) );
}

CInputManager::KEY_STATE CInputManager::GetKeyState( unsigned char key )
{
	return m_mKeyboardState[ key ];
}

float CInputManager::GetMouseSensitivity()
{
	return m_fMouseSensitivity;
}

void CInputManager::SetMouseSensitivity( float s )
{
	m_fMouseSensitivity = s;
}

extern "C" _declspec(dllexport) IInputManager* CreateInputManager( const IInputManager::Desc& oDesc )
{
	return new CInputManager( oDesc );
}