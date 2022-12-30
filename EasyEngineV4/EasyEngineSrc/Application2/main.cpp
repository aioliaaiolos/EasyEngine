#include <windows.h>

// stl
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

// Engine 
#include "../Utils2/DebugTool.h"
#include "IInputManager.h"
#include "IGUIManager.h"
#include "IRenderer.h"
#include "IFileSystem.h"
#include "IWindow.h"
#include "IWidget.h"
#include "ITestMesh.h"
#include "IRessource.h"
#include "ILoader.h"
#include "IXMLParser.h"
#include "IDrawTool.h"
#include "IActionManager.h"
#include "IConsole.h"
#include "IHud.h"
#include "IScriptManager.h"
#include "IEntity.h"
#include "../Utils2/Chunk.h"
#include "ICamera.h"
#include "Exception.h"
#include "ScriptRegistration.h"
#include "ICollisionManager.h"
#include "IGeometry.h"
#include "IPathFinder.h"
#include "IEditor.h"
#include "Interface.h"

//#define CATCH_EXCEPTION

using namespace std;

struct CGFXOption
{
	bool m_bFullscreen;
	CGFXOption() : m_bFullscreen( true ){}
};

void OnWindowEvent( CPlugin* pPlugin, IEventDispatcher::TWindowEvent e, int nWidth, int nHeight );
void OnMouseEvent(CPlugin*, IEventDispatcher::TMouseEvent, int, int);
void UpdateCamera();
void GetOptionsByCommandLine( string sCommandArguments, CGFXOption& oOption );
void DestroyPlugins();

IDrawTool*				m_pDrawTool = NULL;
IWindow*				m_pWindow = NULL;
ILoaderManager*			m_pLoaderManager = NULL;
IRessourceManager*		m_pRessourceManager = NULL;
IRenderer*				m_pRenderer = NULL;
IRenderer*				m_pSoftRenderer = NULL;
ICameraManager* 		m_pCameraManager = NULL;
IEntityManager* 		m_pEntityManager = NULL;
IInputManager*			m_pInputManager = NULL;
IScriptManager* 		m_pScriptManager = NULL;
IGUIManager*			m_pGUIManager = NULL;
IHud*					m_pHud = NULL;
IFileSystem*			m_pFileSystem = NULL;
IConsole*				m_pConsole = NULL;
ISceneManager*			m_pSceneManager  = NULL;
IActionManager*			m_pActionManager = NULL;
ICollisionManager*		m_pCollisionManager= NULL;
IEventDispatcher*		m_pEventDispatcher = NULL;
IXMLParser*				m_pXMLParser = NULL;
IGeometryManager*		m_pGeometryManager = NULL;
IPathFinder*			m_pPathFinder = NULL;
IEditorManager*			m_pEditorManager = nullptr;

vector< IEntity* > m_vLight;
bool	m_bFirstTimeOpenFile = true;
CDebugTool* m_pDebugTool = NULL;
CNode* m_pEntity = NULL;
IScene* m_pScene = NULL;

float m_nDeltaTickCount = 0;
float m_nLastTickCount = 0;
int m_nLastGameMousePosx, m_nLastGameMousePosy;
CMatrix ident;
bool m_bRenderScene = true;
bool bCapture = false;


void InitScene( ISceneManager* pSceneManager )
{	
	try
	{
		InitScriptRegistration();
		m_pScene->DeleteTempDirectories();
		m_pConsole->Open(true);
		FILE* pFile = NULL;
		pFile = m_pFileSystem->OpenFile("start.eas", "r");
		if( pFile )
		{
			fseek( pFile, 0, SEEK_END );
			long pos = ftell( pFile );
			fclose( pFile );
			if (pos > 0) {
				m_pScriptManager->ExecuteCommand("run(\"start\");");
				m_pScriptManager->ExecuteCommand("onLoadingGame();");
			}
		}
		else
			m_pConsole->Println( "Fichier start introuvable." );
	}
	catch( CEException& e )
	{
		m_pConsole->Println(e.what());
	}
	catch (exception& e) {
		m_pConsole->Println(e.what());
	}
}

