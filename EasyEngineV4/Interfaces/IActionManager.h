#ifndef IACTIONMANAGER_H
#define IACTIONMANAGER_H

#include "EEPlugin.h"
#include "IInputManager.h"

using namespace std;

enum TMouse 
{
	AXIS_H = 0,
	AXIS_V
};


class IActionManager : public CPlugin
{
protected:
	IActionManager( const Desc& oDesc ) : CPlugin( oDesc.m_pParent, oDesc.m_sName ){}

public:

	struct Desc : public CPlugin::Desc
	{
		EEInterface& m_oInterface;
		Desc(EEInterface& oInterface) :
			CPlugin::Desc( NULL, "ActionManager" ),
			m_oInterface(oInterface){}
	};

	virtual void 								AddKeyAction( string sActionName, WPARAM key) = 0;
	virtual void								AddMouseAction( string sActionName, IInputManager::TMouseButton b, IInputManager::TMouseButtonState s ) = 0;
	virtual void 								AddGUIAction( const char* szActionName, WPARAM key ) = 0;
	virtual IInputManager::KEY_STATE			GetKeyActionState( string sActionName ) = 0;
	virtual IInputManager::TMouseButtonState	GetMouseActionState( string sActionName ) = 0;
	virtual void 								GetGameMousePos( int& x, int& y ) = 0;
	virtual void 								SetGameMousePos( int x, int y ) = 0;
	virtual void								ForceActionState( string sActionName, IInputManager::KEY_STATE eState ) = 0;
};

#endif // IACTIONMANAGER_H