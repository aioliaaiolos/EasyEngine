#define GUIMANAGER_CPP

#include <algorithm>
#include "Interface.h"
#include "Guimanager.h"
#include "IXMLParser.h"
#include "exception.h"
#include "GUIWindow.h"
#include "IInputManager.h"
#include "IGUIManager.h"
#include "IRenderer.h"
#include "IShader.h"
#include "IRessource.h"
#include "ICamera.h"
#include "IEntity.h"
#include "IFilesystem.h"
#include "PlayerWindow.h"
#include "TopicsWindow.h"
#include "MapWindow.h"
#include "Utils2/Chunk.h"
#include "Utils2/Dimension.h"
#include "Utils2/Position.h"
#include "Utils2/Rectangle.h"


class CMaterial;
using namespace std;


//-----------------------------------------------------------------------------------------------------
//										Constructor
//-----------------------------------------------------------------------------------------------------


CGUIManager::CGUIManager(EEInterface& oInterface):
IGUIManager(),
m_oInterface(oInterface),
m_oRenderer(static_cast<IRenderer&>(*oInterface.GetPlugin("Renderer"))),
m_oRessourceManager(static_cast<IRessourceManager&>(*oInterface.GetPlugin("RessourceManager"))),
m_oXMLParser(static_cast<IXMLParser&>(*oInterface.GetPlugin("XMLParser"))),
m_oInputManager(static_cast<IInputManager&>(*oInterface.GetPlugin("InputManager"))),
m_oCameraManager(static_cast<ICameraManager&>(*oInterface.GetPlugin("CameraManager"))),
m_oEntityManager(static_cast<IEntityManager&>(*oInterface.GetPlugin("EntityManager"))),
m_bActive( true ),
m_nCharspace( 1 ),
m_bGUIMode(false),
m_bDisplayMap(false),
m_pScene(nullptr)
{
	ISceneManager* pSceneManager = static_cast<ISceneManager*>(oInterface.GetPlugin("SceneManager"));
	m_pScene = dynamic_cast<IScene*>(pSceneManager->GetScene("Game"));
	unsigned int nResWidth, nResHeight;
	m_oRenderer.GetResolution( nResWidth, nResHeight );	
	m_pShader = m_oRenderer.GetShader( "GUI" );
	CGUIWidget::Init( nResWidth, nResHeight, m_pShader );

#ifdef DISPLAYCURSOR
	m_pCursor = CreateImageFromSkin("GUI.CURSOR", 20,29 );
#endif // DISPLAYCURSOR
	InitFontMap();
	m_pTopicsWindow = new CTopicsWindow(oInterface, 900, 800);
	m_pMapWindow = new CMinimapWindow(oInterface, *m_pScene, 512, 512);
}


//-----------------------------------------------------------------------------------------------------
//										Destructor
//-----------------------------------------------------------------------------------------------------
CGUIManager::~CGUIManager(void)
{
	for( std::map< unsigned char, CGUIWidget* >::iterator it = m_mWidgetFontWhite.begin(); it != m_mWidgetFontWhite.end(); it++ )
		delete it->second;
	for (std::map< unsigned char, CGUIWidget* >::iterator it = m_mWidgetFontBlue.begin(); it != m_mWidgetFontBlue.end(); it++)
		delete it->second;
}

unsigned int CGUIManager::GetCharSpace()
{
	return m_nCharspace;
}

