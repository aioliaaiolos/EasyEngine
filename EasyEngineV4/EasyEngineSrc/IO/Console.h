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
public:
	
													CConsole(EEInterface& oInterface);
	virtual											~CConsole(void);

	void											Open();
	bool											IsOpen();
	void											Update();
	void											Close();
	void											Cls();
	void											Open( bool bOpen );
	void											Print(string s);
	void											Print(int i);
	void											Print(float f);
	void											Println(string s);
	void											Println(int i);
	void											Println(float f);
	void											Print2D(string s);
	void											SetBlink( bool blink );
	int												GetConsoleShortCut();
	void											SetConsoleShortCut(int key);
	void											EnableInput(bool enable);
	string											GetName() override;
	void											WaitForResponse(ResponseProc callback, void* data) override;

private:

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
	bool											m_bPauseMode;
	ResumeProc										m_PauseCallback;
	void*											m_PauseCallbackParams;
	ResponseProc									m_ResponseCallback;
	string											m_sResponse;
	int												m_nBeginReponseCursorPos;
	void*											m_pResponseData;

	void											OnKeyPress(unsigned char key);
	void											OnKeyRelease(unsigned char key);
	void											AddString(string s, int nPos = -1);
	void											NewLine();
	void											ReplaceString(string s, int nLine = -1);
	unsigned int									ComputePixelCursorPos();
	void											ManageAutoCompletion();
	void											UpdateBlink(int nFontHeight);
	void											OnPressEnter();
	void											GetClipboardContent(string& text);
	void											InitCompletion();
	int												GetLineHeight() override;
	int												GetClientHeight() override;
	void											SetPauseModeOn(ResumeProc callback, void* params) override;
	void											SetPauseModeOff();
	bool											UpdateConsoleHeight();

	static void										OnKeyAction(CPlugin*, unsigned int key, IInputManager::KEY_STATE);
};

extern "C" _declspec(dllexport) IConsole* CreateConsole(EEInterface& oInterface);

#endif // CONSOLE_H