void UpdateCamera()
{
	IInputManager::KEY_STATE eStateZ = m_pActionManager->GetKeyActionState( "Avancer");
	IInputManager::KEY_STATE eStateS = m_pActionManager->GetKeyActionState( "Reculer");
	IInputManager::KEY_STATE eStateD = m_pActionManager->GetKeyActionState( "StrafeRight");	
	IInputManager::KEY_STATE eStateQ = m_pActionManager->GetKeyActionState( "StrafeLeft");
	IInputManager::KEY_STATE eMoreSpeed = m_pActionManager->GetKeyActionState( "MoreSpeed" );
	IInputManager::KEY_STATE eLessSpeed = m_pActionManager->GetKeyActionState( "LessSpeed" );

	m_pActionManager->ForceActionState( "MoreSpeed", IInputManager::RELEASED );
	m_pActionManager->ForceActionState( "LessSpeed", IInputManager::RELEASED );

	int avance = ( eStateZ == IInputManager::PRESSED || eStateZ == IInputManager::JUST_PRESSED ) - ( eStateS == IInputManager::PRESSED || eStateS == IInputManager::JUST_PRESSED);
	int gauche = ( eStateQ == IInputManager::JUST_PRESSED || eStateQ == IInputManager::PRESSED ) - ( eStateD == IInputManager::JUST_PRESSED || eStateD == IInputManager::PRESSED );
	avance *= 10;
	gauche *= 10;

	ICamera* pActiveCamera = m_pCameraManager->GetActiveCamera();
	if (pActiveCamera) {
		if (eLessSpeed == IInputManager::JUST_PRESSED)
			pActiveCamera->SetSpeed(pActiveCamera->GetSpeed() / 2.f);
		if (eMoreSpeed == IInputManager::JUST_PRESSED)
			pActiveCamera->SetSpeed(pActiveCamera->GetSpeed() * 2.f);
	}
	m_pActionManager->GetGameMousePos( m_nLastGameMousePosx, m_nLastGameMousePosy );
	
	if (pActiveCamera) {
		if (m_pCameraManager->GetCameraType(pActiveCamera) == ICameraManager::TFree)
		{
			int x, y;
			m_pInputManager->GetOffsetMouse(x, y);
			float s = m_pInputManager->GetMouseSensitivity(); // 1/20
			pActiveCamera->Move(-x * s, -y * s, 0.f, (float)avance, (float)gauche, 0.f);
		}
	}
}

void OnKeyAction( CPlugin* pPlugin, unsigned int key, IInputManager::KEY_STATE state )
{
	switch( state )
	{
	case IInputManager::JUST_PRESSED:
		if (key == m_pConsole->GetConsoleShortCut())
			m_pConsole->Open(!m_pConsole->IsOpen());
		else if (!m_pConsole->IsOpen()) {
			if (key == VK_TAB) {
				IPlayer* player = m_pEntityManager->GetPlayer();
				if (player)
					player->ToggleDisplayPlayerWindow();
			}
			else if (key == 'M')
				m_pGUIManager->ToggleDisplayMap();
			else if (key == 'E')
			{
				IPlayer* pPlayer = m_pEntityManager->GetPlayer();
				if(pPlayer)
					pPlayer->Action();
			}
		}
	}
}

