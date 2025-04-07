#ifndef IGUIManager_H
#define IGUIManager_H

#include <string>
#include "EEPlugin.h"
#include "IObject.h"
#include <functional>

class IRenderer;
class IRessourceManager;
class IXMLParser;
class IInputManager;
class ILoaderManager;
class CGUIWidget;
class ICameraManager;
class IEntityManager;
class IScene;
class ITexture;
class ICharacter;
class CDimension;
struct IValue;

using namespace std;

class IGUIWidget
{
public:
	virtual void SetPosition(int x, int y) = 0;
};

class IGUIWindow : public virtual IGUIWidget
{
public:
	using CloseWindowCallback = function<void(IGUIWindow*)>;

	virtual void	Display() = 0;
	virtual void	Close() = 0;
	virtual void	SetCloseWindowCallback(CloseWindowCallback callback) = 0;
};

class ITopicWindow : public virtual IGUIWindow
{
public:
	virtual void SetSpeakerId(string sId) = 0;
	virtual void OnChoiceCalled(string sChoices) = 0;
	virtual void OnGoodbyeCalled() = 0;
	virtual void LoadTopics(string sFileName) = 0;
	virtual void SetSpeakerLocalVar(string sVarName, string sValue) = 0;
	virtual void SetSpeakerLocalVar(string sLocalVar, int nValue) = 0;
	virtual IValue* GetSpeakerLocalVar(string sVarName) = 0;
	virtual const string& GetSpeakerID() = 0;
};

class IInventoryWindow : public virtual IGUIWindow
{
public:
	virtual void DisplayItems(ICharacter* pCharacter) = 0;
};

class IGUIManager : public CPlugin
{
protected:
	IGUIManager() : CPlugin(nullptr,  ""){}

public:
	enum ENUM_EVENT
	{
		EVENT_NONE = 0,
		EVENT_MOUSEMOVE,
		EVENT_LMOUSECLICK,
		EVENT_LMOUSERELEASED,
		EVENT_RMOUSECLICK,
		EVENT_MOUSEENTERED,
		EVENT_MOUSEEXITED,
		EVENT_OUTSIDE
	};

	enum TFontColor 
	{
		eWhite = 0,
		eBlue,
		eTurquoise,
		eYellow,
		eRed
	};

	using EVENT_CALLBACK = std::function<void(ENUM_EVENT nEvent, CGUIWidget*, int, int)>;

	struct Desc : public CPlugin::Desc
	{
		IRenderer&				m_oRenderer;
		IRessourceManager&		m_oRessourceManager;
		IXMLParser&				m_oXMLParser;
		std::string				m_sShaderName;
		IInputManager&			m_oInputManager;
		ICameraManager&			m_oCameraManager;
		IEntityManager&			m_oEntityManager;
		IScene&					m_oScene;
		Desc( 
			IRenderer& oRenderer, 
			IRessourceManager& oRessourceManager, 
			IXMLParser& oXMLParser, 
			IInputManager& oInputManager,
			ICameraManager& oCameraManager,
			IEntityManager&	oEntityManager,
			IScene&	oScene):
			CPlugin::Desc( NULL, "" ),
			m_oRenderer( oRenderer ),
			m_oRessourceManager( oRessourceManager ),
			m_oXMLParser( oXMLParser ),
			m_oInputManager( oInputManager ),
			m_oCameraManager(oCameraManager),
			m_oEntityManager(oEntityManager),
			m_oScene(oScene){}
	};

	virtual void					AddWidget( int hWindow, int hWidget ) = 0;
	virtual int						CreateListener( EVENT_CALLBACK pfnCallback ) = 0;
	virtual void					AddEventListener( int hWidget, int hListener ) = 0;
	virtual bool					IsWindowDisplayed(IGUIWindow* pWindow) = 0;
	virtual void					AddWindow(IGUIWindow* pWindow) = 0;
	virtual void					RemoveWindow(IGUIWindow* pWindow) = 0;
	virtual void					SetVisibility( int hWindow, bool bVisible ) = 0;
	virtual void					OnRender() = 0;
	virtual void					Print( std::string sText, int x, int y, TFontColor color = eWhite) = 0;
	virtual void					Print( char c, int x, int y, IGUIManager::TFontColor color = eWhite) = 0;
	virtual void					SetActive( bool bActivate ) = 0;
	virtual bool					GetActive() = 0;
	virtual unsigned int			GetCurrentFontHeight() const = 0;
	virtual unsigned int			GetCurrentFontWidth( char c ) const = 0;
	virtual unsigned int			GetCharSpace() = 0;
	virtual int						CreateStaticText( vector< string >& vText, int nPosX = 0, int nPosY = 0, IGUIManager::TFontColor color = IGUIManager::TFontColor::eWhite) = 0;
	virtual void					DestroyStaticTest( int nID ) = 0;
	virtual void					PrintStaticText( int nTextID ) = 0;
	virtual void					EnableStaticText( int nTextID, bool bEnable ) = 0;
	virtual ITopicWindow*			GetTopicsWindow() = 0;
	virtual IGUIWindow*				CreatePlayerWindow(int nWidth, int nHeight) = 0;
	virtual void					SetGUIMode(bool bGUIMode) = 0;
	virtual bool					GetGUIMode() = 0;
	virtual void					ToggleDisplayMiniMap() = 0;
	virtual void					ToggleDisplayMiniMap2() = 0;
	virtual void					ToggleDisplayShadowMap() = 0;
	virtual void					CreateWidgetArrayFromString(string sText, TFontColor color, vector<CGUIWidget>& widgets) = 0;
	virtual ITexture*				GetColorTexture(TFontColor color) const = 0;
	virtual IInventoryWindow*		CreateInventoryWindow(const CDimension& windowSize) = 0;
	virtual void					OpenTradeWindow(ICharacter* pTrader, bool open) = 0;
};

#endif // IGUIManager_H