void CGUIManager::InitFontMap()
{
	CRectangle rect;
	rect.SetPosition( CPosition( 0, 0 ) );
	rect.SetDimension( CDimension( 256, 256 ) );
	
	vector< unsigned char > vDataWhite, vDataBlue, vDataTurquoise;
	vector< CPoint > vCharSize;
	CDimension dim = rect.m_oDim;
	CreateFontBitmap("Arial", dim.GetWidth(), vDataWhite, vCharSize, 255, 255, 255 );
	CreateFontBitmap("Arial", dim.GetWidth(), vDataBlue, vCharSize, 255, 0, 0);
	CreateFontBitmap("Arial", dim.GetWidth(), vDataTurquoise, vCharSize, 255, 255, 0);
	IShader* pShader = m_oRenderer.GetShader( "gui");
	ITexture* pFontTextureWhite = m_oRessourceManager.CreateTexture2D(pShader, 3, vDataWhite, dim.GetWidth(), dim.GetHeight(), IRenderer::T_RGBA);
	ITexture* pFontTextureBlue  = m_oRessourceManager.CreateTexture2D(pShader, 3, vDataBlue,  dim.GetWidth(), dim.GetHeight(), IRenderer::T_RGBA);
	ITexture* pFontTextureTurquoise = m_oRessourceManager.CreateTexture2D(pShader, 3, vDataTurquoise, dim.GetWidth(), dim.GetHeight(), IRenderer::T_RGBA);
	
	CRectangle char0( 0, 12 * (float)rect.m_oDim.GetHeight() / 16.f, vCharSize[ 48 ].m_x, vCharSize[  48 ].m_y );

	for( int i = 0; i < 16; i++ )
	{
		for( int j = 0; j < 16; j++ )
		{
			char c = (char) ( i * 16 + j );
			CRectangle charRect( j * 16, 16 * i, vCharSize[ i * 16 + j ].m_x, vCharSize[ i * 16 + j ].m_y );			
			m_mWidgetFontBlue[c] = new CGUIWidget(m_oInterface, pFontTextureBlue, charRect);
			m_mWidgetFontTurquoise[c] = new CGUIWidget(m_oInterface, pFontTextureTurquoise, charRect);
			m_mWidgetFontWhite[c] = new CGUIWidget(m_oInterface, pFontTextureWhite, charRect, m_mWidgetFontInfos[c], m_pFontMaterial);
		}
	}
}

void CGUIManager::GetScreenCoordFromTexCoord( const CRectangle& oTexture, const CDimension& oScreenDim, CRectangle& oScreen ) const
{
	int nResWidth = oScreenDim.GetWidth(), nResHeight = oScreenDim.GetHeight();
	oScreen.m_oPos.SetX( -1 );
	float fWidthRatio = 2 * (float)oTexture.m_oDim.GetWidth() / (float)nResWidth;
	float fHeightRatio = 2 * (float)oTexture.m_oDim.GetHeight() / (float)nResHeight;
	oScreen.m_oPos.SetY( 1 - fHeightRatio );
	oScreen.m_oDim.SetWidth( fWidthRatio );
	oScreen.m_oDim.SetHeight( fHeightRatio );
}

void CGUIManager::FlipBitmap( const unsigned char* data, int w, int h, int depth, vector< unsigned char >& vData )
{	
	for( int i = 0; i < h; i++ )
	{
		for( int j = 0; j < w; j++ )
		{
			for( int k = 0; k < depth; k++ )
			{
				int iIndice = 3 * ( w * ( i + 1 ) - ( j + 1 ) ) + k;
				vData.push_back( data[ iIndice  ] );
			}
		}
	}
}

void CGUIManager::CreateFontBitmap( string FontName, int nSize, vector< unsigned char >& vData, vector< CPoint >& vCharSize, int r, int g, int b )
{
	int Quality = nSize / 16;
	HDC hDC = CreateCompatibleDC( NULL );
	BITMAPINFO bi;
	memset( &bi, 0, sizeof(BITMAPINFO));
	bi.bmiHeader.biHeight = nSize;
	bi.bmiHeader.biWidth = nSize;
	bi.bmiHeader.biBitCount = 24;
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biPlanes = 1;
	unsigned char* data = NULL;
	HBITMAP hBitmap = CreateDIBSection( hDC, &bi, DIB_RGB_COLORS, reinterpret_cast< void** >( &data ), NULL, 0 );
	HFONT hFont = CreateFont(Quality, 0, 0, 0, FW_NORMAL, FALSE,
                                  FALSE, FALSE, DEFAULT_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  ANTIALIASED_QUALITY, DEFAULT_PITCH,
                                  FontName.c_str());

	SelectObject(hDC, hBitmap);
	SelectObject(hDC, hFont);

	SetBkColor(hDC, RGB(0, 0, 0));
	SetTextColor( hDC, RGB(r, g, b) );

    char Character = 0;
    for (int j = 0; j < 16; ++j)
	{
        for (int i = 0; i < 16; ++i, ++Character)
        {
            // Stockage de la taille du caractère
            SIZE Size;
            GetTextExtentPoint32(hDC, &Character, 1, &Size);
			vCharSize.push_back( CPoint( Size.cx, Size.cy ) );

            // Affichage du caractère
            RECT Rect = {i * Quality, j * Quality, (i + 1) * Quality, (j + 1) * Quality};
            DrawText(hDC, &Character, 1, &Rect, DT_LEFT);
        }
	}
	vector< unsigned char > vData2;
	FlipBitmap( data, bi.bmiHeader.biWidth, bi.bmiHeader.biHeight, bi.bmiHeader.biBitCount / 8, vData2 );
	
	for( unsigned int i = 0; i < vData2.size() / 3; i++ )
	{
		for( int j = 0; j < 3; j++ )
			vData.push_back( vData2[ 3 * i + j ] );		
		if( vData2[ 3 * i ] == 0 && vData2[ 3 * i + 1 ] == 0 && vData2[ 3 * i + 2 ] == 0 )
			vData.push_back( 0 );
		else
			vData.push_back( 255 );
	}
}

