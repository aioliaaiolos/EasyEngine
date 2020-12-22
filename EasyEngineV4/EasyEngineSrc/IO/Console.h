#ifndef CONSOLE_H
#define CONSOLE_H

// stl
#include <map>
#include <vector>
#include <string>

// Engine
#include "IInputManager.h"
#include "IConsole.h"

using namespace std;

class IInputManager;
class IActionManager;
class IScriptManager;
class IGUIManager;
class IPlugin;

class CConsole : public IConsole
{
	bool											m_bIsOpen;
	std::vector< std::string >						m_vLines;
	IInputManager&									m_oInputManager;
	IScriptManager&									m_oScriptManager;
	IGUIManager&									m_oGUIManager;
	int												m_xPos;
	int												m_yPos;
	int												m_nWidth;
	unsigned int									m_nHeight;
	int												m_nCurrentLineWidth;
	int												m_nCurrentHeight;
	vector< string >								m_vLastCommand;
	int												m_nCurrentCommandOffset;
	int												m_nCursorPos;
	string											m_sLinePrefix;
	int												m_nCursorBlinkRate;
	bool											m_bCursorBlinkState;
	int												m_nLastMillisecondCursorStateChanged;
	int												m_nLastTickCount;
	bool											m_bBlink;
	int												m_nStaticTextID;
	int												m_nConsoleShortCut;
	bool											m_bHasToUpdateStaticTest;
	bool											m_bInputEnabled;
	int												m_nAutoCompletionLastIndexFound;
	string											m_sCompletionPrefix;

	void											OnKeyPress( unsigned char key );
	void											OnKeyRelease( unsigned char key );
	void											AddString( string s );
	void											NewLine();
	void											ReplaceString( string s, int nLine = -1 );
	unsigned int									ComputePixelCursorPos();
	void											ManageAutoCompletion();
	void											UpdateBlink( int nFontHeight );
	void											OnPressEnter();
	void											GetClipboardContent(string& text);
	void											InitCompletion();
	
	static void										OnKeyAction( CPlugin*, unsigned int key, IInputManager::KEY_STATE );	


public:

	
													CConsole( const IConsole::Desc& oDesc );
	virtual											~CConsole(void);

	void											Open();
	bool											IsOpen();
	void											Update();
	void											Close();
	void											Cls();
	void											Open( bool bOpen );
	void											Print(string s);
	void											Print(int i);
	void											Println(string s);
	void											Println(int i);
	void											Print2D(string s);
	void											SetBlink( bool blink );
	int												GetConsoleShortCut();
	void											SetConsoleShortCut(int key);
	void											EnableInput(bool enable);	
};

extern "C" _declspec(dllexport) IConsole* CreateConsole( IConsole::Desc& oDesc );

#endif // CONSOLE_H