void UpdatePerso()
{
	IPlayer* pPerso = m_pEntityManager->GetPlayer();
	if(pPerso && !m_pConsole->IsOpen() && !m_pGUIManager->GetGUIMode())
	{		
		ICamera* pCamera = m_pCameraManager->GetActiveCamera();
		if( pPerso && (m_pCameraManager->GetCameraType(pCamera) == ICameraManager::TLinked) ) {
			IInputManager::KEY_STATE eStateWalk = m_pActionManager->GetKeyActionState( "AvancerPerso");
			if( eStateWalk == IInputManager::JUST_PRESSED || eStateWalk == IInputManager::PRESSED )
			{
				if(m_pInputManager->GetKeyState(VK_SHIFT) == IInputManager::JUST_PRESSED || m_pInputManager->GetKeyState(VK_SHIFT) == IInputManager::PRESSED)
					pPerso->RunAction("walk", true);
				else
					pPerso->RunAction( "run", true );
					
				m_pActionManager->ForceActionState( "AvancerPerso", IInputManager::PRESSED );
			}
			else if( eStateWalk == IInputManager::JUST_RELEASED )
			{
				pPerso->RunAction( "stand", true );
				m_pActionManager->ForceActionState( "AvancerPerso", IInputManager::RELEASED );
			}
			IInputManager::KEY_STATE eStateJump = m_pActionManager->GetKeyActionState("SautPerso");
			if (eStateJump == IInputManager::JUST_PRESSED)
			{
				pPerso->RunAction("jump", true);
				m_pActionManager->ForceActionState("SautPerso", IInputManager::PRESSED);
			}

			IInputManager::TMouseButtonState eStatePiedG = m_pActionManager->GetMouseActionState( "HitLeftFoot");
			if( eStatePiedG == IInputManager::eMouseButtonStateJustDown )
			{
				IFighterEntityInterface* pFighter = dynamic_cast<IFighterEntityInterface*>(pPerso);
				if (pFighter)
					pFighter->SecondaryHit();
			}

			IInputManager::TMouseButtonState eStateRightFist = m_pActionManager->GetMouseActionState("HitRightArm");
			if (eStateRightFist == IInputManager::eMouseButtonStateJustDown)
			{
				IFighterEntityInterface* pFighter = dynamic_cast<IFighterEntityInterface*>(pPerso);
				if (pFighter)
					pFighter->MainHit();
			}
			
			ICamera* pLinkedCamera = m_pCameraManager->GetCameraFromType(ICameraManager::TLinked);
			if (pLinkedCamera) {
				IInputManager::TMouseButtonState eStateZoom = m_pActionManager->GetMouseActionState("Zoom");
				if (eStateZoom == IInputManager::eMouseWheelUp)
					pLinkedCamera->Zoom(1);
				IInputManager::TMouseButtonState eStateUnzoom = m_pActionManager->GetMouseActionState("Unzoom");
				if(eStateUnzoom == IInputManager::eMouseWheelDown)
					pLinkedCamera->Zoom(-1);
			}
			
			if( m_pCameraManager->GetCameraType( m_pCameraManager->GetActiveCamera() ) == ICameraManager::TLinked)
			{
				int x, y;
				m_pInputManager->GetOffsetMouse( x, y );
				float s = m_pInputManager->GetMouseSensitivity();
				pPerso->Yaw( - x * s );
				m_pCameraManager->GetActiveCamera()->Pitch((float)-y/30.f);
			}
		}
	}
}

void OnUpdateWindow()
{
	int nTickCount = GetTickCount();
	m_nDeltaTickCount = nTickCount - m_nLastTickCount;
	m_nLastTickCount = (float)nTickCount;
	m_pInputManager->OnUpdate();
	if( !m_pConsole->IsOpen() && !m_pGUIManager->GetGUIMode())
		UpdateCamera();
	UpdatePerso();
	m_pRenderer->BeginRender();
	try {
		if (m_bRenderScene)
			m_pSceneManager->GetScene("Game")->Update();
		m_pGUIManager->OnRender();
		m_pConsole->Update();
		if (bCapture)
		{
			vector< unsigned char > vPixels;
			unsigned int w, h;
			m_pRenderer->GetResolution(w, h);
			m_pRenderer->ReadPixels(0, 0, w, h, vPixels, IRenderer::T_BGR);
			ILoader::CTextureInfos ti;
			ti.m_ePixelFormat = ILoader::eBGR;
			ti.m_nWidth = 1280;
			ti.m_nHeight = 1024;
			ti.m_vTexels.swap(vPixels);
			m_pLoaderManager->Export("test_HM.bmp", ti);
			bCapture = false;
		}
	}
	catch (CRenderException& e) {
		MessageBoxA(NULL, e.what(), "RenderException", MB_ICONERROR);
	}
	catch (CEException& e) {
		m_pConsole->Println(e.what());
	}
  	m_pRenderer->EndRender();
}