CGUIWidget* CGUIManager::CreateImageFromSkin( const string& sSkinPath, unsigned int nWidth, unsigned int nHeight )
{
	size_t nDotPos = sSkinPath.find( "." );
	string sImageName = sSkinPath.substr( 0, nDotPos ) + ".tga";
	ITexture* pTexture = static_cast< ITexture* > ( m_oRessourceManager.GetRessource(sImageName) );
	pTexture->SetShader( m_oRenderer.GetShader( "gui" ) );
	unsigned int nResWidth, nResHeight;
	m_oRenderer.GetResolution( nResWidth, nResHeight );
	
	ILoader::CMeshInfos mi;
	
	float fNewX = -1.f + 2.f*(float)nWidth/(float)nResWidth;
	float fNewY = 1.f - 2.f*(float)nHeight/(float)nResHeight;

	mi.m_vVertex.push_back( -1.f ); mi.m_vVertex.push_back( fNewY ); mi.m_vVertex.push_back( 0 );	
	mi.m_vVertex.push_back( -1.f ); mi.m_vVertex.push_back( 1.f	);  mi.m_vVertex.push_back( 0 );	
	mi.m_vVertex.push_back( fNewX ); mi.m_vVertex.push_back( 1.f ); mi.m_vVertex.push_back( 0 ); 
	mi.m_vVertex.push_back( fNewX ); mi.m_vVertex.push_back( fNewY ); mi.m_vVertex.push_back( 0 );

	//vector< unsigned int > vIndexArray;
	mi.m_vIndex.push_back( 1 ); mi.m_vIndex.push_back( 0 ); mi.m_vIndex.push_back( 2 );
	mi.m_vIndex.push_back( 3 ); mi.m_vIndex.push_back( 2 ); mi.m_vIndex.push_back( 0 );
	
	string strXMLName = sSkinPath.substr( 0, nDotPos ) + ".xml";
	string sPropertyName = sSkinPath.substr( nDotPos+1, sSkinPath.size() - 1 );
	
	m_oXMLParser.OpenFile( strXMLName );
	CPosition SkinPos;
	int nSkinDimWidth = 0;
	int nSkinDimHeight = 0;
	try
	{
		m_oXMLParser.GetProperty( sPropertyName, SkinPos, nSkinDimWidth, nSkinDimHeight );
		m_oXMLParser.CloseFile();
	}
	catch (CXMLParserException& e)
	{
		string sMessage = sSkinPath + " not found";
		MessageBoxA( NULL, sMessage.c_str(), "skinset not found" , MB_ICONERROR);
	}
	if (nWidth == 0 && nHeight == 0)
	{
		nWidth = nSkinDimWidth;
		nHeight = nSkinDimHeight;
	}
	//vector< float > vUVVertexArray;
	int nTexDimWidth = 0, nTexDimHeight = 0;
	pTexture->GetDimension( nTexDimWidth, nTexDimHeight );
	float fTexMinX, fTexMinY, fTexMaxX, fTexMaxY;
	fTexMinX = (float)SkinPos.GetX() / (float)nTexDimWidth;
	fTexMaxX = ((float)SkinPos.GetX() + (float)nSkinDimWidth )/ (float)nTexDimWidth;

	fTexMaxY = 1- ((float)SkinPos.GetY()  / (float)nTexDimHeight );	
	fTexMinY = 1- (((float)SkinPos.GetY()+ (float)nSkinDimHeight ) / (float)nTexDimHeight );

	mi.m_vUVVertex.push_back( fTexMinX ); mi.m_vUVVertex.push_back( fTexMinY ), mi.m_vUVVertex.push_back( 0 );
	mi.m_vUVVertex.push_back( fTexMinX ); mi.m_vUVVertex.push_back( fTexMaxY ), mi.m_vUVVertex.push_back( 0 );
	mi.m_vUVVertex.push_back( fTexMaxX ); mi.m_vUVVertex.push_back( fTexMaxY ), mi.m_vUVVertex.push_back( 0 );
	mi.m_vUVVertex.push_back( fTexMaxX ); mi.m_vUVVertex.push_back( fTexMinY ), mi.m_vUVVertex.push_back( 0 );

	//vector< unsigned int > vUVIndexArray;
	mi.m_vUVIndex.push_back( 1 ); mi.m_vUVIndex.push_back( 0 ); mi.m_vUVIndex.push_back( 2 );
	mi.m_vUVIndex.push_back( 3 ); mi.m_vUVIndex.push_back( 2 ); mi.m_vUVIndex.push_back( 0 );

	//vector< float > vNormalVertexArray;
	mi.m_vNormalVertex.push_back( 0 ); mi.m_vNormalVertex.push_back( 0 ); mi.m_vNormalVertex.push_back( 1 );
	mi.m_vNormalVertex.push_back( 0 ); mi.m_vNormalVertex.push_back( 0 ); mi.m_vNormalVertex.push_back( 1 );
	mi.m_vNormalVertex.push_back( 0 ); mi.m_vNormalVertex.push_back( 0 ); mi.m_vNormalVertex.push_back( 1 );
	mi.m_vNormalVertex.push_back( 0 ); mi.m_vNormalVertex.push_back( 0 ); mi.m_vNormalVertex.push_back( 1 );
	mi.m_vNormalVertex.push_back( 0 ); mi.m_vNormalVertex.push_back( 0 ); mi.m_vNormalVertex.push_back( 1 );
	mi.m_vNormalVertex.push_back( 0 ); mi.m_vNormalVertex.push_back( 0 ); mi.m_vNormalVertex.push_back( 1 );

	IRessource* pMaterial = NULL;//m_oRessourceManager.CreateMaterial( pTexture, m_oRenderer, "gui" );

	ILoader::CMaterialInfos oMaterialInfos;
	oMaterialInfos.m_sName = "gui";

	ILoader::CAnimatableMeshData oData;
	oData.m_vMeshes.push_back( mi );
	IRessource* pMesh = m_oRessourceManager.CreateMesh( oData, pMaterial );
	
	CGUIWidget* pWidget = new CGUIWidget( nWidth, nHeight );
	pWidget->SetQuad( pMesh );
	pWidget->SetSkinName( sPropertyName );	
	return pWidget;
}

