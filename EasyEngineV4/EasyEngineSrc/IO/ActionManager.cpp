#include "ActionManager.h"
#include "Interface.h"

using namespace std;


CActionManager::CActionManager(IActionManager::Desc& oDesc ) :
IActionManager( oDesc ),
m_fMouseSensibility(0.03f),
m_oInputManager(static_cast<IInputManager&>(*oDesc.m_oInterface.GetPlugin("InputManager")))
{
	m_oInputManager.AbonneToKeyEvent( static_cast< CPlugin* > ( this ), OnKeyAction );
	IEventDispatcher& oEventDspatcher = static_cast<IEventDispatcher&>(*oDesc.m_oInterface.GetPlugin("EventDispatcher"));

	// Not working, need to execute it before getKeyState
	/*
	oEventDspatcher.AbonneToWindowEvent(this, [](CPlugin* pPlugin, IEventDispatcher::TWindowEvent e, int x, int y) 
	{
		CActionManager* pActionManager = static_cast<CActionManager*>(pPlugin);
		for (map< unsigned int, IInputManager::KEY_STATE >::iterator itKey = pActionManager->m_mKeyboardState.begin(); 
			itKey != pActionManager->m_mKeyboardState.end(); itKey++) {
			if (itKey->second == IInputManager::KEY_STATE::JUST_PRESSED)
				itKey->second = IInputManager::KEY_STATE::PRESSED;
			else if (itKey->second == IInputManager::KEY_STATE::JUST_RELEASED)
				itKey->second = IInputManager::KEY_STATE::RELEASED;
		}
	});*/
}

CActionManager::~CActionManager(void)
{
}

void CActionManager::ForceActionState( string sActionName, IInputManager::KEY_STATE eState )
{
	Map_string_WPARAM::iterator itAction = m_MapKeyActionState.find( sActionName );
	map< unsigned int, IInputManager::KEY_STATE >::iterator itKey = m_mKeyboardState.find( itAction->second );
	itKey->second = eState;
}

string CActionManager::GetName()
{
	return "ActionManager";
}

void CActionManager::OnKeyAction( CPlugin* pPlugin, unsigned int key, IInputManager::KEY_STATE state )
{
	CActionManager* pActionManager = static_cast< CActionManager* > ( pPlugin );
	pActionManager->m_mKeyboardState[ key ] = state;
}

IInputManager::KEY_STATE CActionManager::GetKeyActionState( string sActionName )
{
	Map_string_WPARAM::iterator itFind = m_MapKeyActionState.find( sActionName );
	if ( itFind == m_MapKeyActionState.end() )
		return IInputManager::NONE;
	WPARAM key = itFind->second;
	IInputManager::KEY_STATE KeyState = m_mKeyboardState[ key ];
	return KeyState;
}

IInputManager::TMouseButtonState CActionManager::GetMouseActionState( string sActionName )
{
	TStringMouseButton::iterator itMouse = m_mMouseActionButtonState.find( sActionName );
	if( itMouse == m_mMouseActionButtonState.end() )
		return IInputManager::eMouseButtonStateNone;	
	return m_oInputManager.GetMouseButtonState( itMouse->second.first );
}

void CActionManager::AddKeyAction( string sActionName, WPARAM key )
{
	m_MapKeyActionState.insert( Map_string_WPARAM::value_type( sActionName, key ) );
}

void CActionManager::AddMouseAction( string sActionName, IInputManager::TMouseButton b, IInputManager::TMouseButtonState s )
{
	m_mMouseActionButtonState.insert( TStringMouseButton::value_type( sActionName, pair< IInputManager::TMouseButton, IInputManager::TMouseButtonState >( b, s ) ) );
}

IInputManager::KEY_STATE CActionManager::GetGUIActionState(const char* szActionName)
{
	string strActionName = string(szActionName);
	Map_string_WPARAM::iterator itFind = m_MapGUIKeyActionState.find(strActionName);
	WPARAM key = itFind->second;
	IInputManager::KEY_STATE KeyState = m_mKeyboardState[ key ];
	return KeyState;
}

void CActionManager::AddGUIAction(const char* szActionName, WPARAM key)
{
	string strActionName = string(szActionName);
	m_MapGUIKeyActionState.insert(Map_string_WPARAM::value_type(strActionName, key));
}


void CActionManager::AddGameAction(const char* szActionName, TMouse axis)
{
	string strActionName = string(szActionName);
	m_MapMouseActionState.insert(Map_string_AXIS::value_type(strActionName, axis));
}

void CActionManager::AddGUIAction(const char* szActionName, TMouse axis)
{
	string strActionName = string(szActionName);
	m_MapGUIMouseActionState.insert(Map_string_AXIS::value_type(strActionName, axis));
}

void CActionManager::GetGameMousePos( int& x, int& y )
{				
	m_oInputManager.GetVirtualCursorPos( x, y );
	x = (int) ( x * m_fMouseSensibility );
	//x *= m_fMouseSensibility;
	y = (int) ( y * m_fMouseSensibility );
	if ( x >= 36000 )
		x -= 36000;
	if ( x  <= 0 )
		x += 36000;

	if ( y > 90 )
		y = 90;
	if ( y < -90 )
		y = -90;
}

void CActionManager::SetGameMousePos( int x, int y )
{
	m_oInputManager.SetMouseCursorPos( (int)( (float)x / m_fMouseSensibility ), ( int ) ( (float)y / m_fMouseSensibility ) );
}
//
//CPosition CActionManager::GetGUIMousePos() const
//{
//	return m_pInputManager->GetCursorPos();
//}

void CActionManager::SetGameMouseSensibility( float fSensibility )
{
	m_fMouseSensibility = fSensibility;
}

void CActionManager::SetGUIMouseSensibility( float fSensibility )
{
	m_fGUIMouseSensibility = fSensibility;
}

extern "C" _declspec(dllexport) IActionManager* CreateActionManager( IActionManager::Desc& oDesc )
{
	return new CActionManager( oDesc );
}