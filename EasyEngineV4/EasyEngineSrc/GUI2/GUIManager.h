#ifndef GUIMANAGER_H
#define GUIMANAGER_H

// stl
#include <sstream>
#include <map>
#include <string>
#include <set>

// Engine
#include "Listener.h"
#include "IGUIManager.h"
#include "ILoader.h"
 
class CGUIWindow;
class CGUIWidget;
class CLink;
class CRectangle;
class CDimension;
class CPosition;
class IShader;
class ITexture;
class IRessource;
class IAnimatableMesh;
class IMesh;
class CTopicsWindow;
class ICamera;
class IScene;

using namespace std;

struct CPoint
{
	float m_x;
	float m_y;
	CPoint( float x, float y ) : m_x( x ), m_y( y ){}
};



class CGUIManager : public IGUIManager
{
	struct CLine
	{
		vector< CGUIWidget >	m_vWidget;
		int						m_nPosX;
		int						m_nPosY;
	};

	
public:

						CGUIManager(EEInterface& oInterface);
	virtual				~CGUIManager();
	void				SetVisibility( int hWindow, bool bVisible );
	void				AddWidget( int hWindow, int hWidget );
	void				AddWindow(IGUIWindow* pWindow);
	bool				IsWindowDisplayed(IGUIWindow* pWindow);
	void				RemoveWindow(IGUIWindow* pWindow);
	void				SetPosition( int hWidget, int nx, int ny);	
	void				OnRender();
#ifdef 	DISPLAYCURSOR
	CDimension			GetCursorDimension()const;
#endif // DISPLAYCURSOR
	int					CreateListener( IGUIManager::EVENT_CALLBACK pfnCallBack );
	void				AddEventListener( int hWidget, int hListener);
	void				CreateWidgetArrayFromString(string sText, TFontColor color, vector<CGUIWidget>& widgets);
	void				CreateWidgetFromChar(char c, TFontColor color, CGUIWidget& widget);
	void				Print( string sText, int x, int y, TFontColor color = eWhite) override;
	void				Print( char c, int x, int y, IGUIManager::TFontColor color = eWhite) override;
	void				SetActive( bool bActivate );
	bool				GetActive();
	unsigned int		GetCurrentFontHeight() const;
	unsigned int		GetCurrentFontWidth( char c ) const;
	unsigned int		GetCharSpace();
	int					CreateStaticText(vector< string >& vText, int nPosX = 0, int nPosY = 0, IGUIManager::TFontColor color = IGUIManager::TFontColor::eWhite);
	CGUIWidget*			CreateStaticText(string sText, IGUIManager::TFontColor color = IGUIManager::TFontColor::eWhite);
	void				CreateStaticText(string sText, int nMaxWith, vector<CGUIWidget*>& vLineWidgets);
	void				CreateStaticText_(string sText, int nMaxCharacterPerLine, vector<CGUIWidget*>& vLineWidgets);
	IAnimatableMesh*	CreateTextMeshes(string sText, IGUIManager::TFontColor color = IGUIManager::TFontColor::eWhite);
	void				DestroyStaticTest( int nID );
	void				PrintStaticText( int nTextID );
	void				EnableStaticText( int nTextID, bool bEnable );
	ITopicWindow*		GetTopicsWindow();
	void				ToggleDisplayMap();
	void				SetGUIMode(bool bGUIMode);
	bool				GetGUIMode();
	int					GetCurrentFontEspacementX();
	int					GetCurrentFontEspacementY();
	int					GetLetterEspacementX(char c);
	string				GetName() override;
	ITexture*			GetColorTexture(TFontColor color) const override;

private:
	ILoaderManager*								m_pLoaderManager;
	bool										m_bActive;
	bool										m_bGUIMode;

#ifdef DISPLAYCURSOR
	CGUIWidget*									m_pCursor;
#endif // DISPLAYCURSOR

	vector< CLine >								m_vText;
	map<unsigned char, CGUIWidget*>				m_mWidgetFontWhite;
	map<unsigned char, CGUIWidget*>				m_mWidgetFontBlue;
	map<unsigned char, CGUIWidget*>				m_mWidgetFontTurquoise;
	const map<unsigned char, CGUIWidget*>* 		m_pCurrentFont;
	map<unsigned char, ILoader::CMeshInfos>		m_mWidgetFontInfos;
	ILoader::CMeshInfos							m_oLastWidgetInfosCreated;
	EEInterface&								m_oInterface;
	IRenderer&									m_oRenderer;
	IRessourceManager&							m_oRessourceManager;
	ICameraManager&								m_oCameraManager;
	IEntityManager&								m_oEntityManager;
	IXMLParser&									m_oXMLParser;
	IInputManager&								m_oInputManager;
	IScene*										m_pScene;
	IRessource*									m_pWhiteFontMaterial;
	IRessource*									m_pBlueFontMaterial;
	IRessource*									m_pTurquoiseFontMaterial;
	map< int, IAnimatableMesh* >				m_mStaticText;
	std::map< int, CGUIWidget* >				m_mWidget;
	std::map< int, CListener* >					m_mListener;
	CTopicsWindow*								m_pTopicsWindow;
	CGUIWindow*									m_pMapWindow;
	bool										m_bDisplayMap;
	map<IGUIManager::TFontColor, IRessource*>	m_mFontMaterialByColor;
	map<IGUIManager::TFontColor, map<unsigned char, CGUIWidget*>>	m_mFontWidgetByColor;
	map<TFontColor, ITexture*>					m_mFontColor;

	void									GetScreenCoordFromTexCoord(const CRectangle& oTexture, const CDimension& oScreenDim, CRectangle& oScreen) const;
	void									InitFontMap();
	CGUIWidget*								GetWidgetByHandle(int hWidget) const;
	CGUIWidget*								CreateImageFromSkin(const std::string& szFileName, unsigned int nWidth = 0, unsigned int nHeight = 0);
	int										InsertWidgetInMap(CGUIWidget* pWidget);
	bool									IsVisible(CGUIWindow* pWindow);
	CDimension								GetDimension(CGUIWidget* pWidget);
	void									CreateFontBitmap(string FontName, int nSize, vector< unsigned char >& vData, vector< CPoint >& vCharSize, int r, int g, int b);
	void									FlipBitmap(const unsigned char* data, int w, int h, int depth, vector< unsigned char >& vData);
	IGUIWindow*								CreatePlayerWindow(int nWidth, int nHeight);
	void									RenderText();
	void									AddTextToMeshInfos(string sText, int nPosX, int nPosY, float fOffsetY, int& nNumChar, ILoader::CMeshInfos& mi, float& fOffset, IGUIManager::TFontColor color = IGUIManager::eWhite);
	IRessource*								GetFontMaterial(IGUIManager::TFontColor color);
	const map<unsigned char, CGUIWidget*>&	GetFontWidget(IGUIManager::TFontColor color) const;

	IShader*								m_pShader;
	int										m_nCharspace;
	map< int, bool >						m_mStaticTextToRender;
	set<CGUIWindow*>						m_DisplayedWindowsSet;
};

extern "C" _declspec(dllexport) IGUIManager* CreateGUIManager(EEInterface& oInterface);
#endif