//-----------------------------------------------------------------------------------------------------
//										SetVisibility
//-----------------------------------------------------------------------------------------------------
void CGUIManager::SetVisibility( int hWindow, bool bVisible )
{
	CGUIWindow* pWindow = dynamic_cast< CGUIWindow* > ( GetWidgetByHandle( hWindow ) );
	if ( pWindow == NULL )
	{
		exception e( "Invalid CGUIWindow handle" );
		throw e;
	}
	pWindow->SetVisibility( bVisible );
}

void CGUIManager::AddWidget( int hWindow, int hWidget )
{
	CGUIWindow* pWindow = dynamic_cast< CGUIWindow* > ( GetWidgetByHandle( hWindow ) );
	if ( pWindow == NULL )
	{
		exception e( "CGUIManager::AddWidget : Window's handle is invalid" );
		throw e;
	}
	CGUIWidget* pWidget = GetWidgetByHandle( hWidget );
	pWindow->AddWidget( pWidget );
}

//-----------------------------------------------------------------------------------------------------
//										SetPosition
//-----------------------------------------------------------------------------------------------------
void CGUIManager::SetPosition( int hWidget, int nx, int ny)
{
	CGUIWidget* pWidget = GetWidgetByHandle( hWidget );
	pWidget->SetPosition(static_cast<float>(nx), static_cast<float>(ny));
}