void OnRender( IRenderer* pRenderer )
{

}

EEInterface* InitPlugins( string sCmdLine )
{
	EEInterface* pInterface = new EEInterface;
	CPlugin::SetEngineInterface(pInterface);
	string sDirectoryName;
#ifdef _DEBUG
		sDirectoryName = "..\\..\\EasyEngine\\Debug\\";
#else
		sDirectoryName = "..\\..\\EasyEngine\\release\\";
#endif

	CGFXOption oOption;
	GetOptionsByCommandLine( sCmdLine, oOption );

	m_pPathFinder = static_cast<IPathFinder*>(CPlugin::Create(*pInterface, sDirectoryName + "IA.dll", "CreatePathFinder"));
		
	m_pFileSystem = static_cast< IFileSystem* > ( CPlugin::Create( *pInterface, sDirectoryName + "FileUtils.dll", "CreateFileSystem" ) );
	m_pFileSystem->Mount( "..\\data" );
	m_pFileSystem->Mount( "..\\..\\EasyEngine\\data" );

	m_pEventDispatcher = static_cast< IEventDispatcher* >( CPlugin::Create(*pInterface, sDirectoryName + "WindowsGUI.dll", "CreateEventDispatcher" ) );

	RECT rect;
	GetWindowRect(GetDesktopWindow(), &rect);
	int nResX = rect.right, nResY = rect.bottom;

	IInputManager::Desc oInputManagerDesc( *m_pEventDispatcher );
	oInputManagerDesc.m_nResX = nResX;
	oInputManagerDesc.m_nResY = nResY;
	m_pInputManager = static_cast< IInputManager* >( CPlugin::Create( oInputManagerDesc, sDirectoryName + "IO.dll", "CreateInputManager" ) );

	IActionManager::Desc oActionManagerDesc( *m_pInputManager );
	m_pActionManager = static_cast< IActionManager* >( CPlugin::Create( oActionManagerDesc, sDirectoryName + "IO.dll", "CreateActionManager" ) );

	IWindow::Desc oWindowDesc( nResX, nResY, "AC", *m_pEventDispatcher );
	oWindowDesc.m_bFullscreen = oOption.m_bFullscreen;
	oWindowDesc.m_nBits = 32;
	oWindowDesc.m_sName = "Window";
	m_pWindow = static_cast< IWindow* >( CPlugin::Create( oWindowDesc, sDirectoryName + "WindowsGUI.dll", "CreateWindow2" ) );

	m_pRenderer = static_cast< IRenderer* >( CPlugin::Create( *pInterface, sDirectoryName + "Renderer.dll", "CreateRenderer" ) );	

	IGeometryManager::Desc oGeometryManagerDesc( NULL, "GeometryManager" );
	m_pGeometryManager = static_cast< IGeometryManager* >( CPlugin::Create( oGeometryManagerDesc, "Geometry.dll", "CreateGeometryManager" ) );
	m_pLoaderManager =  static_cast< ILoaderManager* >( CPlugin::Create(*pInterface, sDirectoryName + "Loader.dll", "CreateLoaderManager" ) );
	m_pRessourceManager = static_cast< IRessourceManager* >( CPlugin::Create(*pInterface, sDirectoryName + "Ressource.dll", "CreateRessourceManager" ) );
	m_pCollisionManager = static_cast< ICollisionManager* >( CPlugin::Create(*pInterface, "Collision.dll", "CreateCollisionManager" ));
	m_pCameraManager = static_cast< ICameraManager* >( CPlugin::Create( *pInterface, sDirectoryName + "Entity.dll", "CreateCameraManager" ) );
	m_pEntityManager = static_cast< IEntityManager* >(CPlugin::Create(*pInterface, sDirectoryName + "Entity.dll", "CreateEntityManager"));
	m_pCollisionManager->SetEntityManager(m_pEntityManager);

	m_pSceneManager = static_cast< ISceneManager* >( CPlugin::Create(*pInterface, sDirectoryName + "Entity.dll", "CreateSceneManager" ) );

	//IXMLParser::Desc oXMLParserDesc( *m_pFileSystem );
	m_pXMLParser = static_cast< IXMLParser* >( CPlugin::Create( *pInterface, sDirectoryName + "FileUtils.dll", "CreateXMLParser" ) );

	m_pScene = m_pSceneManager->CreateScene("Game", "", "");
	m_pGUIManager = static_cast< IGUIManager* >( CPlugin::Create(*pInterface, sDirectoryName + "GUI.dll", "CreateGUIManager" ) );
	m_pEntityManager->SetGUIManager(m_pGUIManager);
	m_pHud = static_cast< IHud* >(CPlugin::Create(*pInterface, sDirectoryName + "IO.dll", "CreateHud"));
	m_pScriptManager = static_cast< IScriptManager* >(CPlugin::Create(*pInterface, sDirectoryName + "Script.dll", "CreateScriptManager" ));
	RegisterAllFunctions( m_pScriptManager );

	m_pConsole = static_cast< IConsole* >( CPlugin::Create(*pInterface, sDirectoryName + "IO.dll", "CreateConsole" ) );
	m_pConsole->SetConsoleShortCut(192);

	m_pEditorManager = static_cast< IEditorManager* >(CPlugin::Create(*pInterface, sDirectoryName + "Editor.dll", "CreateEditorManager"));

	return pInterface;
}

