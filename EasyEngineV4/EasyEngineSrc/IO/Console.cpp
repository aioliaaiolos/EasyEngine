#include "Console.h"

// System
#include <windows.h>
#include <string.h>

// stl
#include <sstream>
#include <algorithm>

// Engine
#include "Interface.h"
#include "IInputManager.h"
#include "ActionManager.h"
#include "IScriptManager.h"
#include "Exception.h"
#include "IGUIManager.h"

using namespace std;



CConsole::CConsole(EEInterface& oInterface):
m_bIsOpen(false),
m_xPos( 0 ),
m_yPos( 0 ),
m_oInputManager(static_cast<IInputManager&>(*oInterface.GetPlugin("InputManager"))),
m_oGUIManager(static_cast<IGUIManager&>(*oInterface.GetPlugin("GUIManager"))),
m_oScriptManager(static_cast<IScriptManager&>(*oInterface.GetPlugin("ScriptManager"))),
m_nWidth(500),
m_nHeight( 800 ),
m_nCurrentLineWidth( 0 ),
m_nCurrentHeight( 0 ),
m_nCursorPos( 0 ),
m_nCursorBlinkRate( 500 ),
m_bCursorBlinkState( true ),
m_nLastMillisecondCursorStateChanged( 0 ),
m_nLastTickCount( 0 ),
m_bBlink( false ),
m_nStaticTextID( -1 ),
m_nConsoleShortCut( 0 ),
m_bHasToUpdateStaticTest(false),
m_bInputEnabled(true),
m_nAutoCompletionLastIndexFound(-1),
m_bPauseMode(false),
m_PauseCallback(nullptr),
m_PauseCallbackParams(nullptr),
m_ResponseCallback(nullptr),
m_nBeginReponseCursorPos(0),
m_pResponseData(nullptr)
{
	m_oInputManager.AbonneToKeyEvent( static_cast< CPlugin* > ( this ), OnKeyAction );
	m_xPos = 30;
	m_yPos = 50;
	m_sLinePrefix = "> ";
	NewLine();
}

CConsole::~CConsole(void)
{
}

void CConsole::GetClipboardContent(string& text)
{
	// Try opening the clipboard
	if (OpenClipboard(nullptr)) {
		// Get handle of clipboard object for ANSI text
		HANDLE hData = GetClipboardData(CF_TEXT);
		if (hData != nullptr) {				
			// Lock the handle to get the actual text pointer
			char * pszText = static_cast<char*>(GlobalLock(hData));
			if (pszText != nullptr) {
				// Save text in a string class instance					
				text = pszText;
				// Release the lock
				GlobalUnlock(hData);
				// Release the clipboard
				CloseClipboard();
			}
		}
	}
	else
		Println("Error during get clipboard content");
}

void CConsole::SetBlink( bool blink )
{
	m_bBlink = blink;
	if( !m_bBlink )
		m_bCursorBlinkState = true;
}

void CConsole::Open( bool bOpen )
{
	m_bIsOpen = bOpen;
	m_oGUIManager.EnableStaticText( m_nStaticTextID, bOpen );
}

void CConsole::UpdateBlink( int nFontHeight )
{
	int nTickCount = GetTickCount();
	if( !m_bBlink )
		m_nLastTickCount = nTickCount;
	m_nLastMillisecondCursorStateChanged += ( nTickCount - m_nLastTickCount );
	m_nLastTickCount = nTickCount;
	if( m_nLastMillisecondCursorStateChanged > m_nCursorBlinkRate )
	{
		m_nLastMillisecondCursorStateChanged = 0;
		m_bCursorBlinkState = !m_bCursorBlinkState;
	}

	if( m_bCursorBlinkState && !m_bPauseMode ) {
		int nLastIndice = m_vLines.size() - 1;
		unsigned int PixelCursorPos = ComputePixelCursorPos() - 1;
		m_oGUIManager.Print( '|', m_xPos + PixelCursorPos, m_yPos + nLastIndice * nFontHeight);
	}
}