//-----------------------------------------------------------------------------------------------------
//										Update
//-----------------------------------------------------------------------------------------------------
void CGUIManager::OnRender()
{
	m_oRenderer.EnableZTest( false );
	if ( m_bActive )
	{
		int nCursorXPos, nCursorYPos;
		m_oInputManager.GetPhysicalCursorPos( nCursorXPos, nCursorYPos );
		m_pShader->Enable( true );
		m_oRenderer.EnableTextureBlend( true );

		// test
		int x, y;
		m_oInputManager.GetVirtualCursorPos( x, y );
		unsigned int nResWidth, nResHeight;
		m_oRenderer.GetResolution( nResWidth, nResHeight );
#ifdef DISPLAYCURSOR
		if ( x > nResWidth - m_pCursor->GetDimension().GetWidth() )
			m_oInputManager.SetMouseCursorXPos((int) ( nResWidth - m_pCursor->GetDimension().GetWidth() ));

		if ( y > nResHeight - m_pCursor->GetDimension().GetHeight())		
			m_oInputManager.SetMouseCursorYPos((int) ( nResHeight - m_pCursor->GetDimension().GetHeight()));
#endif //DISPLAYCURSOR

		if ( x < 0.f )
			m_oInputManager.SetMouseCursorXPos( 0 );
		if ( y < 0.f )
			m_oInputManager.SetMouseCursorYPos( 0 );
		// fin test

		for(set<CGUIWindow*>::iterator itWindow = m_DisplayedWindowsSet.begin(); itWindow != m_DisplayedWindowsSet.end(); itWindow++)
		{
			CGUIWindow* pWindow = *itWindow;
			pWindow->Display();
			IInputManager::TMouseButtonState eButtonState = m_oInputManager.GetMouseButtonState(IInputManager::eMouseButtonLeft);
			pWindow->UpdateCallback(nCursorXPos, nCursorYPos, eButtonState);			
		}

		RenderText();

#ifdef DISPLAYCURSOR
		m_pCursor->SetPosition( static_cast<float>( nCursorXPos ), static_cast<float>( nCursorYPos ) );
		m_pCursor->GetLogicalPosition( fWidgetLogicalPosx, fWidgetLogicalPosy, nResWidth, nResHeight );
		vPos.clear();
		vPos.push_back( fWidgetLogicalPosx );
		vPos.push_back( fWidgetLogicalPosy );
		m_pShader->SendUniformVec2Array( "vImagePosition", vPos );
		m_pCursor->Display( m_oRessourceManager );
#endif // DISPLAYCURSOR
		m_oRenderer.EnableTextureBlend(false);
	}
	m_oRenderer.EnableZTest( true );
}


#ifdef DISPLAYCURSOR
//-----------------------------------------------------------------------------------------------------
//										GetCursorDimension
//-----------------------------------------------------------------------------------------------------
CDimension CGUIManager::GetCursorDimension()const
{
	return m_pCursor->GetDimension();
}
#endif // DISPLAYCURSOR
//-----------------------------------------------------------------------------------------------------
//										CreateListener
//-----------------------------------------------------------------------------------------------------
int CGUIManager::CreateListener( IGUIManager::EVENT_CALLBACK pfnCallBack )
{
	CListener* pListener = new CListener;
	pListener->SetEventCallBack( pfnCallBack );
	int hListener = (int)m_mListener.size();
	m_mListener[ hListener ] = pListener;
	return hListener;
}

//-----------------------------------------------------------------------------------------------------
//										AddEventListener
//-----------------------------------------------------------------------------------------------------
void CGUIManager::AddEventListener( int hWidget, int hListener )
{
	CListener* pListener = m_mListener[ hListener ];
	CGUIWidget* pWidget = m_mWidget[ hWidget ];
	pWidget->SetListener( pListener );
}