void InitKeyActions()
{
	const char pAzerty[] = { 'Z', 'S', 'Q', 'D' };
	const char pQwerty[] = { 'W', 'S', 'A', 'D' };
	const char* pCurrent = pQwerty;

	m_pActionManager->AddKeyAction("Avancer", pCurrent[0]);
	m_pActionManager->AddKeyAction("Reculer", pCurrent[1]);
	m_pActionManager->AddKeyAction("StrafeLeft", pCurrent[2]);
	m_pActionManager->AddKeyAction("StrafeRight", pCurrent[3]);
	m_pActionManager->AddKeyAction("MoreSpeed", 'V');
	m_pActionManager->AddKeyAction("LessSpeed", 'C');
	m_pActionManager->AddKeyAction("Action", 'E');
	//m_pActionManager->AddKeyAction("Map", 'M');
	m_pActionManager->AddKeyAction("CameraYaw", AXIS_H);
	m_pActionManager->AddKeyAction("CameraPitch", AXIS_V);
	m_pActionManager->AddKeyAction("Console", 222);
	m_pActionManager->AddGUIAction("CursorX", AXIS_H);
	m_pActionManager->AddGUIAction("CursorY", AXIS_V);

	m_pActionManager->AddKeyAction("AvancerPerso", 'T');
	m_pActionManager->AddKeyAction("SautPerso", ' ');
	m_pActionManager->AddMouseAction("HitLeftFoot", IInputManager::eMouseButtonRight, IInputManager::eMouseButtonStateJustDown);
	m_pActionManager->AddMouseAction("HitRightArm", IInputManager::eMouseButtonLeft, IInputManager::eMouseButtonStateJustDown);
	m_pActionManager->AddMouseAction("Zoom", IInputManager::eMouseWheel, IInputManager::eMouseWheelUp);
	m_pActionManager->AddMouseAction("Unzoom", IInputManager::eMouseWheel, IInputManager::eMouseWheelDown);
}