void CConsole::Update()
{
	if ( m_bIsOpen )
	{
		int nFontHeight = m_oGUIManager.GetCurrentFontHeight();
		if (m_bHasToUpdateStaticTest) {
			if (m_nStaticTextID != -1)
				m_oGUIManager.DestroyStaticTest(m_nStaticTextID);
			m_nStaticTextID = m_oGUIManager.CreateStaticText(m_vLines, m_xPos, m_yPos);
			m_bHasToUpdateStaticTest = false;
		}
		m_oGUIManager.PrintStaticText( m_nStaticTextID );
		string sLine;
		if (!m_bPauseMode)
			sLine = m_sLinePrefix;
		sLine += m_vLines[ m_vLines.size() - 1 ];
		if(m_bInputEnabled || (sLine.size()!=2) )
			m_oGUIManager.Print( sLine, m_xPos, m_yPos + ( m_vLines.size() - 1 ) * nFontHeight );
		
		if(m_bInputEnabled)
			UpdateBlink( nFontHeight );
	}
}

int CConsole::GetLineHeight()
{
	return m_oGUIManager.GetCurrentFontHeight();
}

int CConsole::GetClientHeight()
{
	return m_nHeight;
}

void CConsole::SetPauseModeOn(ResumeProc callback, void* params)
{
	m_PauseCallback = callback;
	m_PauseCallbackParams = params;
	m_bPauseMode = true;
	Println("");
	Println("Appuyez sur une touche pour continuer");
}

void CConsole::SetPauseModeOff()
{
	m_bPauseMode = false;
}

unsigned int CConsole::ComputePixelCursorPos()
{
	int nLastIndice = m_vLines.size() - 1;
	string sLastLine = m_sLinePrefix + m_vLines[ nLastIndice ];
	int nCursorPosInPixel = 0;
	for( int i = 0; i < m_nCursorPos + m_sLinePrefix.size(); i++ )
		nCursorPosInPixel += m_oGUIManager.GetCurrentFontWidth( sLastLine[ i ] ) + m_oGUIManager.GetCharSpace();
	return nCursorPosInPixel;
}

void CConsole::OnKeyAction( CPlugin* pPlugin, unsigned int key, IInputManager::KEY_STATE state )
{
	CConsole* pConsole = static_cast< CConsole* >( pPlugin );
	if ( !pConsole->m_bIsOpen )
		return;
	switch( state )
	{
	case IInputManager::JUST_PRESSED:
	case IInputManager::PRESSED:
		pConsole->OnKeyPress( key );
		break;
	case IInputManager::JUST_RELEASED:
	case IInputManager::RELEASED:
		pConsole->OnKeyRelease( key );
		break;
	}
}

void CConsole::OnPressEnter()
{
	string sCommand = m_vLines.back();
	if (m_ResponseCallback) {
		m_sResponse = sCommand.substr(m_nBeginReponseCursorPos);
		m_ResponseCallback(m_sResponse, m_pResponseData);
		m_ResponseCallback = nullptr;
		m_pResponseData = nullptr;
		return;
	}
	
	
	m_vLines.back() = m_sLinePrefix + m_vLines.back();
	if (m_nStaticTextID != -1)
		m_oGUIManager.DestroyStaticTest(m_nStaticTextID);
	vector< string >& vLines = m_vLines;
	m_vLines.resize(m_vLines.size() + 1);
	if (UpdateConsoleHeight())
		vLines.erase(vLines.begin());
	m_nStaticTextID = m_oGUIManager.CreateStaticText(vLines, m_xPos, m_yPos);
	
	m_nCursorPos = 0;
	try
	{
		if( sCommand.size() > 0 )
		{
			m_vLastCommand.push_back( sCommand );
			m_nCurrentCommandOffset = m_vLastCommand.size();
			m_oScriptManager.ExecuteCommand( sCommand );
		}
	}
	catch( CCompilationErrorException& e )
	{
		ostringstream ossMessage;
		string sErrorType;
		string sMessage;
		e.GetErrorMessage( sMessage );
		Println( sMessage );
	}
	catch( CScriptException& e )
	{
		AddString( "Erreur : " );
		AddString( e.what() );
	}
	catch (CFileNotFoundException& e) {
		string msg;
		e.GetErrorMessage(msg);
		Println(string("Erreur : '") + msg + "' introuvable");
	}
	catch (CFileException& e) {
		string msg;
		e.GetErrorMessage(msg);
		Println(string("Erreur a l'ouverture du ficher '") + msg + "'");
	}
	catch (CCharacterAlreadyExistsException& e) {
		string id;
		e.GetErrorMessage(id);
		Println(string("Erreur : le personnage '" + id + " existe deja."));
	}
	catch (CMethodNotImplementedException& e) {
		Println(string("Erreur : Utilisation de la methode '") + e.what() + " non implémentée.");
	}
	catch (CEException& e) {
		string msg;
		e.GetErrorMessage(msg);
		Println(msg);
	}
	catch (exception& e) {
		Println(e.what());
	}
}