//-----------------------------------------------------------------------------------------------------
//										GetDimension
//-----------------------------------------------------------------------------------------------------
CDimension CGUIManager::GetDimension(CGUIWidget* pWidget)
{
	return pWidget->GetDimension();
}

void CGUIManager::AddWindow(IGUIWindow* pWindow)
{
	CGUIWindow* pGUIWindow = dynamic_cast<CGUIWindow*>(pWindow);
	m_DisplayedWindowsSet.insert(pGUIWindow);
	pGUIWindow->OnShow(true);
	if (!m_bGUIMode && pGUIWindow->IsGUIMode())
		SetGUIMode(true);
}

bool CGUIManager::IsWindowDisplayed(IGUIWindow* pWindow)
{
	set<CGUIWindow*>::iterator itWindow = m_DisplayedWindowsSet.find(dynamic_cast<CGUIWindow*>(pWindow));
	return (itWindow != m_DisplayedWindowsSet.end());
}

void CGUIManager::RemoveWindow(IGUIWindow* pWindow)
{
	CGUIWindow* pGUIWindow = dynamic_cast<CGUIWindow*>(pWindow);
	m_DisplayedWindowsSet.erase(pGUIWindow);
	pGUIWindow->OnShow(false);
	bool bGUIMode = false;
	for (set<CGUIWindow*>::iterator itWindow = m_DisplayedWindowsSet.begin(); itWindow != m_DisplayedWindowsSet.end(); itWindow++) {
		if ((*itWindow)->IsGUIMode() == true) {
			bGUIMode = true;
			break;
		}
	}
	if(!bGUIMode)
		SetGUIMode(bGUIMode);
}

//-----------------------------------------------------------------------------------------------------
//										IsVisible
//-----------------------------------------------------------------------------------------------------
bool CGUIManager::IsVisible(CGUIWindow* pWindow)
{
	return pWindow->IsVisible(); 
}

//-----------------------------------------------------------------------------------------------------
//										Print
//-----------------------------------------------------------------------------------------------------

void CGUIManager::Print( string sText, int x, int y, TFontColor color )
{ 
	int nNewSize = m_vText.size() + 1 ;
	m_vText.resize( nNewSize );
	for( unsigned int i = 0; i < sText.size(); i++ )
	{
		CGUIWidget* pWidget = NULL;
		switch (color)
		{
		case eWhite:
			pWidget = m_mWidgetFontWhite[sText[i]];
			break;
		case eBlue:
			pWidget = m_mWidgetFontBlue[sText[i]];
			break;
		case eTurquoise:
			pWidget = m_mWidgetFontTurquoise[sText[i]];
			break;
		default:
			break;
		}
		m_vText[ nNewSize - 1 ].m_vWidget.push_back( *pWidget );
	}
	m_vText[ nNewSize - 1 ].m_nPosX = x;
	m_vText[ nNewSize - 1 ].m_nPosY = y;
}

void CGUIManager::Print( char c, int x, int y )
{
	string s;
	s.push_back( c );
	Print( s, x, y, eWhite );
}