int WINAPI WinMain( HINSTANCE hIstance, HINSTANCE hPrevInstance, LPSTR plCmdLine, int nCmdShow )
{
	m_pDebugTool = new CDebugTool;
#ifdef CATCH_EXCEPTION
	try
	{
#endif // CATCH_EXCEPTION
		EEInterface* pInterface = InitPlugins( plCmdLine );
		InitKeyActions();
		m_pEventDispatcher->AbonneToWindowEvent( NULL, OnWindowEvent );
		m_pInputManager->AbonneToKeyEvent( NULL, OnKeyAction );
		InitScene( m_pSceneManager );
		m_pWindow->ShowModal();
		m_pRenderer->DestroyContext();
		DestroyPlugins();

#ifdef CATCH_EXCEPTION
	}
	catch( CFileNotFoundException& e )
	{
		string sMessage = string( "Fichier \"" ) + e.what() + "\" introuvable";
		MessageBoxA( NULL, sMessage.c_str(), "Erreur fichier", MB_ICONERROR );
	}
 	catch( exception& e )
	{
		MessageBoxA( NULL, e.what(), "", MB_ICONERROR );
	}
#endif // CATCH_EXCEPTION
	

	return 0;
}

void DestroyPlugins()
{
	delete m_pDebugTool;
	delete m_pConsole;
	delete m_pScriptManager;
	delete m_pGUIManager;
	delete m_pXMLParser;
	delete m_pEntityManager;
	delete m_pSceneManager;
	delete m_pRessourceManager;
	delete m_pLoaderManager;
	delete m_pCameraManager;
	delete m_pSoftRenderer;
	delete m_pRenderer;
	delete m_pWindow;
	delete m_pActionManager;
	delete m_pInputManager;
	delete m_pEventDispatcher;
	delete m_pFileSystem;
	delete m_pEditorManager;
	delete m_pCollisionManager;
	delete m_pHud;
	//delete m_pScene;
}


void  OnWindowEvent( CPlugin* pPlugin, IEventDispatcher::TWindowEvent e, int nWidth, int nHeight )
{
	int test = 0;
	switch( e )
	{
	case IEventDispatcher::T_WINDOWUPDATE:
		OnUpdateWindow();
		break;
	}
}



void OnInitGUI()
{
}


void GetOptionsByCommandLine( string sCommandArguments, CGFXOption& oOption )
{
	if ( sCommandArguments.size() > 0 )
	{
		int nBegin = static_cast< int >( sCommandArguments.find( "-" ) );
		int nEnd = static_cast< int >( sCommandArguments.find( "=" ) );
		string sArgName = sCommandArguments.substr( nBegin + 1, nEnd - 1 );
		int nEndArgValue = static_cast< int >( sCommandArguments.find( " ", nEnd ) );
		if ( nEndArgValue == -1 )
		{
			nEndArgValue = static_cast< int >( sCommandArguments.find( "\n", nEnd + 1 ) );
			if ( nEndArgValue == -1 )
				nEndArgValue = static_cast< int >( sCommandArguments.size() - nEnd + 1 );
		}
		string sArgValue = sCommandArguments.substr( nEnd + 1, nEndArgValue );
		string sArgNameLow = sArgName;
		string sArgValueLow = sArgValue;
		transform( sArgName.begin(), sArgName.end(), sArgNameLow.begin(), tolower );
		transform( sArgValue.begin(), sArgValue.end(), sArgValueLow.begin(), tolower );
		if ( sArgName == "fullscreen" )
		{
			if ( sArgValue == "true" ) 
				oOption.m_bFullscreen = true;
			else
			{
				if ( sArgValue == "false" )
					oOption.m_bFullscreen = false;
				else
				{
					string sMessage = string( " Mauvais argument de la ligne de commande pour \"fullscreen\", valeur " ) + sArgValue + " non reconnue";
					exception e( sMessage.c_str() );
					throw e;
				}
			}
		}
	}
}