void CConsole::InitCompletion()
{
	m_sCompletionPrefix = "";
	m_nAutoCompletionLastIndexFound = -1;
}

void CConsole::OnKeyPress( unsigned char key )
{
	if (!m_bInputEnabled)
		return;
	if (m_bPauseMode) {
		m_bPauseMode = false;
		if (m_PauseCallback)
			m_PauseCallback(nullptr);
		return;
	}
	SetBlink( false );
	string& sLine = m_vLines[ m_vLines.size() - 1 ];
	if ( key == VK_BACK )
	{
		InitCompletion();
		if ( sLine.size() > 0 && m_nCursorPos > 0 )
		{
			sLine.erase( sLine.begin() + m_nCursorPos - 1 );
			m_nCursorPos--;
		}
	}
	else if ( key == VK_RETURN )
		OnPressEnter();
	else if( key == VK_ESCAPE )
	{
		InitCompletion();
		m_nCursorPos = 0;
		sLine.clear();
	}
	else if( key == VK_END )
	{
		m_nCursorPos = sLine.size();
	}
	else if( key == VK_HOME )
	{
		m_nCursorPos = 0;
	}
	else if( key == VK_LEFT )
	{
		if( m_nCursorPos > 0 )
			m_nCursorPos--;		
	}
	else if( key == VK_RIGHT )
	{
		if( m_nCursorPos < sLine.size() )
			m_nCursorPos++;
	}
	else if( key == VK_UP || key == VK_DOWN ) // UP || DOWN
	{
		if( m_vLastCommand.size() > 0 )
		{
			int nCurrentCommand;
			if( key == 38 )
				--m_nCurrentCommandOffset;
			else
				++m_nCurrentCommandOffset;
			if( m_nCurrentCommandOffset >= (int)m_vLastCommand.size() )
				m_nCurrentCommandOffset = m_vLastCommand.size() - 1;
			if( m_nCurrentCommandOffset < 0 )
				m_nCurrentCommandOffset = 0;
			nCurrentCommand = m_nCurrentCommandOffset;
			ReplaceString( m_vLastCommand[ nCurrentCommand ] );
			m_nCursorPos = m_vLastCommand[ nCurrentCommand ].size();
		}
	}
	else if( key == VK_DELETE )
	{
		InitCompletion();
		sLine.erase( sLine.begin() + m_nCursorPos );
	}
	else if ( key != VK_SHIFT && key != VK_CONTROL && key != VK_MENU && key != m_nConsoleShortCut)
	{
		IInputManager::KEY_STATE LCtrlPressed = m_oInputManager.GetKeyState( VK_CONTROL );
		unsigned char c = 0;
		if (LCtrlPressed == IInputManager::JUST_PRESSED || LCtrlPressed == IInputManager::PRESSED) {
			if (m_oInputManager.GetKeyState(VK_SPACE) == IInputManager::JUST_PRESSED)
				ManageAutoCompletion();
			else if (key == 'V') {
				string text;
				GetClipboardContent(text);
				Print(text);
				m_nCursorPos = sLine.size();
			}
		}
		else
		{
			unsigned char kbs[256];
			GetKeyboardState(kbs);
			unsigned short ch;
			ToAscii(key, MapVirtualKey(key, MAPVK_VK_TO_VSC), kbs, &ch, 0);
			c = char(ch);
			InitCompletion();
		}
		if ( c != 0 )
		{
			sLine.insert( sLine.begin() + m_nCursorPos, c );
			m_nCursorPos++;
		}
	}
}