int CGUIManager::CreateStaticText( vector< string >& vText, int nPosX, int nPosY )
{
	ILoader::CAnimatableMeshData ami;
	ILoader::CMeshInfos mi;
	mi.m_bCanBeIndexed = false;
	mi.m_bMultiMaterial = false;
	ILoader::CMeshInfos& miA = m_mWidgetFontInfos[ 'a' ];
	mi.m_oMaterialInfos = miA.m_oMaterialInfos;
	mi.m_sShaderName = "gui";
	int nNumChar = 0;
	float fOffsetY = 0.f;
	float fLogicalFontHeight;
	CGUIWidget oTestWidget = *m_mWidgetFontWhite[ 'a' ];
	unsigned int nScreenWidth, nScreenHeight;
	m_oRenderer.GetResolution( nScreenWidth, nScreenHeight );
	float fFontDimX, fFontDimY;
	oTestWidget.GetLogicalDimension( fFontDimX, fFontDimY, nScreenWidth, nScreenHeight );
	oTestWidget.SetPosition( nPosX, nPosY );
	float fPosX, fPosY;
	oTestWidget.GetLogicalPosition( fPosX, fPosY, nScreenWidth, nScreenHeight );
	CGUIWidget oSpaceWidget = *m_mWidgetFontWhite[ ' ' ];
	float fSpaceWidgetWidth, fSpaceWidgetHeight;
	oSpaceWidget.GetLogicalDimension( fSpaceWidgetWidth, fSpaceWidgetHeight, nScreenWidth, nScreenHeight );
	float fTabWidth = fSpaceWidgetWidth * 4;

	for( int iLine = 0; iLine < vText.size(); iLine++ )
	{
		float fOffsetX = 0;
		for( int iChar = 0; iChar < vText[ iLine ].size(); iChar++, nNumChar++ )
		{
			char c = vText[ iLine ][ iChar ];
			ILoader::CMeshInfos& miTemp = m_mWidgetFontInfos[ c ];

			if( iChar > 0 )
			{
				CGUIWidget oTempWidget = *m_mWidgetFontWhite[ vText[ iLine ][ iChar - 1 ] ];
				oTempWidget.SetPosition( oTempWidget.GetDimension().GetWidth() + m_nCharspace, 0 );
				float x, y;
				oTempWidget.GetLogicalPosition( x, y, nScreenWidth, nScreenHeight );
				fOffsetX += x;
			}
			for( int iVertex = 0; iVertex < miTemp.m_vVertex.size(); iVertex++ )
			{
				float ox = 0.f, oy = 0.f;
				if( ( iVertex % 3 ) == 0 )
					ox = fOffsetX + fPosX;
				if( ( ( iVertex - 1 ) % 3  ) == 0 )
					oy = fOffsetY + fPosY;
				mi.m_vVertex.push_back( miTemp.m_vVertex[ iVertex ] + ox + oy );
			}

			for( int iIndex = 0; iIndex < miTemp.m_vIndex.size(); iIndex++ )
				mi.m_vIndex.push_back( miTemp.m_vIndex[ iIndex ] + nNumChar * 4 );
			for( int iUVIndex = 0; iUVIndex < miTemp.m_vUVIndex.size(); iUVIndex++ )
				mi.m_vUVIndex.push_back( miTemp.m_vUVIndex[ iUVIndex ] + nNumChar * 4 );
			mi.m_vUVVertex.insert( mi.m_vUVVertex.begin() + mi.m_vUVVertex.size(), miTemp.m_vUVVertex.begin(), miTemp.m_vUVVertex.end() );
			mi.m_vNormalFace.insert( mi.m_vNormalFace.begin() + mi.m_vNormalFace.size(), miTemp.m_vNormalFace.begin(), miTemp.m_vNormalFace.end() );
			mi.m_vNormalVertex.insert( mi.m_vNormalVertex.begin() + mi.m_vNormalVertex.size(), miTemp.m_vNormalVertex.begin(), miTemp.m_vNormalVertex.end() );			
		}
		fOffsetY -= fFontDimY;
	}
	if( mi.m_vVertex.size() > 0 )
	{
		ami.m_vMeshes.push_back( mi );
		IAnimatableMesh* pARect = m_oRessourceManager.CreateMesh( ami, m_pFontMaterial );
		int nID = m_mStaticText.size();
		m_mStaticText[ nID ] = pARect;
		return nID;
	}
	return -1;
}

void CGUIManager::DestroyStaticTest( int nID )
{
	if( nID != -1 )
	{
		IAnimatableMesh* pAmi = m_mStaticText[ nID ];
		delete pAmi;
		m_mStaticText.erase( nID );
		m_mStaticTextToRender.erase( nID );
	}
}

void CGUIManager::PrintStaticText( int nTextID )
{
	if( nTextID != -1 )
		m_mStaticTextToRender[ nTextID ] = true;
}

void CGUIManager::EnableStaticText( int nTextID, bool bEnable )
{
	map< int, bool >::iterator itText = m_mStaticTextToRender.find( nTextID );
	if( itText != m_mStaticTextToRender.end() )
		itText->second = bEnable;
}

ITopicWindow* CGUIManager::GetTopicsWindow()
{
	return m_pTopicsWindow;
}

void CGUIManager::SetGUIMode(bool bGUIMode)
{
	m_oInputManager.SetEditionMode(bGUIMode);
	m_bGUIMode = bGUIMode;
	
	bool condition = false;
	do{
		INT ret = ShowCursor(m_bGUIMode);
		condition = bGUIMode ? ret > 0 : ret < 0;
	} while (!condition);

}

bool CGUIManager::GetGUIMode()
{
	return m_bGUIMode;
}


//-----------------------------------------------------------------------------------------------------
//										EndRenderText
//-----------------------------------------------------------------------------------------------------
void CGUIManager::RenderText()
{
	
	for( unsigned int i = 0; i < m_vText.size(); i++ )
	{
		CLine& oLine = m_vText[ i ];
		int nCursorPosX = oLine.m_nPosX, nCursorPosY = oLine.m_nPosY;
		for( unsigned int j = 0; j < oLine.m_vWidget.size(); j++ )
		{
			CGUIWidget& oWidget = oLine.m_vWidget[ j ];
			if(  oWidget == *m_mWidgetFontWhite[ '\n' ] )
			{
				nCursorPosX = oLine.m_nPosX;
				nCursorPosY += GetCurrentFontEspacementY();
				continue;
			}
			if( j > 0 )
				nCursorPosX += oLine.m_vWidget[ j - 1 ].GetDimension().GetWidth() + m_nCharspace;
			oWidget.SetPosition( nCursorPosX, nCursorPosY );
			oWidget.Display();
		}
	}

	for( map< int, bool >::iterator itTextID = m_mStaticTextToRender.begin(); itTextID != m_mStaticTextToRender.end(); itTextID++ )
	{
		map< int, IAnimatableMesh* >::iterator itText = m_mStaticText.find( itTextID->first );
		if( itText != m_mStaticText.end() && itTextID->second )
		{
			vector< float > vPos;
			vPos.push_back( 0.f );
			vPos.push_back( 0.f );
			m_pShader->SendUniformVec2Array( "vImagePosition", vPos );
			itText->second->GetMesh( 0 )->Update();
		}
	}
	m_vText.clear();
}

int CGUIManager::GetCurrentFontEspacementY()
{
	return (int)m_mWidgetFontWhite['A']->GetDimension().GetHeight() + 2;
}

string CGUIManager::GetName()
{
	return "GUIManager";
}

void CGUIManager::ToggleDisplayMap()
{
	m_bDisplayMap = !m_bDisplayMap;
	if (m_bDisplayMap)
		AddWindow(m_pMapWindow);
	else
		RemoveWindow(m_pMapWindow);
	m_pScene->DisplayMinimap(m_bDisplayMap);
}

unsigned int CGUIManager::GetCurrentFontHeight() const
{
	map< unsigned char, CGUIWidget* >::const_iterator it =  m_mWidgetFontWhite.find( 'A' );
	return ( unsigned int )it->second->GetDimension().GetHeight();
}

unsigned int CGUIManager::GetCurrentFontWidth( char c ) const
{
	map< unsigned char, CGUIWidget* >::const_iterator it = m_mWidgetFontWhite.find( c );
	return ( unsigned int )it->second->GetDimension().GetWidth();
}

CGUIWidget* CGUIManager::GetWidgetByHandle( int hWidget ) const
{
	map< int, CGUIWidget* >::const_iterator itWidget = m_mWidget.find( hWidget );
	if ( itWidget == m_mWidget.end() )
	{
		exception e( "Identifiant de widget incorrect" );
		throw e;
	}
	CGUIWidget* pWidget = itWidget->second;
	return pWidget;
}

int CGUIManager::InsertWidgetInMap( CGUIWidget* pWidget )
{
	int hWidget = (int)m_mWidget.size();
	m_mWidget[ hWidget ] = pWidget;
	return hWidget;
}

void CGUIManager::SetActive( bool bActivate )
{
	m_bActive = bActivate;
}

bool CGUIManager::GetActive()
{
	return m_bActive;
}

IGUIWindow* CGUIManager::CreatePlayerWindow(int nWidth, int nHeight)
{
	IGUIWindow* playerWindow = new CPlayerWindow(m_oInterface, CDimension(nWidth, nHeight));
	return playerWindow;
}

extern "C" _declspec(dllexport) IGUIManager* CreateGUIManager(EEInterface& oInterface)
{
	return new CGUIManager(oInterface);
}