void CConsole::ManageAutoCompletion()
{
	vector< string > vFuncNames;
	m_oScriptManager.GetRegisteredFunctions( vFuncNames );
	if (m_sCompletionPrefix.empty()) {
		string sBegin = m_vLines[m_vLines.size() - 1];
		string sBeginLow = sBegin;
		transform(sBegin.begin(), sBegin.end(), sBeginLow.begin(), tolower);
		m_sCompletionPrefix = sBeginLow;
	}
	bool bFind = false;
	int i = m_nAutoCompletionLastIndexFound + 1;
	for( i ; i < vFuncNames.size(); i++ ) {
		string sFuncNameLow = vFuncNames[ i ];
		transform( vFuncNames[ i ].begin(), vFuncNames[ i ].end(), sFuncNameLow.begin(), tolower );
		if( sFuncNameLow.find(m_sCompletionPrefix) == 0 ) {
			if( bFind )	{
				bFind = false;
				break;
			}
			else {
				bFind = true;
				m_nAutoCompletionLastIndexFound = i;
				break;
			}
		}
	}
	if( bFind )	{
		m_vLines[ m_vLines.size() - 1 ] = vFuncNames[m_nAutoCompletionLastIndexFound];
		m_nCursorPos = m_vLines[ m_vLines.size() - 1 ].size();
	}
	if (i == vFuncNames.size())
		m_nAutoCompletionLastIndexFound = -1;
}

void CConsole::OnKeyRelease( unsigned char key )
{
	SetBlink( true );
}

void CConsole::ReplaceString( string s , int nLine )
{
	if( nLine == -1 )
		nLine = m_vLines.size() - 1;
	m_vLines[ nLine ] = s;
}

void CConsole::Close()
{
	//m_vLines[ m_vLines.size() - 1 ] = "";
	m_oGUIManager.EnableStaticText( m_nStaticTextID, false );
}

void CConsole::AddString( string s )
{
	if( s.size() == 0 )
		return;
	string sTemp;
	int nEndlnIndex = s.find( "\n" );
	if( nEndlnIndex != -1 )
	{
		sTemp = s.substr( 0, nEndlnIndex );
		m_vLines.back().insert( m_vLines.back().end(), sTemp.begin(), sTemp.end() );
		string sQueue = s.substr( nEndlnIndex + 1 );
		if( sQueue.size() > 0 )
		{
			NewLine();
			AddString( sQueue );
		}
	}
	else
		m_vLines.back().insert( m_vLines.back().end(), s.begin(), s.end() );
}

bool CConsole::UpdateConsoleHeight()
{
	bool bUpdated = false;
	m_nCurrentHeight = m_vLines.size() * m_oGUIManager.GetCurrentFontHeight();
	while (m_nCurrentHeight > m_nHeight)
	{
		m_vLines.erase(m_vLines.begin());
		m_nCurrentHeight = (int)m_vLines.size() * m_oGUIManager.GetCurrentFontHeight();
		bUpdated = true;
	}
	return bUpdated;
}

void CConsole::NewLine()
{
	m_vLines.resize( m_vLines.size() + 1 );
	m_nCurrentLineWidth = 0;
	UpdateConsoleHeight();
}

bool CConsole::IsOpen()
{
	return m_bIsOpen;
}

void CConsole::Cls()
{
	m_vLines.clear();
	m_vLines.push_back("");
	m_oGUIManager.DestroyStaticTest( m_nStaticTextID );
}

void CConsole::Print( string s )
{
	AddString(s);
	
}

void CConsole::Print(int i)
{
	ostringstream oss;
	oss << i;
	Println(oss.str());
	Print(oss.str());
}

void CConsole::Print(float f)
{
	ostringstream oss;
	oss << f;
	Println(oss.str());
	Print(oss.str());
}

void CConsole::Println(string s)
{
	Print(s);
	m_bHasToUpdateStaticTest = true;
	NewLine();
}

void CConsole::Println(int i)
{
	ostringstream oss;
	oss << i;
	Println(oss.str());
}

void CConsole::Println(float f)
{
	ostringstream oss;
	oss << f;
	Println(oss.str());
}

int CConsole::GetConsoleShortCut()
{
	return m_nConsoleShortCut;
}

void CConsole::SetConsoleShortCut(int key)
{
	m_nConsoleShortCut = key;
}

void CConsole::EnableInput(bool enable)
{
	m_bInputEnabled = enable;
}

string CConsole::GetName()
{
	return "Console";
}

void CConsole::WaitForResponse(ResponseProc callback, void* data)
{
	m_ResponseCallback = callback;
	m_nBeginReponseCursorPos = m_nCursorPos;
	m_pResponseData = data;
}

extern "C" _declspec(dllexport) IConsole* CreateConsole(EEInterface& oInterface)
{
	return new CConsole(oInterface);
}