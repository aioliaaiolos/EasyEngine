// System
#include <time.h>

// Engine
#include "ScriptRegistration.h"
#include "IEntity.h"
#include "IInputManager.h"
#include "IConsole.h"
#include "IWindow.h"
#include "ILoader.h"
#include "IEntity.h"
#include "IRenderer.h"
#include "ICamera.h"
#include "IRessource.h"
#include "Exception.h"
#include "IGUIManager.h"
#include "ICollisionManager.h"
#include "IShader.h"
#include "IGeometry.h"
#include "IHud.h"
#include "IEditor.h"
#include "IPhysic.h"
#include "../Utils2/RenderUtils.h"
#include "../Utils2/DebugTool.h"
#include "../Utils2/EasyFile.h"
#include "../Utils2/StringUtils.h"
#include "IEventDispatcher.h"

// stl
#include <sstream>
#include <algorithm>
#include <regex>

extern EEInterface*			pInterface;
extern IScene*				m_pScene;
extern IScriptManager*		m_pScriptManager;
extern IConsole*			m_pConsole;
extern IWindow*				m_pWindow;
extern ILoaderManager*		m_pLoaderManager;
extern IEntityManager*		m_pEntityManager;
extern IRenderer*			m_pRenderer;
extern ICameraManager*		m_pCameraManager;
extern ISceneManager*		m_pSceneManager;
extern IGUIManager*			m_pGUIManager;
extern IHud*				m_pHud;
extern ICollisionManager*	m_pCollisionManager;
extern IRessourceManager*	m_pRessourceManager;
extern IFileSystem*			m_pFileSystem;
extern CDebugTool*			m_pDebugTool;
extern IGeometryManager*	m_pGeometryManager;
extern bool					m_bRenderScene;
extern IEventDispatcher*	m_pEventDispatcher;
extern IEditorManager*		m_pEditorManager;
extern IPathFinder*			m_pPathFinder;
extern IPhysic*				m_pPhysic;

IEntity* m_pRepere = NULL;
vector< string > g_vStringsResumeMode;
map<IEntity*, int> g_mEntityPositionLine;
IMapEditor* m_pMapEditor = nullptr;
ICharacterEditor* m_pCharacterEditor = nullptr;
IWorldEditor* m_pWorldEditor = nullptr;
int g_nSlotPosition = 0;
map<IEntity*, int> g_mSlots;
bool g_bEnableWatchLog = false;

enum TObjectType
{
	eNone = 0,
	eEntity,
	eBone
};

TObjectType	m_eSelectionType = eNone;
INode*	m_pSelectedNode = NULL;

struct CNodeInfos
{
	string	m_sName;
	int		m_nID;
};

void InitScriptRegistration()
{
	m_pMapEditor = dynamic_cast<IMapEditor*>(m_pEditorManager->GetEditor(IEditor::Type::eMap));
	if (!m_pMapEditor) {
		m_pConsole->Println("Erreur, Map Editor n'existe pas");
	}
	m_pCharacterEditor = dynamic_cast<ICharacterEditor*>(m_pEditorManager->GetEditor(IEditor::Type::eCharacter));
	if (!m_pCharacterEditor) {
		m_pConsole->Println("Erreur, Character Editor n'existe pas");
	}
	m_pWorldEditor = dynamic_cast<IWorldEditor*>(m_pEditorManager->GetEditor(IEditor::Type::eWorld));
	if (!m_pWorldEditor) {
		m_pConsole->Println("Erreur, World Editor n'existe pas");
	}
}

IEntity* CreateEntity( string sName )
{
	bool bak = m_pRessourceManager->IsCatchingExceptionEnabled();
	m_pRessourceManager->EnableCatchingException( false );
	IEntity* pEntity = NULL;
	try
	{
		pEntity = m_pEntityManager->CreateEntity(sName, "");
		ostringstream oss;
		oss << "L'entité \"" << sName << "\"a été chargée avec l'identifiant " << m_pEntityManager->GetEntityID( pEntity ) << ".";
		m_pConsole->Println( oss.str() );
	}
	catch( CFileNotFoundException& e )
	{		
		string sMessage = string( "Erreur : fichier \"" ) + e.m_sFileName + "\" manquant.";
		m_pConsole->Println( sMessage );
	}
	catch( CRessourceException& e )
	{
		string s;
		e.GetErrorMessage( s );
		m_pConsole->Println( s );
	}
	catch( CBadFileFormat )
	{
		m_pConsole->Println( "Mauvais format de fichier, essayez de le réexporter" );
	}
	catch( CEException )
	{
		string sMessage = string( "\"" ) + sName + "\" introuvable";
		m_pConsole->Println( sMessage );
	}
	m_pRessourceManager->EnableCatchingException( bak );
	return pEntity;
}

void EnableWatchLog(IScriptState* pState)
{
	CScriptFuncArgInt* pEnable = static_cast<CScriptFuncArgInt*>(pState->GetArg(0));
	g_bEnableWatchLog = pEnable->m_nValue != 0;
}

void DisplayOpenglVersion(IScriptState* pState)
{
	string sVersion;
	m_pRenderer->GetOpenglVersion(sVersion);
	m_pConsole->Println(sVersion);
}

void DisplayGlslVersion(IScriptState* pState)
{
	string sVersion;
	m_pRenderer->GetGlslVersion(sVersion);
	m_pConsole->Println(sVersion);
}

void SpawnEntity(IScriptState* pState)
{
	CScriptFuncArgString* pName = static_cast< CScriptFuncArgString* >(pState->GetArg(0));
	string sName = pName->m_sValue;
	if (sName.find(".bme") == -1)
		sName += ".bme";
	bool bak = m_pRessourceManager->IsCatchingExceptionEnabled();
	m_pRessourceManager->EnableCatchingException(false);
	try
	{
		if(m_pMapEditor->IsEnabled())
			m_pMapEditor->SpawnEntity(sName);
		else if(m_pWorldEditor->IsEnabled())
			m_pWorldEditor->SpawnEntity(sName);
	}
	catch (CFileNotFoundException& e)
	{
		string sMessage = string("Erreur : fichier \"") + e.m_sFileName + "\" manquant.";
		m_pConsole->Println(sMessage);
	}
	catch (CRessourceException& e)
	{
		string s;
		e.GetErrorMessage(s);
		m_pConsole->Println(s);
	}
	catch (CBadFileFormat)
	{
		ostringstream oss;
		oss << "\"" << sName << "\" : Mauvais format de fichier, essayez de le réexporter";
		m_pConsole->Println(oss.str());
	}
	catch (CMethodNotImplementedException& e) {
		ostringstream oss;
		oss << "Erreur : la méthode " << e.what() << " n'est pas implementée.";
		m_pConsole->Println(oss.str());
	}
	catch (CEException& e)
	{
		m_pConsole->Println(e.what());
	}
	m_pRessourceManager->EnableCatchingException(bak);
}

void SpawnCharacter(IScriptState* pState)
{
	CScriptFuncArgString* pID = static_cast< CScriptFuncArgString* >(pState->GetArg(0));
	string id = pID->m_sValue;
	bool bak = m_pRessourceManager->IsCatchingExceptionEnabled();
	m_pRessourceManager->EnableCatchingException(false);
	try
	{
		if (m_pWorldEditor->IsEnabled()) {
			IEntity* pCharacter = m_pWorldEditor->SpawnCharacter(id);
			int nCharacterId = m_pEntityManager->GetEntityID(pCharacter);
			pState->SetReturnValue((float)nCharacterId);
		}
		else
			m_pConsole->Println("Erreur : impossible de spawner un personnage en dehors du world editor");
	}
	catch (CCharacterAlreadyExistsException& e) {
		ostringstream oss;
		oss << "Erreur : Il existe deja une instance de " << e.what() << " dans le monde";
		m_pConsole->Println(oss.str());
	}
	catch (CNodeNotFoundException& e) {
		string sMessage;
		e.GetErrorMessage(sMessage);
		m_pConsole->Println(sMessage);
	}
	catch (CEException& e)
	{
		m_pConsole->Println(e.what());
	}
	m_pRessourceManager->EnableCatchingException(bak);
}

void SpawnArea(IScriptState* pState)
{
	CScriptFuncArgString* pAreaName = static_cast< CScriptFuncArgString* >(pState->GetArg(0));
	m_pWorldEditor->SpawnArea(pAreaName->m_sValue);
}

void EditCharacter(IScriptState* pState)
{
	CScriptFuncArgString* pID = static_cast< CScriptFuncArgString* >(pState->GetArg(0));
	string id = pID->m_sValue;
	try
	{
		//m_pCharacterEditor->SpawnEntity(id);
		m_pCharacterEditor->Edit(id);
	}
	catch (CCharacterAlreadyExistsException& e) {
		m_pConsole->Println(string("Erreur, le personnage ") + e.what() + " existe deja");
	}
	catch (CEException& e)
	{
		m_pConsole->Println(e.what());
	}
}

void ChangeCharacterName(IScriptState* pState)
{
	CScriptFuncArgString* pOld = static_cast< CScriptFuncArgString* >(pState->GetArg(0));
	CScriptFuncArgString* pNew = static_cast< CScriptFuncArgString* >(pState->GetArg(1));
	m_pEntityManager->ChangeCharacterName(pOld->m_sValue, pNew->m_sValue);
}

void NormalizeCharacterDatabase(IScriptState* pState)
{
	m_pEntityManager->NormalizeCharacterDatabase();
}

void EditCloth(IScriptState* pState)
{
	CScriptFuncArgString* pClothName = static_cast< CScriptFuncArgString* >(pState->GetArg(0));
	try {
		m_pCharacterEditor->EditCloth(pClothName->m_sValue);
	}
	catch (CEException& e) {
		m_pConsole->Println(e.what());
	}
}

void OffsetCloth(IScriptState* pState)
{
	CScriptFuncArgFloat* px = static_cast< CScriptFuncArgFloat* >(pState->GetArg(0));
	CScriptFuncArgFloat* py = static_cast< CScriptFuncArgFloat* >(pState->GetArg(1));
	CScriptFuncArgFloat* pz = static_cast< CScriptFuncArgFloat* >(pState->GetArg(2));
	try {
		m_pCharacterEditor->OffsetCloth(px->m_fValue, py->m_fValue, pz->m_fValue);
	}
	catch (CEException& e) {
		m_pConsole->Println(e.what());
	}
}

void SaveCloth(IScriptState* pState)
{
	m_pCharacterEditor->SaveCurrentEditableCloth();
}

void OffsetEyes(IScriptState* pState)
{
	CScriptFuncArgFloat* px = static_cast< CScriptFuncArgFloat* >(pState->GetArg(0));
	CScriptFuncArgFloat* py = static_cast< CScriptFuncArgFloat* >(pState->GetArg(1));
	CScriptFuncArgFloat* pz = static_cast< CScriptFuncArgFloat* >(pState->GetArg(2));
	m_pCharacterEditor->OffsetEyes(px->m_fValue, py->m_fValue, pz->m_fValue);
}

void TurnEyes(IScriptState* pState)
{
	CScriptFuncArgFloat* pYaw	= static_cast< CScriptFuncArgFloat* >(pState->GetArg(0));
	CScriptFuncArgFloat* pPitch = static_cast< CScriptFuncArgFloat* >(pState->GetArg(1));
	CScriptFuncArgFloat* pRoll	= static_cast< CScriptFuncArgFloat* >(pState->GetArg(2));
	m_pCharacterEditor->TurnEyes(pYaw->m_fValue, pPitch->m_fValue, pRoll->m_fValue);
}

void SaveCurrentEditableBody(IScriptState* pState)
{
	m_pCharacterEditor->SaveCurrentEditableBody();
}

void EditWorld(IScriptState* pState)
{
	CScriptFuncArgString* pID = static_cast< CScriptFuncArgString* >(pState->GetArg(0));
	string id = pID->m_sValue;
	try
	{
		m_pWorldEditor->Edit(id);
	}
	catch (CEException& e)
	{
		m_pConsole->Println(e.what());
	}
}

void EditMap(IScriptState* pState)
{
	CScriptFuncArgString* pID = static_cast< CScriptFuncArgString* >(pState->GetArg(0));
	string id = pID->m_sValue;
	try
	{
		m_pMapEditor->Edit(id);
	}
	catch (CEException& e)
	{
		m_pConsole->Println(e.what());
	}
}

void AdaptGroundToAllEntities(IScriptState* pState)
{
	try {
		m_pMapEditor->AdaptGroundToAllEntities();
	}
	catch (CEException& e) {
		m_pConsole->Println(e.what());
	}
}

void SetHairs(IScriptState* pState)
{
	CScriptFuncArgString* pHairs = static_cast< CScriptFuncArgString* >(pState->GetArg(0));
	m_pCharacterEditor->SetHairs(pHairs->m_sValue);
}

void ShowGUICursor(IScriptState* pState)
{
	CScriptFuncArgInt* pShowCursor = (CScriptFuncArgInt*)pState->GetArg(0);
	ShowCursor(pShowCursor->m_nValue == 1 ? TRUE : FALSE);
}

void DisplayFov( IScriptState* pState )
{
	m_pConsole->Println( "Fonction pas encore implémentée" );
}

void SetFov( IScriptState* pState )
{
	CScriptFuncArgFloat* pFov = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 0 ) );
	m_pRenderer->SetFov( pFov->m_fValue );
}

void print(IScriptState* pState)
{
	CScriptFuncArgInt* pInt = static_cast< CScriptFuncArgInt* >(pState->GetArg(0));
	m_pConsole->Println(pInt->m_nValue);
}

void SetEntityName( IScriptState* pState )
{
	CScriptFuncArgInt* pEntityID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgString* pEntityName = static_cast< CScriptFuncArgString* >( pState->GetArg( 1 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pEntityID->m_nValue );
	if( pEntity )
		pEntity->SetEntityName( pEntityName->m_sValue );
	else
	{
		ostringstream oss;
		oss << "Erreur : Entité " << pEntityID->m_nValue << " inconnue";
		m_pConsole->Println( oss.str() );
	}
}

void Choice(IScriptState* pState)
{
	CScriptFuncArgString* pChoice = static_cast<CScriptFuncArgString*>(pState->GetArg(0));
	m_pGUIManager->GetTopicsWindow()->OnChoiceCalled(pChoice->m_sValue);
}

void Goto( IScriptState* pState )
{
	CScriptFuncArgInt* pEntityID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgFloat* px = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 1 ) );
	CScriptFuncArgFloat* py = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 2 ) );
	CScriptFuncArgFloat* pz = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 3 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pEntityID->m_nValue );
	try
	{
		if( pEntity )
			pEntity->Goto( CVector(px->m_fValue, py->m_fValue, pz->m_fValue), 10.f );
		else
		{
			ostringstream oss;
			oss << "Erreur : Entité " << pEntityID->m_nValue << " introuvable";
			m_pConsole->Println( oss.str() );
		}
	}
	catch( CEException& e )
	{
		string sMessage;
		e.GetErrorMessage( sMessage );
		m_pConsole->Println( sMessage );
	}
}

void DisplayAnimationBBox( IScriptState* pState )
{
	CScriptFuncArgInt* pEntityID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgInt* pBool = static_cast< CScriptFuncArgInt* >( pState->GetArg( 1 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pEntityID->m_nValue );
	if( pEntity )
	{
		bool bDraw = pBool->m_nValue == 1 ? true : false;
		pEntity->DrawAnimationBoundingBox( bDraw );
	}
}

void CreateBox( IScriptState* pState )
{
	CScriptFuncArgFloat* px = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 0 ) );
	CScriptFuncArgFloat* py = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 1 ) );
	CScriptFuncArgFloat* pz = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 2 ) );
	IEntity* pBox = m_pEntityManager->CreateBox(CVector( px->m_fValue, py->m_fValue, pz->m_fValue ) );
	pBox->Link( m_pScene );
	ostringstream oss;
	int id = m_pEntityManager->GetEntityID(pBox);
	oss << "La boite a été créée avec l'identifiant " << id << ".";
	m_pConsole->Println( oss.str() );
	pState->SetReturnValue((float)id);
}

void CreateSphere( IScriptState* pState )
{
	CScriptFuncArgFloat* pRadius = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 0 ) );
	//ISphere* pSphere = m_pGeometryManager->CreateSphere( CVector(), pRadius->m_fValue );
	IEntity* pSphereEntity = m_pEntityManager->CreateSphere( pRadius->m_fValue );
	pSphereEntity->Link( m_pScene );
	ostringstream oss;
	int id = m_pEntityManager->GetEntityID(pSphereEntity);
	oss << "La sphere a été créée avec l'identifiant " << id << ".";
	m_pConsole->Println( oss.str() );
	pState->SetReturnValue(id);
}

void CreateQuad(IScriptState* pState)
{
	CScriptFuncArgFloat* pLenght = static_cast< CScriptFuncArgFloat* >(pState->GetArg(0));
	CScriptFuncArgFloat* pWidth = static_cast< CScriptFuncArgFloat* >(pState->GetArg(1));
	IEntity* pQuadEntity = m_pEntityManager->CreateQuad(pLenght->m_fValue, pWidth->m_fValue);
	pQuadEntity->Link(m_pScene);
	ostringstream oss;
	int id = m_pEntityManager->GetEntityID(pQuadEntity);
	oss << "Le quad a été créée avec l'identifiant " << id << ".";
	m_pConsole->Println(oss.str());
	pState->SetReturnValue(id);
}

void RayTrace(IScriptState* pState)
{
	CScriptFuncArgFloat* px = static_cast< CScriptFuncArgFloat* >(pState->GetArg(0));
	CScriptFuncArgFloat* py = static_cast< CScriptFuncArgFloat* >(pState->GetArg(1));

	unsigned int w, h;
	m_pRenderer->GetResolution(w, h);
	float logicalx = (px->m_fValue / (float)w - 0.5f) * 2.0f;
	//float logicaly = (py->m_fValue / (float)h - 0.5f) * 2.0f;
	float logicaly = (0.5f - py->m_fValue / (float)h) * 2.0f;
	CVector logicalP1(logicalx, logicaly, -1.f);
	CVector logicalP2(logicalx, logicaly, 1.f);
	CMatrix V, M, P;
	m_pCameraManager->GetActiveCamera()->GetWorldMatrix().GetInverse(V);
	m_pRenderer->GetProjectionMatrix(P);

	CMatrix PVM = P * V * M;
	CMatrix PVMInv;
	PVM.GetInverse(PVMInv);

	CMatrix m;
	CVector p1 = PVMInv * logicalP1;
	CVector p2 = PVMInv * logicalP2;

	IEntity* pLine = m_pEntityManager->CreateLineEntity(p1, p2);
	pLine->Link(m_pScene);
}

void CreateRepere( IScriptState* pState )
{
	m_pRepere = m_pEntityManager->CreateRepere(*m_pRenderer);
	m_pRepere->Link(m_pScene);
	ostringstream oss;
	int id = m_pEntityManager->GetEntityID(m_pRepere);
	oss << "Le repère a été créé avec l'identifiant " << id  << ".";
	m_pConsole->Println( oss.str() );
	pState->SetReturnValue(id);
}

void ChangeBase( IScriptState* pState )
{
	CScriptFuncArgInt* pEntity1ID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgInt* pEntity2ID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 1 ) );
	IEntity* pEntity1 = static_cast< IEntity* >( m_pEntityManager->GetEntity( pEntity1ID->m_nValue ) );
	IEntity* pEntity2 = static_cast< IEntity* >( m_pEntityManager->GetEntity( pEntity2ID->m_nValue ) );
	CMatrix oWorld1, oWorld1Inv, oWorld2, oNewWorld2, id;
	pEntity1->GetWorldMatrix( oWorld1 );
	oWorld1.GetInverse( oWorld1Inv );
	pEntity2->GetWorldMatrix( oWorld2 );
	oNewWorld2 = oWorld1Inv * oWorld2;
	pEntity1->SetLocalMatrix( id );
	pEntity2->SetLocalMatrix( oNewWorld2 );
}

void SetPreferedKeyBBox( IScriptState* pState )
{
	CScriptFuncArgString* pFileName = static_cast< CScriptFuncArgString* >( pState->GetArg( 0 ) );
	CScriptFuncArgString* pObjectName = static_cast< CScriptFuncArgString* >( pState->GetArg( 1 ) );
	CScriptFuncArgString* pAnimationName = static_cast< CScriptFuncArgString* >( pState->GetArg( 2 ) );
	CScriptFuncArgInt* pKey = static_cast< CScriptFuncArgInt* >( pState->GetArg( 3 ) );
	string sFileName = pFileName->m_sValue;
	if( sFileName.find( ".bme" ) == -1 )
		sFileName += ".bme";
	string sObjectName = pObjectName->m_sValue;
	std::transform( pObjectName->m_sValue.begin(), pObjectName->m_sValue.end(), sObjectName.begin(), tolower );
	string sAnimationName = pAnimationName->m_sValue;
	if( sAnimationName.find( ".bke" ) == -1 )
		sAnimationName += ".bke";
	
	ILoader::CAnimatableMeshData ami;
	m_pLoaderManager->Load( sFileName, ami );
	unsigned int i = 0;
	bool bFind = false;
	for( i = 0; i < ami.m_vMeshes.size(); i++ )
	{
		string sObjectNameLow = ami.m_vMeshes[ i ].m_sName;
		std::transform( ami.m_vMeshes[ i ].m_sName.begin(), ami.m_vMeshes[ i ].m_sName.end(), sObjectNameLow.begin(), tolower );
		if( sObjectNameLow == sObjectName )
		{
			bFind = true;
			break;
		}
	}
	if( bFind )
	{
		ILoader::CMeshInfos& mi = ami.m_vMeshes[ i ];
		string sAnimationNameWithoutExt = sAnimationName.substr( 0, sAnimationName.size() - 4 );
		string sAnimationNameWithoutExtLow = sAnimationNameWithoutExt;
		std::transform( sAnimationNameWithoutExt.begin(), sAnimationNameWithoutExt.end(), sAnimationNameWithoutExtLow.begin(), tolower );
		mi.m_oPreferedKeyBBox[ sAnimationNameWithoutExtLow ] = pKey->m_nValue;
		m_pLoaderManager->Export( sFileName, ami );
	}
	else
		m_pConsole->Println( "L'objet entré en argument n'existe pas dans le fichier indiqué" );
}

void SetLife(IScriptState* pState)
{
	CScriptFuncArgInt* pEntityId = static_cast< CScriptFuncArgInt* >(pState->GetArg(0));
	CScriptFuncArgInt* pLife = static_cast< CScriptFuncArgInt* >(pState->GetArg(1));
	IEntity* pEntity = m_pEntityManager->GetEntity(pEntityId->m_nValue);
	IFighterEntityInterface* pFighter = dynamic_cast<IFighterEntityInterface*>(pEntity);
	if(pFighter)
		pFighter->SetLife(pLife->m_nValue);
}

void Attack(IScriptState* pState)
{
	CScriptFuncArgInt* pAgressorId = static_cast< CScriptFuncArgInt* >(pState->GetArg(0));
	CScriptFuncArgInt* pVictimId = static_cast< CScriptFuncArgInt* >(pState->GetArg(1));
	IAEntityInterface* pAgressor = dynamic_cast<IAEntityInterface*>(m_pEntityManager->GetEntity(pAgressorId->m_nValue));
	if (pAgressor) {
		IFighterEntityInterface* pVictim = dynamic_cast<IFighterEntityInterface*>(m_pEntityManager->GetEntity(pVictimId->m_nValue));
		if (pVictim) {
			pAgressor->Attack(pVictim);
		}
	}
}


vector<unsigned char> vCallbackByteCode;
void TalkToCallback(IAEntityInterface* pThis, IFighterEntityInterface* pInterlocutor)
{
	m_pScriptManager->ExecuteByteCode(vCallbackByteCode);
}

void TalkTo(IScriptState* pState)
{
	CScriptFuncArgInt* pTalkerId = static_cast< CScriptFuncArgInt* >(pState->GetArg(0));
	CScriptFuncArgInt* pInterlocutorId = static_cast< CScriptFuncArgInt* >(pState->GetArg(1));
	CScriptFuncArgString* pCallback = static_cast< CScriptFuncArgString* >(pState->GetArg(2));

	string sTalkToScriptCallback;
	sTalkToScriptCallback = pCallback->m_sValue;

	IAEntityInterface* pTalker = dynamic_cast<IAEntityInterface*>(m_pEntityManager->GetEntity(pTalkerId->m_nValue));
	if (pTalker) {
		IFighterEntityInterface* pInterlocutor = dynamic_cast<IFighterEntityInterface*>(m_pEntityManager->GetEntity(pInterlocutorId->m_nValue));
		if (pInterlocutor) {
			if (!sTalkToScriptCallback.empty()) {
				pTalker->TalkTo(pInterlocutor, TalkToCallback);
				m_pScriptManager->Compile(sTalkToScriptCallback + "();", vCallbackByteCode);
			}
			else
				pTalker->TalkTo(pInterlocutor);
		}
	}
}

void ComputeKeysBoundingBoxes( IScriptState* pState )
{
	CScriptFuncArgString* pFileName = static_cast< CScriptFuncArgString* >( pState->GetArg( 0 ) );
	CScriptFuncArgString* pObjectName = static_cast< CScriptFuncArgString* >( pState->GetArg( 1 ) );
	CScriptFuncArgString* pAnimationName = static_cast< CScriptFuncArgString* >( pState->GetArg( 2 ) );
	string sFileName = pFileName->m_sValue;
	if( sFileName.find( ".bme" ) == -1 )
		sFileName += ".bme";
	string sObjectName = pObjectName->m_sValue;
	std::transform( pObjectName->m_sValue.begin(), pObjectName->m_sValue.end(), sObjectName.begin(), tolower );
	string sAnimationName = pAnimationName->m_sValue;
	if( sAnimationName.find( ".bke" ) == -1 )
		sAnimationName += ".bke";
	
	ILoader::CAnimatableMeshData oData;
	m_pLoaderManager->Load( sFileName, oData );
	unsigned int i = 0;
	bool bFind = false;
	for( i = 0; i < oData.m_vMeshes.size(); i++ )
	{
		string sObjectNameLow = oData.m_vMeshes[ i ].m_sName;
		std::transform( oData.m_vMeshes[ i ].m_sName.begin(), oData.m_vMeshes[ i ].m_sName.end(), sObjectNameLow.begin(), tolower );
		if( sObjectNameLow == sObjectName )
		{
			bFind = true;
			break;
		}
	}
	if( bFind )
	{
		ILoader::CMeshInfos& mi = oData.m_vMeshes[ i ];
		IAnimation* pAnimation = static_cast< IAnimation* >( m_pRessourceManager->GetRessource( sAnimationName) );
		map< int, vector< CKey > > mBoneKeys;
		pAnimation->GetBoneKeysMap( mBoneKeys );
		IWeightTable* pWeightTable = m_pGeometryManager->CreateWeightTable();
		pWeightTable->BuildFromArrays( mi.m_vWeightVertex, mi.m_vWeigtedVertexID );

		map< int, int > mKeys;
		for( map< int, vector < CKey > >::iterator itBone = mBoneKeys.begin(); itBone != mBoneKeys.end(); itBone++ )
		{
			vector< CKey >& vKey = itBone->second;
			for( unsigned int i = 0; i < vKey.size(); i++ )
				mKeys[ vKey[ i ].m_nTimeValue ] = i;
		}

		mi.m_oKeyBoundingBoxes.clear();
		for( map< int, int >::iterator itKey = mKeys.begin(); itKey != mKeys.end(); itKey++ )
		{
			// On récupère la matrice de passage de tous les bones du squelette
			map< int, CMatrix > mPassage;
			for( ILoader::TSkeletonMap::const_iterator itBone = oData.m_mBones.begin(); itBone != oData.m_mBones.end(); itBone++ )
			{
				// On récupère la matrice de passage de la clé initiale à la clé courante
				CMatrix oPassage;
				map< int, vector< CKey > >::const_iterator itBoneKey = mBoneKeys.find( itBone->first );
				if( itBoneKey != mBoneKeys.end() )
				{
					// On récupère la matrice de la clé courante
					int nCurrentKeyIndex = 0;
					CMatrix oFirstKeyWorldTM, oCurrentKeyWorldTM, oFirstKeyWorldTMInv;
					bool bFoundKey = false;
					for( unsigned int iKey = 0; iKey < itBoneKey->second.size(); iKey++ )
					{
						if( itBoneKey->second.at( iKey ).m_nTimeValue == itKey->first )
						{
							oCurrentKeyWorldTM = itBoneKey->second.at( iKey ).m_oWorldTM;
							bFoundKey = true;
							break;
						}
					}
					if( !bFoundKey )
					{
						// Si il n'existe pas de clé à cette position de l'animation, on cherche la précédent et la suivante
						unsigned int nLastTimeKey = 0, nNextTimeKey = -1;
						unsigned int nLastKeyIndex = 0, nNextKeyIndex = -1;
						for( unsigned int iKey = 0; iKey < itBoneKey->second.size(); iKey++ )
						{
							if( itBoneKey->second.at( iKey ).m_nTimeValue <  itKey->first )
							{
								if( nLastTimeKey < itBoneKey->second.at( iKey ).m_nTimeValue )
								{
									nLastTimeKey = itBoneKey->second.at( iKey ).m_nTimeValue;
									nLastKeyIndex = iKey;
								}
							}
							else 
							{
								if( nNextTimeKey > itBoneKey->second.at( iKey ).m_nTimeValue )
								{
									nNextTimeKey = itBoneKey->second.at( iKey ).m_nTimeValue;
									nNextKeyIndex = iKey;
								}
							}
						}
						// Une fois qu'on les a trouvé, on calcule la matrice interpolée entre ces deux clés
						if( nNextKeyIndex == -1 )
							nNextKeyIndex = 0;
						const CMatrix& oLast = itBoneKey->second.at( nLastKeyIndex ).m_oWorldTM;
						const CMatrix& oNext = itBoneKey->second.at( nNextKeyIndex ).m_oWorldTM;
						float t = float( itKey->first - nLastTimeKey ) / ( nNextTimeKey - nLastTimeKey );
						CMatrix::GetInterpolationMatrix( oLast, oNext, oCurrentKeyWorldTM, t );
					}
					// On récupère la matrice de la 1ere clé :
					oFirstKeyWorldTM = itBoneKey->second.at( 0 ).m_oWorldTM;
					oFirstKeyWorldTM.GetInverse( oFirstKeyWorldTMInv );
					oPassage = oCurrentKeyWorldTM * oFirstKeyWorldTMInv;
				}
				mPassage[ itBone->first ] = oPassage;
			}

			// On parcours tous les vertex de la weight table et on calcul leur image
			IBox* pBox = m_pGeometryManager->CreateBox();
			map< int, bool > mComputedVertexIndex;
			for( unsigned int i = 0; i < mi.m_vIndex.size(); i++ )
			{
				int iIndex = mi.m_vIndex[ i ];
				map< int, bool >::const_iterator itVertexIndex = mComputedVertexIndex.find( iIndex );
				if( itVertexIndex == mComputedVertexIndex.end() )
				{
					mComputedVertexIndex[ iIndex ] = true;
					CVector v = CVector( mi.m_vVertex[ 3 * iIndex ], mi.m_vVertex[ 3 * iIndex + 1 ], mi.m_vVertex[ 3 * iIndex + 2 ] );
					map< int, float > mWeight;
					pWeightTable->Get( iIndex, mWeight );
					CMatrix oWeightedMatrix( 0.f );
					for( map< int, float >::const_iterator itBoneWeight = mWeight.begin(); itBoneWeight != mWeight.end(); itBoneWeight++ )
					{
						map< int, CMatrix >::const_iterator itPassage = mPassage.find( itBoneWeight->first );
						oWeightedMatrix = oWeightedMatrix + ( itPassage->second * itBoneWeight->second );
					}
					CVector v2 = oWeightedMatrix * v;
					pBox->AddPoint( v2 );
				}
			}
			string sAnimationNameWithoutExt = sAnimationName.substr( 0, sAnimationName.size() - 4 );
			mi.m_oKeyBoundingBoxes[ sAnimationNameWithoutExt ][ itKey->first ] = pBox;
		}
		m_pLoaderManager->Export( sFileName, oData );
	}
}

void LocalTranslate( IScriptState* pState )
{
	CScriptFuncArgInt* pEntityID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pEntityID->m_nValue );
	if( pEntity )
	{
		CScriptFuncArgFloat* px = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 1 ) );
		CScriptFuncArgFloat* py = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 2 ) );
		CScriptFuncArgFloat* pz = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 3 ) );
		pEntity->LocalTranslate( px->m_fValue, py->m_fValue, pz->m_fValue );
	}
	else
	{
		ostringstream oss;
		oss << "Erreur : entité " << pEntityID->m_nValue << " introuvable";
		m_pConsole->Println( oss.str() );
	}
}

ICameraManager::TCameraType GetCamTypeByString(string sCamType)
{
	if (sCamType == "link")
		return ICameraManager::TLinked;
	if(sCamType == "free")
		return ICameraManager::TFree;
	if (sCamType == "map")
		return ICameraManager::TMap;
}

void SetCameraType( IScriptState* pState )
{
	CScriptFuncArgString* pCamType = static_cast< CScriptFuncArgString* >( pState->GetArg( 0 ) );
	ICamera* pCamera = m_pCameraManager->GetCameraFromType(GetCamTypeByString(pCamType->m_sValue));
	if (pCamera) {
		m_pCameraManager->SetActiveCamera(pCamera);

		if (pCamType->m_sValue == "link")
		{
			IPlayer* player = m_pEntityManager->GetPlayer();
			if (player)
				pCamera->Link(dynamic_cast<IEntity*>(player));
			else
				m_pConsole->Println("Erreur : vous devez définir un personnage (fonction SetCurrentPlayer(persoID)) avant de définir une caméra liée.");
		}
	}
	else {
		ostringstream oss;
		oss << "Erreur, camera " << pCamType->m_sValue << " inexistante.";
		m_pConsole->Println(oss.str());
	}
}

void DisplayCamera(IScriptState* pState)
{
	CScriptFuncArgString* pCamType = static_cast< CScriptFuncArgString* >(pState->GetArg(0));
	CScriptFuncArgInt* pDisplay = static_cast< CScriptFuncArgInt* >(pState->GetArg(1));
	ICamera* pCamera = m_pCameraManager->GetCameraFromType(GetCamTypeByString(pCamType->m_sValue));
	if(pCamera)
		pCamera->DisplayViewCone(pDisplay->m_nValue > 0 ? true : false);
	else {
		ostringstream oss;
		oss << "Erreur : camera \"" << pCamType->m_sValue << "\" inexistante";
		m_pConsole->Print(oss.str());
	}
}

void InitCamera(IScriptState* pState)
{
	CScriptFuncArgString* pCamtype = (CScriptFuncArgString*) pState->GetArg(0);
	ICamera* pCamera = m_pCameraManager->GetCameraFromType(GetCamTypeByString(pCamtype->m_sValue));
	if (pCamera) {
		CMatrix m;
		pCamera->SetLocalMatrix(m);
	}
	else {
		m_pConsole->Println("Erreur : camera inexistante");
	}
}

void GetCameraID(IScriptState* pState)
{
	CScriptFuncArgString* pType = (CScriptFuncArgString*)pState->GetArg(0);
	ICameraManager::TCameraType type = ICameraManager::TFree;
	if(pType->m_sValue == "link")
		type = ICameraManager::TLinked;
	else if(pType->m_sValue == "map")
		type = ICameraManager::TMap;
	ICamera* pCamera = m_pCameraManager->GetCameraFromType(type);
	pState->SetReturnValue(m_pEntityManager->GetEntityID(pCamera));
}

void SetCurrentPlayer( IScriptState* pState )
{
	CScriptFuncArgInt* pPlayerID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	IPlayer* pPlayer = dynamic_cast<IPlayer*>(m_pEntityManager->GetEntity(pPlayerID->m_nValue ));
	if(pPlayer)
		m_pEntityManager->SetPlayer( dynamic_cast<IPlayer*>(pPlayer) );
	else
	{
		ostringstream oss;
		oss << "Erreur : SetCurrentPlayer(" << pPlayerID->m_nValue << ") -> Id not exists";
		m_pConsole->Println(oss.str());
	}		
}

void GetPlayerId(IScriptState* pState)
{
	IPlayer* pPlayer = m_pEntityManager->GetPlayer();
	pState->SetReturnValue(m_pEntityManager->GetEntityID(pPlayer));
}

void SetGravity( IScriptState* pState )
{
	CScriptFuncArgFloat* pGravity = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 0 ) );
	m_pPhysic->SetGravity(pGravity->m_fValue);
}

void DisplayNodeInfos( IScriptState* pState )
{
	CScriptFuncArgInt* pEntityID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgInt* pNodeID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 1 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pEntityID->m_nValue );
	if( pEntity )
	{
		IBone* pBone = static_cast< IBone* >( pEntity->GetSkeletonRoot()->GetChildBoneByID( pNodeID->m_nValue ) );		
		string sBoneName;
		pBone->GetName( sBoneName );
		sBoneName = string( "Bone \"" ) + sBoneName + "\" : ";
		m_pConsole->Println( sBoneName );
		CMatrix oTM;
		pBone->GetLocalMatrix( oTM );
		string sTM;
		m_pDebugTool->SetNumberWidth( 10 );
		m_pDebugTool->SerializeMatrix( oTM, 0.f, sTM );
		m_pConsole->Println( "Matrice locale : " );
		m_pConsole->Println( sTM );
		m_pDebugTool->SerializeMatrix( oTM, 0.f, sTM );
		m_pConsole->Println( "Matrice world : " );
		m_pConsole->Println( sTM );
	}
	else
	{
		ostringstream ossMessage;
		ossMessage << "Erreur d'identifiant pour l'entité " << pEntityID->m_nValue;
		m_pConsole->Println( ossMessage.str() );
	}
}

void Debug(IScriptState* pState)
{
	CScriptFuncArgInt* pValue = static_cast< CScriptFuncArgInt* >(pState->GetArg(0));
	pValue = pValue;
}

void SetCurrentAnimationSpeed(IScriptState* pState)
{
	CScriptFuncArgInt* pEntityID = static_cast< CScriptFuncArgInt* >(pState->GetArg(0));
	CScriptFuncArgFloat* pSpeed = static_cast< CScriptFuncArgFloat* >(pState->GetArg(1));
	IEntity* pEntity = m_pEntityManager->GetEntity(pEntityID->m_nValue);
	if (pEntity)
	{
		IAnimation* pAnimation = pEntity->GetCurrentAnimation();
		if (pAnimation) {
			pEntity->GetCurrentAnimation()->SetSpeed(pSpeed->m_fValue);
		}
		else {
			ostringstream oss;
			oss << "Erreur, l'entité sélectionnée ne contient pas d'animation courante";
			m_pConsole->Println(oss.str());
		}
	}
	
}

void SetAnimationSpeed( IScriptState* pState )
{
	CScriptFuncArgInt* pEntityID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgString* pAnimationName = static_cast< CScriptFuncArgString* >( pState->GetArg( 1 ) );
	CScriptFuncArgFloat* pSpeed = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 2 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pEntityID->m_nValue );
	if( pEntity )
	{
		transform(pAnimationName->m_sValue.begin(), pAnimationName->m_sValue.end(), pAnimationName->m_sValue.begin(), tolower);
		IEntity::TAnimation eAnim = IEntity::eNone;
		if( pAnimationName->m_sValue == "stand" )
			eAnim = IEntity::eStand;
		else if( pAnimationName->m_sValue == "walk" )
			eAnim = IEntity::eWalk;
		else if( pAnimationName->m_sValue == "run" )
			eAnim = IEntity::eRun;
		else if (pAnimationName->m_sValue == "dying")
			eAnim = IEntity::eDying;
		else {
			ostringstream oss;
			oss << "Erreur, parametre \"" << pAnimationName->m_sValue << "\" incorrect, valeurs possibles : stand, walk, run, dying" ;
			m_pConsole->Println(oss.str());
			return;
		}
		pEntity->SetAnimationSpeed( eAnim, pSpeed->m_fValue );
	}
	else
	{
		ostringstream oss;
		oss << "Erreur : entité " << pEntityID->m_nValue << " inconnue";
		m_pConsole->Println( oss.str() );
	}
}

void StopRender( IScriptState* pState )
{
	CScriptFuncArgInt* pRender = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	m_bRenderScene = pRender->m_nValue == 1 ? false : true;
}

void StopUpdateEntity(IScriptState* pState)
{
	CScriptFuncArgInt* pEntityId = static_cast< CScriptFuncArgInt* >(pState->GetArg(0));
	//m_pEntityManager-
}

void Walk( IScriptState* pState )
{
	CScriptFuncArgInt* pEntityID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pEntityID->m_nValue );
	//pEntity->Walk();
	if(pEntity)
		pEntity->RunAction( "Walk", true );
	else {
		ostringstream oss;
		oss << "Error : entity " << pEntityID->m_nValue << " not found";
		m_pConsole->Println(oss.str());
	}
}

void Run( IScriptState* pState )
{
	CScriptFuncArgInt* pEntityID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pEntityID->m_nValue );
	if (pEntity)
		pEntity->RunAction("Run", true);
	else
	{
		ostringstream oss;
		oss << "Error : Entity " << pEntityID->m_nValue << " not found";
		m_pConsole->Println(oss.str());;
	}
}

void Stand( IScriptState* pState )
{
	CScriptFuncArgInt* pEntityID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pEntityID->m_nValue );
	if( pEntity )
		//pEntity->Stand();
		pEntity->RunAction( "Stand", true );
	else
	{
		ostringstream oss;
		oss << "Erreur : Entité " << pEntityID->m_nValue << " inconnue";
		m_pConsole->Println( oss.str() );
	}
}

void RunAction( IScriptState* pState )
{
	CScriptFuncArgInt* pEntityID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pEntityID->m_nValue );
	if( pEntity )
	{
		CScriptFuncArgString* pAction = static_cast< CScriptFuncArgString* >( pState->GetArg( 1 ) );
		CScriptFuncArgInt* pLoop = static_cast< CScriptFuncArgInt* >( pState->GetArg( 2 ) );
		bool bLoop = pLoop->m_nValue == 1 ? true : false;
		pEntity->RunAction( pAction->m_sValue, bLoop );
	}
	else
	{
		ostringstream oss;
		oss << "Erreur : Entité " << pEntityID->m_nValue << " inconnue";
		m_pConsole->Println( oss.str() );
	}
}



void GenerateRandomNPC(IScriptState* pState)
{
	CScriptFuncArgString* pNPCFileName = (CScriptFuncArgString*)pState->GetArg(0);
	CScriptFuncArgString* pArmorName = (CScriptFuncArgString*)pState->GetArg(1);
	CScriptFuncArgInt* pNPCCount = (CScriptFuncArgInt*)pState->GetArg(2);
	CScriptFuncArgInt* pPercentRadius = (CScriptFuncArgInt*)pState->GetArg(3);


	srand((unsigned)time(NULL));
	ostringstream ossNPCId;	

	for (int i = 0; i < pNPCCount->m_nValue; i++) {
		ossNPCId << "NPC_" << i;
		IEntity* pEntity = m_pEntityManager->CreateNPC(pNPCFileName->m_sValue, ossNPCId.str());
		pEntity->Link(m_pScene);
		int id = m_pEntityManager->GetEntityID(pEntity);
		ostringstream oss;
		oss << "L'entité \"" << pNPCFileName->m_sValue << "\"a été chargée avec l'identifiant " << id << ".";
		m_pConsole->Println(oss.str());

		pEntity->Link(m_pScene);
		IBox* pBox = static_cast<IBox*>(m_pScene->GetBoundingGeometry());
		pBox->GetDimension();
		float r = rand();
		float factor = (float)pPercentRadius->m_nValue / 100.f;
		float x = factor * pBox->GetDimension().m_x * (r - 0.5f * RAND_MAX) / RAND_MAX;
		float y = 2000.f;
		r = rand();
		float z = factor * pBox->GetDimension().m_z * (r - 0.5f * RAND_MAX) / RAND_MAX;
		pEntity->SetWorldPosition(x, y, z);
		m_pEntityManager->WearArmorToDummy(id, "2");
		pEntity->RunAction("stand", true);
		float angle = 360 * r / RAND_MAX;
		pEntity->Yaw(angle);
	}
}

void SetScale( IScriptState* pState )
{
	CScriptFuncArgInt* pEntityID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pEntityID->m_nValue );
	if( pEntity )
	{
		CScriptFuncArgFloat* px = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 1 ) );
		CScriptFuncArgFloat* py = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 2 ) );
		CScriptFuncArgFloat* pz = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 3 ) );
		pEntity->SetScaleFactor( px->m_fValue, py->m_fValue, pz->m_fValue );
	}
	else
	{
		ostringstream oss;
		oss << "Erreur : Entité " << pEntityID->m_nValue << " inconnue";
		m_pConsole->Println( oss.str() );
	}
}

void CreateMobileEntity( IScriptState* pState )
{
	CScriptFuncArgString* pName = static_cast< CScriptFuncArgString* >( pState->GetArg( 0 ) );
	CScriptFuncArgString* pStringID = static_cast< CScriptFuncArgString* >(pState->GetArg(1));
	string sName = pName->m_sValue;
	if( sName.find( ".bme" ) == -1 )
		sName += ".bme";
	bool bak = m_pRessourceManager->IsCatchingExceptionEnabled();
	m_pRessourceManager->EnableCatchingException( false );

	try
	{
		IEntity* pEntity = m_pEntityManager->CreateMobileEntity( sName, m_pFileSystem, pStringID->m_sValue);
		pEntity->Link( m_pScene );
		int id = m_pEntityManager->GetEntityID(pEntity);
		ostringstream oss;
		oss << "L'entité \"" << pName->m_sValue << "\"a été chargée avec l'identifiant " << id << ".";
		m_pConsole->Println( oss.str() );
		pState->SetReturnValue((float)id);
	}
	catch( CFileNotFoundException& e )
	{		
		ostringstream oss;
		oss <<"Erreur : fichier \"" << e.m_sFileName << "\" manquant, l'entité \"" << pName->m_sValue << "\" ne peut pas être chargée." ;
		m_pConsole->Println( oss.str() );
	}
	catch( CRessourceException& e )
	{
		string s;
		e.GetErrorMessage( s );
		m_pConsole->Println( s );
	}
	catch( CBadFileFormat& e )
	{
		string sMessage;
		e.GetErrorMessage( sMessage );
		m_pConsole->Println( sMessage );
	}
	catch( CEException )
	{
		string sMessage = string( "\"" ) + sName + "\" introuvable";
		m_pConsole->Println( sMessage );
	}
	m_pRessourceManager->EnableCatchingException( bak );
}

void GetVec3DFromArg(IScriptState* pState, int argIdx, CVector& v)
{
	v.m_x = ((CScriptFuncArgFloat*)pState->GetArg(argIdx))->m_fValue;
	v.m_y = ((CScriptFuncArgFloat*)pState->GetArg(argIdx + 1))->m_fValue;
	v.m_z = ((CScriptFuncArgFloat*)pState->GetArg(argIdx + 2))->m_fValue;
}

void CreateLineEntity(IScriptState* pState)
{
	CVector first, last;
	GetVec3DFromArg(pState, 0, first);
	GetVec3DFromArg(pState, 3, last);
	IEntity* pLine = m_pEntityManager->CreateLineEntity(first, last);
	int id = m_pEntityManager->GetEntityID(pLine);
	pLine->Link(m_pScene);
	pState->SetReturnValue(id);
	ostringstream oss;
	oss << "La ligne a été créée avec l'identifiant " << id;
	m_pConsole->Println(oss.str());
}

void CreateNPC( IScriptState* pState )
{
	CScriptFuncArgString* pName = static_cast< CScriptFuncArgString* >( pState->GetArg( 0 ) );
	CScriptFuncArgString* pID = static_cast< CScriptFuncArgString* >(pState->GetArg(1));
	string sFileName = pName->m_sValue;
	bool bak = m_pRessourceManager->IsCatchingExceptionEnabled();
	m_pRessourceManager->EnableCatchingException( false );

	try
	{
		IEntity* pEntity = m_pEntityManager->CreateNPC(sFileName, pID->m_sValue);
		pEntity->Link( m_pScene );
		int id = m_pEntityManager->GetEntityID(pEntity);
		ostringstream oss;
		oss << "L'entité \"" << pName->m_sValue << "\"a été chargée avec l'identifiant " << id << ".";
		m_pConsole->Println( oss.str() );
		pState->SetReturnValue(id);
	}
	catch( CFileNotFoundException& e )
	{		
		ostringstream oss;
		oss <<"Erreur : fichier \"" << e.m_sFileName << "\" manquant, l'entité \"" << pName->m_sValue << "\" ne peut pas être chargée." ;
		m_pConsole->Println( oss.str() );
	}
	catch( CRessourceException& e )
	{
		string s;
		e.GetErrorMessage( s );
		m_pConsole->Println( s );
	}
	catch( CBadFileFormat& e )
	{
		string sMessage;
		e.GetErrorMessage( sMessage );
		m_pConsole->Println( sMessage );
	}
	catch( CEException )
	{
		string sMessage = string( "\"" ) + sFileName + "\" introuvable";
		m_pConsole->Println( sMessage );
	}
	m_pRessourceManager->EnableCatchingException( bak );
}


void CreatePlayer(IScriptState* pState)
{
	CScriptFuncArgString* pName = static_cast< CScriptFuncArgString* >(pState->GetArg(0));
	string sName = pName->m_sValue;
	bool bak = m_pRessourceManager->IsCatchingExceptionEnabled();
	m_pRessourceManager->EnableCatchingException(false);

	try
	{
		IEntity* pEntity = m_pEntityManager->CreatePlayer(sName);
		pEntity->Link(m_pScene);
		int id = m_pEntityManager->GetEntityID(pEntity);
		ostringstream oss;
		oss << "L'entité \"" << pName->m_sValue << "\"a été chargée avec l'identifiant " << id << ".";
		m_pConsole->Println(oss.str());
		pState->SetReturnValue(id);
	}
	catch (CFileNotFoundException& e)
	{
		ostringstream oss;
		oss << "Erreur : fichier \"" << e.m_sFileName << "\" manquant, l'entité \"" << pName->m_sValue << "\" ne peut pas être chargée.";
		m_pConsole->Println(oss.str());
	}
	catch (CRessourceException& e)
	{
		string s;
		e.GetErrorMessage(s);
		m_pConsole->Println(s);
	}
	catch (CBadFileFormat& e)
	{
		string sMessage;
		e.GetErrorMessage(sMessage);
		m_pConsole->Println(sMessage);
	}
	catch (CEException)
	{
		string sMessage = string("\"") + sName + "\" introuvable";
		m_pConsole->Println(sMessage);
	}
	m_pRessourceManager->EnableCatchingException(bak);
}

void SaveCharacter(IScriptState* pState)
{
	m_pCharacterEditor->Save();
	m_pConsole->Println("Sauvegarde réussie");
}

void SaveCharacterInWorld(IScriptState* pState)
{
	CScriptFuncArgString* pID = (CScriptFuncArgString*)(pState->GetArg(0));
	m_pEntityManager->SaveCharacter(pID->m_sValue);
}

void RemoveCharacterFromWorld(IScriptState* pState)
{
	CScriptFuncArgString* pID = static_cast< CScriptFuncArgString* >(pState->GetArg(0));
	if(m_pWorldEditor->IsEnabled())
		m_pWorldEditor->RemoveCharacter(pID->m_sValue);
}

void RemoveCharacterFromDB(IScriptState* pState)
{
	CScriptFuncArgString* pID = static_cast< CScriptFuncArgString* >(pState->GetArg(0));
	m_pEntityManager->RemoveCharacterFromDB(pID->m_sValue);
}

void CreateMinimapEntity(IScriptState* pState)
{
	CScriptFuncArgString* pName = static_cast< CScriptFuncArgString* >(pState->GetArg(0));
	string sName = pName->m_sValue;
	if (sName.find(".bme") == -1)
		sName += ".bme";
	bool bak = m_pRessourceManager->IsCatchingExceptionEnabled();
	m_pRessourceManager->EnableCatchingException(false);

	try
	{
		IEntity* pEntity = m_pEntityManager->CreateMinimapEntity(sName, m_pFileSystem);
		pEntity->Link(m_pScene);
		int id = m_pEntityManager->GetEntityID(pEntity);
		ostringstream oss;
		oss << "L'entité \"" << pName->m_sValue << "\"a été chargée avec l'identifiant " << id << ".";
		m_pConsole->Println(oss.str());
		pState->SetReturnValue(id);
	}
	catch (CFileNotFoundException& e)
	{
		ostringstream oss;
		oss << "Erreur : fichier \"" << e.m_sFileName << "\" manquant, l'entité \"" << pName->m_sValue << "\" ne peut pas être chargée.";
		m_pConsole->Println(oss.str());
	}
	catch (CRessourceException& e)
	{
		string s;
		e.GetErrorMessage(s);
		m_pConsole->Println(s);
	}
	catch (CBadFileFormat& e)
	{
		string sMessage;
		e.GetErrorMessage(sMessage);
		m_pConsole->Println(sMessage);
	}
	catch (CEException)
	{
		string sMessage = string("\"") + sName + "\" introuvable";
		m_pConsole->Println(sMessage);
	}
	m_pRessourceManager->EnableCatchingException(bak);
}

void CreateTestEntity(IScriptState* pState)
{
	CScriptFuncArgString* pName = static_cast< CScriptFuncArgString* >(pState->GetArg(0));
	string sName = pName->m_sValue;
	if (sName.find(".bme") == -1)
		sName += ".bme";
	bool bak = m_pRessourceManager->IsCatchingExceptionEnabled();
	m_pRessourceManager->EnableCatchingException(false);

	try
	{
		IEntity* pEntity = m_pEntityManager->CreateTestEntity(sName, m_pFileSystem);
		pEntity->Link(m_pScene);
		int id = m_pEntityManager->GetEntityID(pEntity);
		ostringstream oss;
		oss << "L'entité \"" << pName->m_sValue << "\"a été chargée avec l'identifiant " << id << ".";
		m_pConsole->Println(oss.str());
		pState->SetReturnValue(id);
	}
	catch (CFileNotFoundException& e)
	{
		ostringstream oss;
		oss << "Erreur : fichier \"" << e.m_sFileName << "\" manquant, l'entité \"" << pName->m_sValue << "\" ne peut pas être chargée.";
		m_pConsole->Println(oss.str());
	}
	catch (CRessourceException& e)
	{
		string s;
		e.GetErrorMessage(s);
		m_pConsole->Println(s);
	}
	catch (CBadFileFormat& e)
	{
		string sMessage;
		e.GetErrorMessage(sMessage);
		m_pConsole->Println(sMessage);
	}
	catch (CEException)
	{
		string sMessage = string("\"") + sName + "\" introuvable";
		m_pConsole->Println(sMessage);
	}
	m_pRessourceManager->EnableCatchingException(bak);
}

void DisplayAnimationTime( IScriptState* pState )
{
	CScriptFuncArgInt* pEntityID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pEntityID->m_nValue );
	if( pEntity )
	{
		ostringstream oss;
		oss << pEntity->GetCurrentAnimation()->GetAnimationTime();
		m_pConsole->Println( oss.str() );
	}
	else
		m_pConsole->Println( "Erreur : identifiant incorrect" );
}

void SetAnimationTime( IScriptState* pState )
{
	CScriptFuncArgInt* pEntityID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgInt* pFrame = static_cast< CScriptFuncArgInt* >( pState->GetArg( 1 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pEntityID->m_nValue );
	if( pEntity )
		pEntity->GetCurrentAnimation()->SetAnimationTime( pFrame->m_nValue );
	else
		m_pConsole->Println( "Erreur : identifiant incorrect" );
}

void NextAnimationKey( IScriptState* pState )
{
	IBone* pBone = dynamic_cast< IBone* >( m_pSelectedNode );
	if( pBone )
	{
		pBone->NextKey();
		pBone->Update();
	}
	else
		m_pConsole->Println( "Erreur : le noeud sélectionné n'est pas un bone" );
}

void NextAnimationFrame( IScriptState* pState )
{
	CScriptFuncArgInt* pEntityID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pEntityID->m_nValue );
	if( pEntity )
		pEntity->GetCurrentAnimation()->NextFrame();
	else
		m_pConsole->Println( "Erreur : identifiant incorrect" );
}

void SetConstantLocalTranslate( IScriptState* pState )
{
	CScriptFuncArgInt* pEntityID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pEntityID->m_nValue );
	if( pEntity )
	{
		CScriptFuncArgFloat* px = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 1 ) );
		CScriptFuncArgFloat* py = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 2 ) );
		CScriptFuncArgFloat* pz = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 3 ) );
		pEntity->ConstantLocalTranslate( CVector( px->m_fValue, py->m_fValue, pz->m_fValue ) );
	}
}

void SetZCollisionError( IScriptState* pState )
{
	CScriptFuncArgFloat* pEpsilon = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 0 ) );
	m_pPhysic->SetZCollisionError( pEpsilon->m_fValue );
}

void GetNodeId(IScriptState* pState)
{
	CScriptFuncArgInt* pEntityId = (CScriptFuncArgInt*)pState->GetArg(0);
	CScriptFuncArgString* pNodeName = (CScriptFuncArgString*)pState->GetArg(1);

	IEntity* pEntity = m_pEntityManager->GetEntity(pEntityId->m_nValue);
	if (pEntity && pEntity->GetSkeletonRoot()) {
		IBone* pBone = dynamic_cast<IBone*>(pEntity->GetSkeletonRoot()->GetChildBoneByName(pNodeName->m_sValue));
		if (pBone) {
			pState->SetReturnValue(pBone->GetID());
			return;
		}
		m_pConsole->Println("Node introuvable");
	}
	pState->SetReturnValue(-1);
}

void LinkToId( IScriptState* pState )
{
	CScriptFuncArgInt* pIDEntity1 = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgInt* pIDNode1 = static_cast< CScriptFuncArgInt* >( pState->GetArg( 1 ) );
	CScriptFuncArgInt* pIDEntity2 = static_cast< CScriptFuncArgInt* >( pState->GetArg( 2 ) );
	CScriptFuncArgInt* pIDNode2 = static_cast< CScriptFuncArgInt* >( pState->GetArg( 3 ) );
	CScriptFuncArgString* pLinkType = static_cast< CScriptFuncArgString* >( pState->GetArg( 4 ) );

	IEntity* pEntity1 = m_pEntityManager->GetEntity( pIDEntity1->m_nValue );
	if( pEntity1 )
	{
		INode* pNode1 = NULL;
		bool bEntity1 = false;
		bool bBone2 = false;
		IBone* pBone2 = NULL;
		if( pIDNode1->m_nValue != -1 )
			pNode1 = pEntity1->GetSkeletonRoot()->GetChildBoneByID( pIDNode1->m_nValue );
		else
		{
			pNode1 = pEntity1;
			bEntity1 = true;
		}

		if( pIDEntity2->m_nValue == -1 )
			pNode1->Unlink();
		else
		{
			IEntity* pEntity2 = m_pEntityManager->GetEntity( pIDEntity2->m_nValue );
			INode* pNode2 = NULL;
			if( pIDNode2->m_nValue != -1 )
			{
				IBone* pSkeletonRoot = pEntity2->GetSkeletonRoot();
				if( pSkeletonRoot )
				{
					pNode2 = pSkeletonRoot->GetChildBoneByID( pIDNode2->m_nValue );
					pBone2 = dynamic_cast< IBone* >( pNode2 );
					if( pBone2 )
						bBone2 = true;
				}
				else
				{
					ostringstream oss;
					oss << "Erreur : l'entité " << pIDEntity2->m_nValue << " ne possède pas de squelette";
					m_pConsole->Println( oss.str() );
					return;
				}
			}
			else
				pNode2 = pEntity2;

			if( bEntity1 && bBone2 )
			{
				IEntity::TLinkType t;
				if( pLinkType->m_sValue == "preserve" )
					t = IEntity::ePreserveChildRelativeTM;
				else if( pLinkType->m_sValue == "settoparent" )
					t = IEntity::eSetChildToParentTM;
				pEntity2->LinkEntityToBone( pEntity1, pBone2, t );
			}
			else {
				if(pNode1)
					pNode1->Link(pNode2);
				else {
					ostringstream oss;
					oss << "Erreur : il n'existe pas de node ayant l'identifiant " << pIDNode1->m_nValue << " dans la premiere entite";
					m_pConsole->Println(oss.str());
					return;
				}
			}
		}
	}
}

void HideEntity( IScriptState* pState )
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgInt* pHide = static_cast< CScriptFuncArgInt* >( pState->GetArg( 1 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pID->m_nValue );
	bool bHide = pHide->m_nValue == 1 ? true : false;
	pEntity->Hide( bHide );
}

void Sleep( IScriptState* pState )
{
	CScriptFuncArgInt* pTime = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	Sleep( pTime->m_nValue );
}

void StopAnimation( IScriptState* pState )
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pID->m_nValue );
	pEntity->GetCurrentAnimation()->Stop();
}

void DetachAnimation( IScriptState* pState )
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pID->m_nValue );
	pEntity->DetachCurrentAnimation();
}

void PauseAnimation( IScriptState* pState )
{
	CScriptFuncArgInt* pIDEntity = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	
	bool bGoodEntityID = true;
	if( pIDEntity )
	{
		CScriptFuncArgInt* pIDBool = static_cast< CScriptFuncArgInt* >( pState->GetArg( 1 ) );
		IEntity* pEntity = m_pEntityManager->GetEntity( pIDEntity->m_nValue );
		if( pEntity )
		{
			bool bPause = pIDBool->m_nValue == 1 ? true : false;
			//pEntity->GetCurrentAnimation()->Pause( bPause );
			pEntity->PauseCurrentAnimation(bPause);
		}
		else
			bGoodEntityID = false;
	}
	else
		bGoodEntityID = false;
	if( !bGoodEntityID )
	{
		ostringstream ossMessage;
		ossMessage << "Erreur, l'identifiant numéro " << pIDEntity->m_nValue << " n'est pas valide";
		m_pConsole->Println( ossMessage.str() );
	}	
}

//ID entité, ID bone
void SelectBone( IScriptState* pState )
{
	CScriptFuncArgInt* pIDEntity = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgString* pBoneName = static_cast< CScriptFuncArgString* >(pState->GetArg(1));
	IEntity* pEntity = m_pEntityManager->GetEntity( pIDEntity->m_nValue );
	if( pEntity )
	{
		IBone* pSkeleton = pEntity->GetSkeletonRoot();
		if( pSkeleton )
		{
			m_pSelectedNode = pSkeleton->GetChildBoneByName(pBoneName->m_sValue);
			if( m_pSelectedNode )
			{
				m_eSelectionType = eBone;
				string sBoneName;
				m_pSelectedNode->GetName( sBoneName );
				string sMessage = string( "Bone \"" ) + sBoneName + "\" sélectionné";
				m_pConsole->Println( sMessage );
			}
			else
				m_pConsole->Println( "Identifiant de bone incorrect" );
		}
		else
			m_pConsole->Println( "Erreur : L'entité sélectionné n'a pas de squelette" );
	}
	else
		m_pConsole->Println( "Identifiant d'entité incorrect" );
}

void Yaw( IScriptState* pState )
{
	if( m_pSelectedNode )
	{
		CScriptFuncArgFloat* pAngle = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 0 ) );
		m_pSelectedNode->Yaw( pAngle->m_fValue );
		m_pSelectedNode->Update();
	}
}

void Pitch( IScriptState* pState )
{
	if( m_pSelectedNode )
	{
		CScriptFuncArgFloat* pAngle = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 0 ) );
		m_pSelectedNode->Pitch( pAngle->m_fValue );
		m_pSelectedNode->Update();
	}
}

void Roll( IScriptState* pState )
{
	if( m_pSelectedNode )
	{
		CScriptFuncArgFloat* pAngle = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 0 ) );
		m_pSelectedNode->Roll( pAngle->m_fValue );
		m_pSelectedNode->Update();
	}
}

void GetSkeletonInfos( INode* pNode, vector< CNodeInfos >& vInfos )
{
	IBone* pBone = dynamic_cast< IBone* >( pNode );
	if( pBone )
	{
		CNodeInfos ni;
		pNode->GetName( ni.m_sName );
		ni.m_nID = pNode->GetID();
		vInfos.push_back( ni );
		for( unsigned int i = 0; i < pNode->GetChildCount(); i++ )
			GetSkeletonInfos( pNode->GetChild( i ), vInfos );
	}
}

void DisplayEntitySkeletonInfos( IScriptState* pState )
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pID->m_nValue );
	if( pEntity )
	{
		vector< CNodeInfos > vInfos;
		GetSkeletonInfos( pEntity->GetSkeletonRoot(), vInfos );
		ostringstream oss;
		for( unsigned int i = 0; i < vInfos.size(); i++ )
		{
			oss.str( "" );
			oss << "Nom : " << vInfos[ i ].m_sName << " , ID : " << vInfos[ i ].m_nID;
			m_pConsole->Println( oss.str() );
		}
	}
	else
		m_pConsole->Println( "Erreur d'identifiant" );
}

void reset( IScriptState* pState )
{
	m_pScene->Clear();
	RunScript( "start.eas" );
}


void SetAnimation( IScriptState* pState )
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >(  pState->GetArg( 0 ) );
	CScriptFuncArgString* pAnim = static_cast< CScriptFuncArgString* >( pState->GetArg( 1 ) );
	string sAnimationFileName = pAnim->m_sValue + ".bke";
	IEntity* pEntity = dynamic_cast< IEntity* >( m_pEntityManager->GetEntity( pID->m_nValue ) );
	if( pEntity )
	{
		try
		{
			if( !pEntity->HasAnimation( sAnimationFileName ) )
				pEntity->AddAnimation( sAnimationFileName );
			pEntity->SetCurrentAnimation( sAnimationFileName );
		}
		catch( CFileNotFoundException& e )
		{
			string sMessage = string( "fichier \"" ) + e.m_sFileName + "\" introuvable";
			m_pConsole->Println( sMessage );
		}
		catch( CEException&e )
		{
			string sMessage;
			e.GetErrorMessage( sMessage );
			m_pConsole->Println( sMessage );
		}
	}
	else
		m_pConsole->Println( "Erreur : L'identifiant entré ne correspond pas à celui d'une entité animable" );
}

void PlayCurrentAnimation( IScriptState* pState )
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgInt* pLoop = static_cast< CScriptFuncArgInt* >(pState->GetArg(1));
	IEntity* pEntity = m_pEntityManager->GetEntity( pID->m_nValue );
	if( pEntity )
	{
		IAnimation* pAnimation = pEntity->GetCurrentAnimation();
		if( pAnimation )
			pAnimation->Play(pLoop->m_nValue != 0);
		else
			m_pConsole->Println( "Errreur : L'entité sélectionnée est animable mais ne contient pas l'animation demandée." );
	}
	else
		m_pConsole->Println( "Erreur : Identifiant incorrect" );
}
//
//void ClearRessources( IScriptState* pState )
//{
//	m_pSceneManager->ClearScene( m_pScene );
//	m_pRessourceManager->DestroyAllRessources();
//}

void LoadShader( IScriptState* pState )
{
	CScriptFuncArgString* pShaderName = static_cast< CScriptFuncArgString* >( pState->GetArg( 0 ) );
	m_pRenderer->LoadShader( pShaderName->m_sValue );
}

void LoadTopicFile(IScriptState* pState)
{
	m_pGUIManager->GetTopicsWindow()->LoadTopics("Topics.json");
}

void DisplayShaderName( IScriptState* pState )
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pID->m_nValue );
	string sShaderName;
	pEntity->GetRessource()->GetShader()->GetName( sShaderName );
	m_pConsole->Println( sShaderName );
}

void SetHMPrecision( IScriptState* pState )
{
	CScriptFuncArgInt* pPrecision = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	m_pCollisionManager->SetHeightMapPrecision( pPrecision->m_nValue );
}

void StopDisplayHM( IScriptState* pState )
{
	m_pCollisionManager->StopDisplayHeightMap();
}

void DisplayHM( IScriptState* pState )
{
	CScriptFuncArgString* pString = static_cast< CScriptFuncArgString* >( pState->GetArg( 0 ) );
	string sFileName = pString->m_sValue;
	if( sFileName.find( ".bme" ) == -1 )
		sFileName += ".bme";
	IMesh* pMesh = dynamic_cast< IMesh* >( m_pRessourceManager->GetRessource( sFileName) );
	if( pMesh )
		m_pCollisionManager->DisplayHeightMap( pMesh );
	else
		m_pConsole->Println("Erreur : ressource non valide");
}

void DisplaySceneChilds( IScriptState* pState )
{
	string sName;
	for( unsigned int i = 0; i < m_pScene->GetChildCount(); i++ )
	{
		m_pScene->GetChild( i )->GetName( sName );
		m_pConsole->Println( sName );
	}
}

string g_sBegin;
void DisplayFonctionList(void* params)
{
	int lineCount = (m_pConsole->GetClientHeight() / m_pConsole->GetLineHeight()) - 4;
	vector<string>::iterator it = g_vStringsResumeMode.begin();
	int index = 0;
	while(it != g_vStringsResumeMode.end())
	{
		string sFuncName = *it;
		string sFuncNameLow = sFuncName;
		it = g_vStringsResumeMode.erase(it);
		transform(sFuncName.begin(), sFuncName.end(), sFuncNameLow.begin(), tolower);
		if (sFuncNameLow.find(g_sBegin) != -1) {
			m_pConsole->Println(sFuncName);
			index++;
			if (index > lineCount) {
				break;
			}
		}
	}
	if(g_vStringsResumeMode.size() > 0)
		m_pConsole->SetPauseModeOn(DisplayFonctionList, nullptr);
}

void flist( IScriptState* pState )
{
	CScriptFuncArgString* pString = static_cast< CScriptFuncArgString* >( pState->GetArg( 0 ) );
	m_pScriptManager->GetRegisteredFunctions(g_vStringsResumeMode);
	g_sBegin = pString->m_sValue;
	transform( pString->m_sValue.begin(), pString->m_sValue.end(), g_sBegin.begin(), tolower );
	DisplayFonctionList(nullptr);
}

void DisplayLightIntensity( IScriptState* pState )
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	IEntity* pLightEntity = m_pEntityManager->GetEntity( pID->m_nValue );
	float fIntensity = m_pRessourceManager->GetLightIntensity( pLightEntity->GetRessource() );
	ostringstream oss;
	oss << fIntensity;
	m_pConsole->Println( oss.str() );
}

void RunScript( string sFileName )
{
	FILE* pFile = m_pFileSystem->OpenFile( sFileName, "r" );
	if( pFile )
	{
		fseek( pFile, 0, SEEK_END );
		long size = ftell( pFile );
		fseek( pFile, 0, SEEK_SET );
		string script;
		script.resize( size );
		fread( &script[ 0 ], sizeof( char ), size, pFile );
		try
		{
			m_pScriptManager->ExecuteCommand( script );
		}
		catch( CCompilationErrorException& e )
		{
			string sMessage;
			e.GetErrorMessage( sMessage );
			m_pConsole->Println( sMessage );
		}
		fclose( pFile );
	}
	else
		m_pConsole->Println( "Fichier introuvable" );
}

void run( IScriptState* pState )
{
	CScriptFuncArgString* pName = static_cast< CScriptFuncArgString* >( pState->GetArg( 0 ) );
	string sFileName = pName->m_sValue;
	if( sFileName.find( ".eas" ) == -1 )
		sFileName += ".eas";
	RunScript( sFileName );
}

void LoadImage( IScriptState* pState )
{
	CScriptFuncArgString* pName = static_cast< CScriptFuncArgString* >( pState->GetArg( 0 ) );
	ILoader::CTextureInfos ti;
	ti.m_bFlip = true;
	try
	{
		m_pLoaderManager->Load( pName->m_sValue, ti );
		m_pConsole->Println("Fichier chargé.");
	}
	catch( ILoaderManager::CBadExtension& )
	{
		m_pConsole->Println("Erreur : extension de fichier non gérée.");
	}
}

void DisplayRepere( IScriptState* pState )
{
	CScriptFuncArgInt* pDisplay = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	if( pDisplay->m_nValue == 1 && m_pRepere->GetParent() != m_pScene )
		m_pRepere->Link( m_pScene );
	else if( pDisplay->m_nValue == 0 )
		m_pRepere->Unlink();
}

DWORD WINAPI ExportBMEToAsciiCallback(void* lpThreadParameter)
{
	vector<string>* callbackArg = (vector<string>*)lpThreadParameter;
	string sBMEName = callbackArg->at(0);
	string sOutputName = callbackArg->at(1);
	delete callbackArg;
	ILoader::CAnimatableMeshData oData;
	ILoader* pLoader = m_pLoaderManager->GetLoader("bme");
	try {
		pLoader->Load(sBMEName, oData, *m_pFileSystem);	
		string rootDirectory;
		m_pFileSystem->GetLastDirectory(rootDirectory);
		for (int i = 0; i < oData.m_vMessages.size(); i++) {
			m_pConsole->Println(oData.m_vMessages[i]);
		}

		m_pConsole->Println("Export en cours...");
	
		oData.m_vMessages.clear();
		pLoader->SetAsciiExportPrecision(7);

		string outputPath = rootDirectory + "\\" + sOutputName;
		pLoader->Export(outputPath, oData);
		m_pConsole->Println("Export terminé");
	}
	catch (CFileNotFoundException& e)
	{
		ostringstream log;
		log << "Erreur : Fichier \"" << e.m_sFileName << "\" manquant";
		m_pConsole->Println(log.str());
	}
	catch (CEException& e)
	{
		m_pConsole->Println(e.what());
	}
	catch (exception& e) {
		char msg[256];
		sprintf(msg, "Error while accessing \"%s\"", e.what());
		m_pConsole->Println(msg);
	}
	m_pConsole->EnableInput(true);
	return 0;
}

void Reset(IScriptState* pState)
{
	m_pRessourceManager->Reset();
	m_pEntityManager->Clear();
}

void ExportBMEToAscii( IScriptState* pState )
{
	CScriptFuncArgString* pFileName = static_cast< CScriptFuncArgString* >( pState->GetArg( 0 ) );
	CScriptFuncArgString* pOutFileName = static_cast< CScriptFuncArgString* >(pState->GetArg(1));
	string sBMEName = pFileName->m_sValue;
	int nExtPos = (int)sBMEName.find( ".bme" );
	string sFileNameWithoutExt;
	if( nExtPos == -1 )
	{
		sFileNameWithoutExt = sBMEName;
		sBMEName += ".bme";		
	}
	else
		sFileNameWithoutExt = sBMEName.substr(0, nExtPos );
	string sOutputName = pOutFileName ? pOutFileName->m_sValue : sFileNameWithoutExt + ".txt";
	

	vector<string>* callbackArg = new vector<string>;
	callbackArg->push_back(sBMEName);
	callbackArg->push_back(sOutputName);
	DWORD threadID;
	CreateThread(NULL, 0, ExportBMEToAsciiCallback, callbackArg, 0, &threadID);

	m_pConsole->EnableInput(false);
}

void ExportBKEToAscii( IScriptState* pState )
{
	CScriptFuncArgString* pFileName = static_cast< CScriptFuncArgString* >( pState->GetArg( 0 ) );
	CScriptFuncArgInt* pPrecision = static_cast< CScriptFuncArgInt* >( pState->GetArg( 1 ) );
	string sBKEName = pFileName->m_sValue;
	int nExtPos = (int)sBKEName.find( ".bke" );
	string sFileNameWithoutExt;
	if( nExtPos == -1 )
	{
		sFileNameWithoutExt = sBKEName;
		sBKEName += ".bke";
	}
	else
		sFileNameWithoutExt = sBKEName.substr( nExtPos );
	string sTXTName = sFileNameWithoutExt + ".txt";
	ILoader::CAnimationInfos oData;
	ILoader* pLoader = m_pLoaderManager->GetLoader( "bke" );
	pLoader->Load( sBKEName, oData, *m_pFileSystem );
	pLoader->SetAsciiExportPrecision( pPrecision->m_nValue );
	pLoader->Export( sTXTName, oData );
	string sMessage = string( "Fichier exporté dans \"" ) + sTXTName + "\"";
	m_pConsole->Println( sMessage );
}

void ExportBSEToAscii(IScriptState* pState)
{
	CScriptFuncArgString* pFileName = static_cast< CScriptFuncArgString* >(pState->GetArg(0));
	string sBSEName = pFileName->m_sValue;
	int nExtPos = (int)sBSEName.find(".bse");
	string sFileNameWithoutExt;
	if (nExtPos == -1)
	{
		sFileNameWithoutExt = sBSEName;
		sBSEName += ".bse";
	}
	else
		sFileNameWithoutExt = sBSEName.substr(0, nExtPos);

	string root;
	m_pFileSystem->GetLastDirectory(root);
	string sTXTName = root + "/" + sFileNameWithoutExt + ".txt";
	sBSEName = root + "/" + sBSEName;

	ILoader::CSceneInfos oData;
	ILoader* pLoader = m_pLoaderManager->GetLoader("bse");
	pLoader->Load(sBSEName, oData, *m_pFileSystem);
	//pLoader->SetAsciiExportPrecision(pPrecision->m_nValue);
	pLoader->Export(sTXTName, oData);
	string sMessage = string("Fichier exporté dans \"") + sTXTName + "\"";
	m_pConsole->Println(sMessage);
}

void ClearScene( IScriptState* pState )
{
	ICamera* pLinkedCamera = m_pCameraManager->GetCameraFromType( ICameraManager::TLinked );
	if (pLinkedCamera) {
		if (pLinkedCamera->GetParent())
			pLinkedCamera->Unlink();
		CMatrix oLinkedMatrix;
		pLinkedCamera->SetLocalMatrix(oLinkedMatrix);
	}

	m_pScene->Clear();
	ICamera* pCamera = m_pCameraManager->GetCameraFromType( ICameraManager::TFree );
	m_pCameraManager->SetActiveCamera( pCamera );
	m_pEntityManager->AddEntity( pCamera, "FreeCamera" );
	if (pLinkedCamera)
		m_pEntityManager->AddEntity( pLinkedCamera, "LinkedCamera" );
	m_pRepere = m_pEntityManager->CreateRepere( *m_pRenderer );
	m_pRepere->Link( m_pScene );
	m_pEntityManager->AddEntity( m_pScene, "SceneGame", 0 );
}

void SetSceneMap( IScriptState* pState )
{
	CScriptFuncArgString* pRessourceFileName = static_cast< CScriptFuncArgString* >( pState->GetArg( 0 ) );
	CScriptFuncArgString* pDiffuseFileName = static_cast< CScriptFuncArgString* >(pState->GetArg(1));
	CScriptFuncArgInt* pLength = static_cast< CScriptFuncArgInt* >(pState->GetArg(2));
	CScriptFuncArgFloat* pHeight = static_cast< CScriptFuncArgFloat* >(pState->GetArg(3));

	bool bak = m_pRessourceManager->IsCatchingExceptionEnabled();
	m_pRessourceManager->EnableCatchingException( false );
	try
	{
		m_pMapEditor->SetSceneMap(pRessourceFileName->m_sValue, pDiffuseFileName->m_sValue, pLength->m_nValue, pHeight->m_fValue);
	}
	catch( CRessourceException& e )
	{
		string s;
		e.GetErrorMessage( s );
		m_pConsole->Println( s );
	}
	catch( CBadFileFormat& )
	{
		m_pConsole->Println( "Mauvais format de fichier, essayez de le réexporter" );
	}
	catch( CFileNotFoundException& e )
	{
		string sFileName;
		e.GetErrorMessage( sFileName );
		string sMessage = string("Erreur : fichier \"") + sFileName + "\" manquant";
		m_pConsole->Println( sMessage );
	}
	catch( CEException&  )
	{
		string sMessage = string( "\"" ) + pRessourceFileName->m_sValue + "\" introuvable";
		m_pConsole->Println( sMessage );
	}
	m_pRessourceManager->EnableCatchingException( bak );
}

void DrawCollisionModels(IScriptState* pState)
{
	CScriptFuncArgInt* pCharacterID = dynamic_cast<CScriptFuncArgInt*>(pState->GetArg(0));
	CScriptFuncArgInt* pDisplay = dynamic_cast<CScriptFuncArgInt*>(pState->GetArg(1));
	IEntity* pEntity = m_pEntityManager->GetEntity(pCharacterID->m_nValue);
	
	if (pEntity) {
		pEntity->DrawCollisionBoundingBoxes(pDisplay->m_nValue == 0 ? false : true);
	}
}

void EnableInstancingMode(IScriptState* pState)
{
	CScriptFuncArgInt* pEnable = dynamic_cast<CScriptFuncArgInt*>(pState->GetArg(0));
	m_pEntityManager->EnableInstancing(pEnable->m_nValue == 0 ? false : true);
}

void SetTexture(IScriptState* pState)
{
	CScriptFuncArgString* pTextureName = dynamic_cast<CScriptFuncArgString*>(pState->GetArg(0));
	m_pCharacterEditor->SetTexture(pTextureName->m_sValue);
}

void SetTextureInWorld(IScriptState* pState)
{
	CScriptFuncArgInt* pCharacterID = dynamic_cast<CScriptFuncArgInt*>(pState->GetArg(0));
	CScriptFuncArgString* pTextureName = dynamic_cast<CScriptFuncArgString*>(pState->GetArg(1));
	ICharacter* pCharacter = dynamic_cast<ICharacter*>(m_pEntityManager->GetEntity(pCharacterID->m_nValue));
	if (pCharacter) {
		pCharacter->SetDiffuseTexture(pTextureName->m_sValue);
	}
	else {
		ostringstream oss;
		oss << "Erreur, l'entite " << pCharacterID << " n'est pas un Character";
		m_pConsole->Println(oss.str());
	}
	
}

void SetEntityWeight( IScriptState* pState )
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgFloat* pWeight = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 1 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pID->m_nValue );
	if( pEntity )
		pEntity->SetWeight( pWeight->m_fValue );
	else
	{
		ostringstream oss;
		oss << "Erreur : Entité " << pID->m_nValue << " introuvable";
		m_pConsole->Println( oss.str() );
	}
}

void LoadHM( IScriptState* pState )
{
	CScriptFuncArgString* pFileName = static_cast< CScriptFuncArgString* >( pState->GetArg( 0 ) );
	vector< vector< unsigned char > > vPixels;
	try
	{
		m_pCollisionManager->LoadHeightMap( pFileName->m_sValue, vPixels );
		m_pConsole->Println( "Height map correctement chargée" );
	}
	catch ( ILoaderManager::CBadExtension&  )
	{
		m_pConsole->Println( "Erreur -> fichier non trouvé" );
	}
	int test = 0;
}

void GetFreeFileName( string sPrefixName, string sExtension, string& sFileName )
{
	ostringstream ossFile;
	int i = 0;
	FILE* pFile = NULL;
	do
	{
		ossFile << sPrefixName << i << "." << sExtension;
		fopen_s( &pFile, ossFile.str().c_str(), "r" );
		if( pFile )
		{
			fclose( pFile );
			i++;
			ossFile.str( "" );
		}
	}
	while( pFile );
	sFileName = ossFile.str();
}


bool g_bHMHackEnabled = false;

void EnableHMHack(IScriptState* pState)
{
	CScriptFuncArgInt* pEnable = static_cast< CScriptFuncArgInt* >(pState->GetArg(0));
	g_bHMHackEnabled = (pEnable->m_nValue == 1);

	m_pCollisionManager->EnableHMHack(false);
	m_pCollisionManager->EnableHMHack2(false);
	

	ostringstream oss;
	oss << "Height map hack " << g_bHMHackEnabled ? " enabled" : "disabled";
	m_pConsole->Println(oss.str());
}

void CreateHM( IScriptState* pState )
{
	CScriptFuncArgString* pString = static_cast< CScriptFuncArgString* >( pState->GetArg( 0 ) );
	string sFileName = pString->m_sValue;
	if( sFileName.find( ".bme" ) == -1 )
		sFileName += ".bme";
	IEntity* pEntity = NULL;
	try
	{
		pEntity = m_pEntityManager->CreateEntity(sFileName, "");
	}
	catch( CEException& e )
	{
		string sError;
		e.GetErrorMessage( sError );
		string s = string( "Erreur : " ) + sError;
		m_pConsole->Println( s );
	}
	if( pEntity )
	{
		IMesh* pMesh = dynamic_cast< IMesh* >( pEntity->GetRessource() );
		if( pMesh )
		{
			IMesh* pGround = static_cast<IMesh*>(m_pScene->GetRessource());
			string sSceneFileName;
			if(pGround)
				pGround->GetFileName(sSceneFileName);
			if (g_bHMHackEnabled) {
				if (sSceneFileName == sFileName) {
					m_pCollisionManager->EnableHMHack(true);
					m_pCollisionManager->EnableHMHack2(false);
				}
				else {
					m_pCollisionManager->EnableHMHack(false);
					m_pCollisionManager->EnableHMHack2(true);
				}
			}
			ILoader::CTextureInfos ti;
			m_pCollisionManager->CreateHeightMap( pMesh, ti, IRenderer::T_BGR );
			ti.m_ePixelFormat = ILoader::eBGR;
			string sTextureFileName = string( "HM_" ) + pString->m_sValue + ".bmp";
			m_pLoaderManager->Export( sTextureFileName, ti );
		}
	}
	else
		m_pConsole->Println( "Une erreur s'est produite lors de la création de l'entité" );
}

void CreateHMFromFile(IScriptState* pState)
{
	CScriptFuncArgString* pString = static_cast<CScriptFuncArgString*>(pState->GetArg(0));
	string sFileName = pString->m_sValue;
	if (sFileName.find(".bme") == -1)
		sFileName += ".bme";
	IEntity* pEntity = NULL;
	try
	{
		m_pCollisionManager->CreateHeightMap(sFileName);
	}
	catch (CEException& e)
	{
		string sError;
		e.GetErrorMessage(sError);
		string s = string("Erreur : ") + sError;
		m_pConsole->Println(s);
	}
}

void ScreenCapture( IScriptState* pState )
{
	ostringstream ossFile;
	int i = 0;
	FILE* pFile = NULL;
	do
	{
		ossFile << "Capture_" << i << ".bmp";
		fopen_s( &pFile, ossFile.str().c_str(), "r" );
		if( pFile )
		{
			fclose( pFile );
			i++;
			ossFile.str( "" );
		}
	}
	while( pFile );
	CRenderUtils::ScreenCapture( ossFile.str(), m_pRenderer, m_pLoaderManager, m_pSceneManager->GetScene( "Game" ) );
}

void SetLightIntensity( IScriptState* pState )
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgFloat* pIntensity = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 1 ) );

	ILightEntity* pLightEntity = dynamic_cast<ILightEntity*>(m_pEntityManager->GetEntity(pID->m_nValue));
	if (pLightEntity) {
		ILight* pLight = dynamic_cast<ILight*>(pLightEntity->GetRessource());
		if(pLight)
			pLight->SetIntensity(pIntensity->m_fValue);
	}
	else {
		ostringstream oss;
		oss << "Erreur : " << pID->m_nValue << " n'est pas un identifiant de lumière";
		m_pConsole->Println(oss.str());
	}
}

void SetLightAmbient(IScriptState* pState)
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >(pState->GetArg(0));
	CScriptFuncArgFloat* pAmbient = static_cast< CScriptFuncArgFloat* >(pState->GetArg(1));	
	ILightEntity* pLightEntity = dynamic_cast<ILightEntity*>(m_pEntityManager->GetEntity(pID->m_nValue));
	if (pLightEntity) {
		ILight* pLight = dynamic_cast<ILight*>(pLightEntity->GetRessource());
		if(pLight)
			pLight->SetAmbient(pAmbient->m_fValue);
	}
	else {
		ostringstream oss;
		oss << "Erreur : " << pID->m_nValue << " n'est pas un identifiant de lumière";
		m_pConsole->Println(oss.str());
	}

}

void CreateLight(IScriptState* pState)
{
	CScriptFuncArgInt* pr = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgInt* pg = static_cast< CScriptFuncArgInt* >( pState->GetArg( 1 ) );
	CScriptFuncArgInt* pb = static_cast< CScriptFuncArgInt* >( pState->GetArg( 2 ) );
	CScriptFuncArgString* pType = static_cast< CScriptFuncArgString* >( pState->GetArg( 3 ) );
	CScriptFuncArgFloat* pIntensity = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 4 ) );
	CVector Color( (float)pr->m_nValue / 255.f, (float)pg->m_nValue / 255.f, (float)pb->m_nValue / 255.f, 1.f );
	string sType = pType->m_sValue;
	transform( pType->m_sValue.begin(), pType->m_sValue.end(), sType.begin(), tolower );
	IRessource::TLight type;
	if( sType == "omni" )
		type = IRessource::OMNI;
	else if( sType == "dir" )
		type = IRessource::DIRECTIONAL;
	else if( sType == "spot" )
		type = IRessource::SPOT;
	else
	{
		m_pConsole->Println( "Paramètre 4 invalide, vous devez entrer un type de lumière parmis les 3 suivants : \"omni\" , \"dir\" , \"spot\" " );
		return;
	}
	IEntity* pEntity = m_pEntityManager->CreateLightEntity( Color, type, pIntensity->m_fValue );
	pEntity->Link( m_pScene );
	ostringstream oss;
	oss << "La lumière a été créée avec l'identifiant " << m_pEntityManager->GetEntityID( pEntity );;
	m_pConsole->Println( oss.str() );
	pState->SetReturnValue(m_pEntityManager->GetEntityID(pEntity));
	
}

void CreateLightw( IScriptState* pState )
{
	CScriptFuncArgString* pType = static_cast< CScriptFuncArgString* >( pState->GetArg( 0 ) );
	CScriptFuncArgFloat* pIntensity = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 1 ) );
	CVector Color( 1.f, 1.f, 1.f, 1.f );
	string sType = pType->m_sValue;
	transform( pType->m_sValue.begin(), pType->m_sValue.end(), sType.begin(), tolower );
	IRessource::TLight type;
	if( sType == "omni" )
		type = IRessource::OMNI;
	else if( sType == "dir" )
		type = IRessource::DIRECTIONAL;
	else if( sType == "spot" )
		type = IRessource::SPOT;
	else
	{
		m_pConsole->Println( "Paramètre 1 invalide, vous devez entrer un type de lumière parmis les 3 suivants : \"omni\" , \"dir\" , \"spot\" " );
		return;
	}
	IEntity* pEntity = m_pEntityManager->CreateLightEntity( Color, type, pIntensity->m_fValue );
	pEntity->Link( m_pScene );
	ostringstream oss;
	oss << "La lumière a été créée avec l'identifiant " << m_pEntityManager->GetEntityID( pEntity );
	m_pConsole->Println( oss.str() );
	pState->SetReturnValue(m_pEntityManager->GetEntityID(pEntity));
}

void CreateCollisionMap(IScriptState* pState)
{
	IEntity* pNode = nullptr;
	CScriptFuncArgInt* pId = static_cast<CScriptFuncArgInt*>(pState->GetArg(0));	
	CScriptFuncArgInt* pCellSize = static_cast<CScriptFuncArgInt*>(pState->GetArg(1));
	CScriptFuncArgFloat* pBias = static_cast<CScriptFuncArgFloat*>(pState->GetArg(2));
	if (pId->m_nValue == 0)
		pNode = m_pScene;
	else
		pNode = dynamic_cast<IEntity*>(m_pEntityManager->GetEntity(pId->m_nValue));
	vector<vector<bool>> vGrid;
	ICollisionMap* pCollisionMap = m_pCollisionManager->CreateCollisionMap(pNode, pBias->m_fValue);
	pCollisionMap->Generate(pCellSize->m_nValue);
}

void EnablePathFindSaving(IScriptState* pState)
{
	CScriptFuncArgInt* pEnable = static_cast<CScriptFuncArgInt*>(pState->GetArg(0));
	CScriptFuncArgInt* pXmin = static_cast<CScriptFuncArgInt*>(pState->GetArg(1));
	CScriptFuncArgInt* pYmin = static_cast<CScriptFuncArgInt*>(pState->GetArg(2));
	CScriptFuncArgInt* pXmax = static_cast<CScriptFuncArgInt*>(pState->GetArg(3));
	CScriptFuncArgInt* pYMax = static_cast<CScriptFuncArgInt*>(pState->GetArg(4));
	m_pPathFinder->EnableSaveGrid(pEnable->m_nValue != 0, pXmin->m_nValue, pYmin->m_nValue, pXmax->m_nValue, pYMax->m_nValue);
}

void IsAbsolutePath(IScriptState* pState)
{
	CScriptFuncArgString* pPath = static_cast<CScriptFuncArgString*>(pState->GetArg(0));
	pState->SetReturnValue(CEasyFile::IsAbsolutePath(pPath->m_sValue) ? 1 : 0);
}

void TestRegExpr(IScriptState* pState)
{
	CScriptFuncArgString* pString= static_cast<CScriptFuncArgString*>(pState->GetArg(0));
	CScriptFuncArgString* pRegExpr = static_cast<CScriptFuncArgString*>(pState->GetArg(1));

	regex reg(pRegExpr->m_sValue);
	sregex_iterator itBegin = sregex_iterator(pString->m_sValue.begin(), pString->m_sValue.end(), reg);
	sregex_iterator itEnd = sregex_iterator();
	pState->SetReturnValue(std::distance(itBegin, itEnd) > 0);
}

void RollEntity( IScriptState* pState )
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgFloat* pRoll = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 1 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pID->m_nValue );
	if (!pEntity) {
		ostringstream oss;
		oss << "RollEntity() : Erreur lors du chargement de l'entité" << pID->m_nValue;
		m_pConsole->Println(oss.str());
	}
	else
		pEntity->Roll( pRoll->m_fValue );
}

void PitchEntity( IScriptState* pState )
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgFloat* pPitch = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 1 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pID->m_nValue );
	if( pEntity )
		pEntity->Pitch( pPitch->m_fValue );
	else
		m_pConsole->Println("Identifiant invalide");
}

void YawEntity( IScriptState* pState )
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgFloat* pYaw = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 1 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pID->m_nValue );
	if(pEntity)
		pEntity->Yaw( pYaw->m_fValue );
	else {
		ostringstream oss;
		oss << "Erreur : Entite " << pID->m_nValue << " introuvable";
		m_pConsole->Println(oss.str());
	}
}

void SetEntityShader( IScriptState* pState )
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgString* pShaderName = static_cast< CScriptFuncArgString* >( pState->GetArg( 1 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pID->m_nValue );
	IShader* pShader = m_pRenderer->GetShader( pShaderName->m_sValue );
	pEntity->SetShader( pShader );
}

void SetEntitySpecular(IScriptState* pState)
{
	CScriptFuncArgInt* pID = (CScriptFuncArgInt*)pState->GetArg(0);
	CScriptFuncArgFloat* pr = (CScriptFuncArgFloat*)pState->GetArg(1);
	CScriptFuncArgFloat* pg = (CScriptFuncArgFloat*)pState->GetArg(2);
	CScriptFuncArgFloat* pb = (CScriptFuncArgFloat*)pState->GetArg(3);
	IEntity* pEntity = m_pEntityManager->GetEntity(pID->m_nValue);
	pEntity->SetCustomSpecular(CVector(pr->m_fValue, pg->m_fValue, pb->m_fValue));
}

void SetSpecular(IScriptState* pState)
{
	CScriptFuncArgFloat* pr = (CScriptFuncArgFloat*)pState->GetArg(0);
	CScriptFuncArgFloat* pg = (CScriptFuncArgFloat*)pState->GetArg(1);
	CScriptFuncArgFloat* pb = (CScriptFuncArgFloat*)pState->GetArg(2);
	if (m_pCharacterEditor->IsEnabled())
		m_pCharacterEditor->SetSpecular(pr->m_fValue, pg->m_fValue, pb->m_fValue);
}

void SetShininess(IScriptState* pState)
{
	CScriptFuncArgFloat* pShininess = (CScriptFuncArgFloat*)pState->GetArg(0);
	m_pCharacterEditor->SetShininess(pShininess->m_fValue);
}

void SetEntityShininess(IScriptState* pState)
{
	CScriptFuncArgInt* pID = (CScriptFuncArgInt*)pState->GetArg(0);
	CScriptFuncArgFloat* pShininess = (CScriptFuncArgFloat*)pState->GetArg(1);
	IEntity* pEntity = m_pEntityManager->GetEntity(pID->m_nValue);
	if (pEntity) {
		IMesh* pMesh = dynamic_cast<IMesh*>(pEntity->GetRessource());
		if (pMesh) {
			for (int i = 0; i < pMesh->GetMaterialCount(); i++) {
				pMesh->GetMaterial(i)->SetShininess(pShininess->m_fValue);
			}
		}
	}
	else {
		m_pConsole->Println(string("Error : Entity '") + std::to_string(pID->m_nValue) + "' not found");
	}
}


void ColorizeEntity(IScriptState* pState)
{
	CScriptFuncArgInt* pID = (CScriptFuncArgInt*)pState->GetArg(0);
	CScriptFuncArgFloat* pr = (CScriptFuncArgFloat*)pState->GetArg(1);
	CScriptFuncArgFloat* pg = (CScriptFuncArgFloat*)pState->GetArg(2);
	CScriptFuncArgFloat* pb = (CScriptFuncArgFloat*)pState->GetArg(3);
	IEntity* pEntity = m_pEntityManager->GetEntity(pID->m_nValue);
	if (pEntity) {
		IMesh* pMesh = dynamic_cast<IMesh*>(pEntity->GetRessource());
		if (pMesh)
			pMesh->Colorize(pr->m_fValue, pg->m_fValue, pb->m_fValue, 0.f);
	}
}

void GetNodeInfos( INode* pNode, int nLevel = 0 )
{
	IEntity* pEntity = dynamic_cast< IEntity* >( pNode );
	if( pEntity ) {

		ostringstream sLine;		
		for( int j = 0; j < nLevel; j++ )
			sLine << "\t";
		string sEntityName;
		pEntity->GetEntityName(sEntityName);
		if (sEntityName.empty())
			pEntity->GetName(sEntityName);
		if (sEntityName.find("CollisionPrimitive") == -1) {
			sLine << "Entity name = " << sEntityName << ", ID = " << m_pEntityManager->GetEntityID(pEntity);
			g_vStringsResumeMode.push_back(sLine.str());
		}
	}
	for( unsigned int i = 0; i < pNode->GetChildCount(); i++ )
		GetNodeInfos( pNode->GetChild( i ), nLevel + 1 );
}

void GetCollisionNodeInfos(INode* pNode, int nLevel = 0)
{
	IEntity* pEntity = dynamic_cast< IEntity* >(pNode);
	if (pEntity) {
		ICollisionEntity* pCollisionEntity = dynamic_cast<ICollisionEntity*>(pEntity);
		if (pCollisionEntity) {
			ostringstream sLine;
			for (int j = 0; j < nLevel; j++)
				sLine << "\t";
			string sEntityName;
			pEntity->GetEntityName(sEntityName);
			if (sEntityName.empty())
				pEntity->GetName(sEntityName);
			sLine << "Entity name = " << sEntityName << ", ID = " << m_pEntityManager->GetEntityID(pEntity);
			g_vStringsResumeMode.push_back(sLine.str());
		}
	}
	if (pNode) {
		for (unsigned int i = 0; i < pNode->GetChildCount(); i++)
			GetCollisionNodeInfos(pNode->GetChild(i), nLevel + 1);
	}
}

void Kill(IScriptState* pState)
{
	CScriptFuncArgInt* pId = (CScriptFuncArgInt*)(pState->GetArg(0));
	m_pEntityManager->Kill(pId->m_nValue);
}

void SetNPCState(IScriptState* pState)
{
	CScriptFuncArgInt* pNPCState = (CScriptFuncArgInt*)(pState->GetArg(0));
	pState = pState;
}

void WearArmorToDummy(IScriptState* pState)
{
 	CScriptFuncArgInt* pId = (CScriptFuncArgInt*)(pState->GetArg(0));
	CScriptFuncArgString* pArmor = (CScriptFuncArgString*)(pState->GetArg(1));
	m_pEntityManager->WearArmorToDummy(pId->m_nValue, pArmor->m_sValue);
}

void WearShoes(IScriptState* pState)
{
	CScriptFuncArgString* pShoes = (CScriptFuncArgString*)(pState->GetArg(0));
	m_pCharacterEditor->WearShoes(pShoes->m_sValue);
}

void UnWearShoes(IScriptState* pState)
{
	CScriptFuncArgString* pShoes = (CScriptFuncArgString*)(pState->GetArg(0));
	m_pCharacterEditor->UnWearShoes(pShoes->m_sValue);
}

void UnWearAllShoes(IScriptState* pState)
{
	m_pCharacterEditor->UnWearAllShoes();
}

void WearCloth(IScriptState* pState)
{
	CScriptFuncArgString* pClothPath = (CScriptFuncArgString*)(pState->GetArg(0));
	CScriptFuncArgString* pDummyName = (CScriptFuncArgString*)(pState->GetArg(1));
	m_pCharacterEditor->WearCloth(pClothPath->m_sValue, pDummyName->m_sValue);
}

void WearCharacterItem(IScriptState* pState)
{
	CScriptFuncArgString* pCharacterID = (CScriptFuncArgString*)(pState->GetArg(0));
	CScriptFuncArgString* pItemID = (CScriptFuncArgString*)(pState->GetArg(1));
	ICharacter* pCharacter = dynamic_cast<ICharacter*>(m_pEntityManager->GetEntity(pCharacterID->m_sValue));
	if (pCharacter) {
		pCharacter->WearItem(pItemID->m_sValue);
	}
	else {
		throw CEException(string("Error in WearCharacterItem() : character '") + pCharacterID->m_sValue + "' does not exists");
	}
}

void UnwearAllClothes(IScriptState* pState)
{
	CScriptFuncArgInt* pEntityId = (CScriptFuncArgInt*)(pState->GetArg(0));
	ICharacter* pCharacter = dynamic_cast<ICharacter*>(m_pEntityManager->GetEntity(pEntityId->m_nValue));
	pCharacter->UnwearAllClothes();
}

void AddItem(IScriptState* pState)
{
	CScriptFuncArgString* pItemName = (CScriptFuncArgString*)(pState->GetArg(0));
	m_pCharacterEditor->AddItem(pItemName->m_sValue);
}

void AddCharacterItem(IScriptState* pState)
{
	CScriptFuncArgString* pCharacterName = (CScriptFuncArgString*)(pState->GetArg(0));
	CScriptFuncArgString* pItemName = (CScriptFuncArgString*)(pState->GetArg(1));
	ICharacter* pCharacter = dynamic_cast<ICharacter*>(m_pEntityManager->GetEntity(pCharacterName->m_sValue));
	pCharacter->AddItem(pItemName->m_sValue);
}

void RemoveCharacterItem(IScriptState* pState)
{
	CScriptFuncArgString* pCharacterName = (CScriptFuncArgString*)(pState->GetArg(0));
	CScriptFuncArgString* pItemID = (CScriptFuncArgString*)(pState->GetArg(1));
	ICharacter* pCharacter = dynamic_cast<ICharacter*>(m_pEntityManager->GetEntity(pCharacterName->m_sValue));
	pCharacter->RemoveItem(pItemID->m_sValue);
}

void RemoveItem(IScriptState* pState)
{
	CScriptFuncArgString* pItemID = (CScriptFuncArgString*)(pState->GetArg(0));
	m_pCharacterEditor->RemoveItem(pItemID->m_sValue);
}

void GetItemCount(IScriptState* pState)
{
	CScriptFuncArgString* pCharacterName = (CScriptFuncArgString*)(pState->GetArg(0));
	CScriptFuncArgString* pItemID = (CScriptFuncArgString*)(pState->GetArg(1));
	ICharacter* pCharacter = dynamic_cast<ICharacter*>(m_pEntityManager->GetEntity(pCharacterName->m_sValue));
	pState->SetReturnValue(pCharacter->GetItemCount(pItemID->m_sValue));
}

void SetBody(IScriptState* pState)
{
	CScriptFuncArgString* pBody = (CScriptFuncArgString*)(pState->GetArg(0));
	m_pCharacterEditor->SetBody(pBody->m_sValue);
}

void DisplayPickingRaySelected(IScriptState* pState)
{
	CScriptFuncArgInt* pDisplay = (CScriptFuncArgInt*)(pState->GetArg(0));
	if(m_pMapEditor->IsEnabled())
		m_pMapEditor->EnableDisplayPickingRaySelected(pDisplay->m_nValue > 0);
	else if (m_pWorldEditor->IsEnabled()) {
		m_pWorldEditor->EnableDisplayPickingRayMouseMove(pDisplay->m_nValue > 0);
	}
}

void DisplayPickingRayMouseMove(IScriptState* pState)
{
	CScriptFuncArgInt* pDisplay = (CScriptFuncArgInt*)(pState->GetArg(0));
	if (m_pMapEditor->IsEnabled())
		m_pMapEditor->EnableDisplayPickingRayMouseMove(pDisplay->m_nValue > 0);
	else if (m_pWorldEditor->IsEnabled()) {
		m_pWorldEditor->EnableDisplayPickingRayMouseMove(pDisplay->m_nValue > 0);
	}
}

void DisplayPickingIntersectPlane(IScriptState* pState)
{
	CScriptFuncArgInt* pDisplay = (CScriptFuncArgInt*)(pState->GetArg(0));
	if (m_pMapEditor->IsEnabled())
		m_pMapEditor->EnableDisplayPickingIntersectPlane(pDisplay->m_nValue > 0);
	else if (m_pWorldEditor->IsEnabled()) {
		m_pWorldEditor->EnableDisplayPickingIntersectPlane(pDisplay->m_nValue > 0);
	}
}

void DisplayCharacters(IScriptState* pState)
{
	vector<string> vCharactersName;
	m_pEntityManager->GetCharactersName(vCharactersName);
	for (int i = 0; i < vCharactersName.size(); i++) {
		m_pConsole->Println(vCharactersName[i]);
	}

}

void DisplayEntitiesResume(void* params)
{
	int maxLineCount = m_pConsole->GetClientHeight() / m_pConsole->GetLineHeight() - 3;
	vector<string>::iterator it = g_vStringsResumeMode.begin();
	int index = 0;
	while (it != g_vStringsResumeMode.end()) {
		m_pConsole->Println(*it);
		it = g_vStringsResumeMode.erase(it);
		if (index++ >= maxLineCount)
			break;
	}
	if (g_vStringsResumeMode.size() > 0)
		m_pConsole->SetPauseModeOn(DisplayEntitiesResume, nullptr);
}

void DisplayEntities( IScriptState* pState )
{
	g_vStringsResumeMode.clear();
	GetNodeInfos( m_pScene );
	DisplayEntitiesResume(nullptr);
}

void DisplayCollisionEntities(IScriptState* pState)
{
	CScriptFuncArgInt* pParentID = static_cast< CScriptFuncArgInt* >(pState->GetArg(0));
	IEntity* pParent = m_pEntityManager->GetEntity(pParentID->m_nValue);
	g_vStringsResumeMode.clear();
	GetCollisionNodeInfos(pParent);
	DisplayEntitiesResume(nullptr);
}

void DisplayMobileEntities(IScriptState* pState)
{
	string sText;
	m_pEntityManager->SerializeMobileEntities(m_pScene, sText);
	m_pConsole->Println(sText);
}

void GetEntityID( IScriptState* pState )
{
	ostringstream oss;
	CScriptFuncArgString* pName = static_cast< CScriptFuncArgString* >(  pState->GetArg( 0 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pName->m_sValue );
	if( pEntity )
	{
		int nID = m_pEntityManager->GetEntityID( pEntity );
		pState->SetReturnValue(nID);
	}
	else {
		pState->SetReturnValue(-1);
		m_pConsole->Println("Error : Entity '" + pName->m_sValue + "' doesn't exists");
	}
}

void GetCharacterID(IScriptState* pState)
{
	CScriptFuncArgString* pName = static_cast<CScriptFuncArgString*>(pState->GetArg(0));
	vector<IEntity*> characters; 
	m_pScene->GetCharactersInfos(characters);
	for (IEntity* entity : characters)
	{
		string name;
		entity->GetEntityName(name);
		if (name == pName->m_sValue) {
			pState->SetReturnValue(entity->GetID());
			return;
		}
	}
	m_pConsole->Println(string("Error : character '" + pName->m_sValue + "' not found"));
}

void AttachScriptToEntity(IScriptState* pState)
{
	CScriptFuncArgString* pScriptName = static_cast<CScriptFuncArgString*>(pState->GetArg(0));
	CScriptFuncArgString* pEntityName = static_cast< CScriptFuncArgString* >(pState->GetArg(1));
	IEntity* pEntity = dynamic_cast<ICharacter*>(m_pEntityManager->GetEntity(pEntityName->m_sValue));
	if (pEntity) {
		pEntity->AttachScript(pScriptName->m_sValue);
	}
	if (!pEntity) {
		CEException e(string("Error : entity '") + pEntityName->m_sValue + "' doesn't exists");
		throw e;
	}
}

void IsIntersect(IScriptState* pState)
{
	CScriptFuncArgInt* pEntity1Id = static_cast< CScriptFuncArgInt* >(pState->GetArg(0));
	CScriptFuncArgInt* pEntity2Id = static_cast< CScriptFuncArgInt* >(pState->GetArg(1));
	IEntity* pEntity1 = m_pEntityManager->GetEntity(pEntity1Id->m_nValue);
	IEntity* pEntity2 = m_pEntityManager->GetEntity(pEntity2Id->m_nValue);
	string sErrorMessage;
	if (!pEntity1)
		sErrorMessage = "Error : entity id '" + std::to_string(pEntity1Id->m_nValue) + "' doesn't exists";
	else if (!pEntity2)
		sErrorMessage = "Error : entity id '" + std::to_string(pEntity2Id->m_nValue) + "' doesn't exists";
	else {
		int ret = pEntity1->TestCollision(pEntity2) ? 1 : 0;
		pState->SetReturnValue(ret);
	}
	if (!sErrorMessage.empty())
		throw CEException(sErrorMessage);
}

void DisplayBBox( IScriptState* pState )
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgInt* pDraw = static_cast< CScriptFuncArgInt* >( pState->GetArg( 1 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pID->m_nValue );
	if (!pEntity) {
		ostringstream oss;
		oss << "Erreur : DisplayBBox(" << pID->m_nValue << ") -> entite " << pID->m_nValue << " introuvable.";
		m_pConsole->Println(oss.str());
	}
	else {
		bool bDraw = pDraw->m_nValue == 1 ? true : false;
		pEntity->DrawBoundingBox(bDraw);
	}
}

void DisplayBBoxInfos(IScriptState* pState)
{
	CScriptFuncArgInt* pID = static_cast<CScriptFuncArgInt*>(pState->GetArg(0));
	IEntity* pEntity = m_pEntityManager->GetEntity(pID->m_nValue);
	IBox* pBox = dynamic_cast<IBox*>(pEntity->GetBoundingGeometry());
	if (pBox) {
		ostringstream oss;
		oss << "Dimensions : ( " << pBox->GetDimension().m_x << ", " << pBox->GetDimension().m_y << ", " << pBox->GetDimension().m_z << ")  "
			<< " min point : " << pBox->GetMinPoint().m_x << ", " << pBox->GetMinPoint().m_y << ", " << pBox->GetMinPoint().m_z << ")";
		m_pConsole->Println(oss.str());
	}
}

void DisplayBkgColor( IScriptState* pState )
{
	CVector vColor;
	m_pRenderer->GetBackgroundColor( vColor );
	ostringstream oss;
	oss << (int)( vColor.m_x * 255. )<< " , " << (int)( vColor.m_y * 255. )<< " , " << (int)( vColor.m_z * 255. );
	m_pConsole->Println( oss.str() );
}

void SetBkgColor( IScriptState* pState )
{
	vector< int > v;
	for( int i = 0; i < 3; i++ )
	{
		CScriptFuncArgInt* pArg = static_cast< CScriptFuncArgInt* >( pState->GetArg( i ) );
		v.push_back( pArg->m_nValue );
	}
	m_pRenderer->SetBackgroundColor( (float)v[0 ] / 255.f, (float)v[ 1 ] / 255.f, (float)v[ 2 ] / 255.f, 1 );
}

void DisplayCamPos( IScriptState* pState )
{
	CVector vPos;
	ICamera* pCamera = m_pCameraManager->GetActiveCamera();
	ostringstream oss;
	if (pCamera) {
		pCamera->GetWorldPosition(vPos);
		oss << vPos.m_x << " , " << vPos.m_y << " , " << vPos.m_z;
	}
	else
		oss << "Error : No current camera defined";
	m_pConsole->Println( oss.str() );
}

void PrintHUD(IScriptState* pState)
{
	CScriptFuncArgString* pText = static_cast<CScriptFuncArgString*>(pState->GetArg(0));
	CScriptFuncArgInt* px = static_cast<CScriptFuncArgInt*>(pState->GetArg(1));
	CScriptFuncArgInt* py = static_cast<CScriptFuncArgInt*>(pState->GetArg(2));
	int slot = m_pHud->CreateNewSlot(px->m_nValue, py->m_nValue);
	m_pHud->PrintInSlot(slot, 0, pText->m_sValue);
}

void RemoveHudSlot(IScriptState* pState)
{
	CScriptFuncArgInt* pSlot = static_cast<CScriptFuncArgInt*>(pState->GetArg(0));
	m_pHud->RemoveText(pSlot->m_nValue);
}

void EntityCallback(CPlugin*, IEventDispatcher::TEntityEvent e, IEntity* pEntity)
{
	if (e == IEventDispatcher::TEntityEvent::T_UPDATE) {
		CVector pos, localPos;
		pEntity->GetWorldPosition(pos);
		pEntity->GetLocalPosition(localPos);
		ostringstream oss;
		oss << "Entity " << pEntity->GetID() << ", world position = (" << pos.m_x << ", " << pos.m_y << ", " << pos.m_z 
											<< "), local position = (" << localPos.m_x << ", " << localPos.m_y << ", " << localPos.m_z << ")";
		int slot = 0;
		if (g_mSlots.empty())
			g_mSlots[pEntity] = m_pHud->CreateNewSlot(800, 100);
		else {
			map<IEntity*, int>::iterator itEntity = g_mSlots.find(pEntity);
			if (itEntity == g_mSlots.end()) {
				slot = m_pHud->CreateNewSlot(800, 100 + m_pHud->GetSlotCount() * 20);
				g_mSlots[pEntity] = slot;
			}
			else
				slot = itEntity->second;
		}
		m_pHud->PrintInSlot(slot, 0, oss.str());
		if (g_bEnableWatchLog) {
			FILE* pFile = fopen("log.txt", "a");
			if (pFile) {
				oss << "\n";
				fwrite(oss.str().c_str(), sizeof(char), oss.str().size(), pFile);
				fclose(pFile);
			}
		}
	}
}

void WatchEntityPosition(IScriptState* pState)
{
	CScriptFuncArgInt* pId = dynamic_cast<CScriptFuncArgInt*>((pState->GetArg(0)));
	IEntity* pEntity = m_pEntityManager->GetEntity(pId->m_nValue);
	if(pEntity)
		pEntity->AbonneToEntityEvent(EntityCallback);
	else {
		ostringstream oss;
		oss << "Error : entity " << pId->m_nValue << " not found";
		m_pConsole->Println(oss.str());
	}
}

void StopWatchEntityPosition(IScriptState* pState)
{
	CScriptFuncArgInt* pId = dynamic_cast<CScriptFuncArgInt*>((pState->GetArg(0)));
	IEntity* pEntity = m_pEntityManager->GetEntity(pId->m_nValue);
	int i = 0;
	pEntity->DeabonneToEntityEvent(EntityCallback);
}

void SetCamPos( IScriptState* pState )
{
	CScriptFuncArgFloat* px = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 0 ) );
	CScriptFuncArgFloat* py = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 1 ) );
	CScriptFuncArgFloat* pz = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 2 ) );
	ICamera* pCamera = m_pCameraManager->GetActiveCamera();
	if (pCamera)
		pCamera->SetLocalPosition(px->m_fValue, py->m_fValue, pz->m_fValue);
	else
		m_pConsole->Println("Erreur : Aucune camera active");
}

void YawCamera(IScriptState* pState)
{
	CScriptFuncArgFloat* pYaw = static_cast< CScriptFuncArgFloat* >(pState->GetArg(0));
	m_pCameraManager->GetActiveCamera()->Yaw(pYaw->m_fValue);
}

void PitchCamera(IScriptState* pState)
{
	CScriptFuncArgFloat* pPitch = static_cast< CScriptFuncArgFloat* >(pState->GetArg(0));
	m_pCameraManager->GetActiveCamera()->Pitch(pPitch->m_fValue);
}

void RollCamera(IScriptState* pState)
{
	CScriptFuncArgFloat* pRoll = static_cast< CScriptFuncArgFloat* >(pState->GetArg(0));
	m_pCameraManager->GetActiveCamera()->Roll(pRoll->m_fValue);
}

void SetEntityPos( IScriptState* pState )
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgFloat* px = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 1 ) );
	CScriptFuncArgFloat* py = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 2 ) );
	CScriptFuncArgFloat* pz = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 3 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pID->m_nValue );
	if (pEntity)
		pEntity->SetWorldPosition(px->m_fValue, py->m_fValue, pz->m_fValue);
	else
		m_pConsole->Println("Identifiant invalide");
}

void SetEntityDummyRootPos(IScriptState* pState)
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >(pState->GetArg(0));
	CScriptFuncArgFloat* px = static_cast< CScriptFuncArgFloat* >(pState->GetArg(1));
	CScriptFuncArgFloat* py = static_cast< CScriptFuncArgFloat* >(pState->GetArg(2));
	CScriptFuncArgFloat* pz = static_cast< CScriptFuncArgFloat* >(pState->GetArg(3));
	IEntity* pEntity = m_pEntityManager->GetEntity(pID->m_nValue);
	if (pEntity) {
		IBone* pDummy = pEntity->GetSkeletonRoot();
		if(pDummy)
			pDummy->SetWorldPosition(px->m_fValue, py->m_fValue, pz->m_fValue);
		else 
			m_pConsole->Println("Cet entite ne possede pas de dummy root");
	}
	else
		m_pConsole->Println("Identifiant invalide");
}

void DisplayEntityPosition( IScriptState* pState )
{
	ostringstream oss;
	CScriptFuncArgInt* pInt = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pInt->m_nValue );
	if (pEntity) {
		CVector vPos;
		pEntity->GetWorldPosition(vPos);
		oss << vPos.m_x << " , " << vPos.m_y << " , " << vPos.m_z;
	}
	else
		oss << "Error : entity id " << pInt->m_nValue << " does not exists";
	m_pConsole->Println(oss.str());
}

void Exit( IScriptState* pState )
{
	m_pWindow->Close();
}

void GenerateAssemblerListing(IScriptState* pState)
{
	CScriptFuncArgInt* pEnable = (CScriptFuncArgInt*)(pState->GetArg(0));
	m_pScriptManager->GenerateAssemblerListing(pEnable->m_nValue);
}

void CreateEntity( IScriptState* pState )
{
	CScriptFuncArgString* pName = static_cast< CScriptFuncArgString* >( pState->GetArg( 0 ) );
	string sName = pName->m_sValue;
	bool bak = m_pRessourceManager->IsCatchingExceptionEnabled();
	m_pRessourceManager->EnableCatchingException( false );
	try
	{
		IEntity* pEntity = m_pEntityManager->CreateEntity(sName, "");
		int id = m_pEntityManager->GetEntityID(pEntity);
		pEntity->Link( m_pScene );
		ostringstream oss;
		oss << "L'entité \"" << pName->m_sValue << "\"a été chargée avec l'identifiant " << id << ".";
		m_pConsole->Println( oss.str() );
		pState->SetReturnValue(id);
	}
	catch( CFileNotFoundException& e )
	{		
		string sMessage = string( "Erreur : fichier \"" ) + e.m_sFileName + "\" manquant.";
		m_pConsole->Println( sMessage );
	}
	catch( CRessourceException& e )
	{
		string s;
		e.GetErrorMessage( s );
		m_pConsole->Println( s );
	}
	catch( CBadFileFormat )
	{
		ostringstream oss;
		oss << "\"" << sName << "\" : Mauvais format de fichier, essayez de le réexporter";
		m_pConsole->Println( oss.str() );
	}
	catch( CEException& e )
	{
		m_pConsole->Println( e.what() );
	}
	m_pRessourceManager->EnableCatchingException( bak );
}

void LoadMap(IScriptState* pState)
{
	CScriptFuncArgString* pName = static_cast< CScriptFuncArgString* >(pState->GetArg(0));
	string sFileName = pName->m_sValue;
	try
	{
		m_pMapEditor->Load(pName->m_sValue);
	}
	catch (CFileNotFoundException& e)
	{
		string s = string("Fichier \"") + e.what() + "\" introuvable";
		m_pConsole->Println(s);
	}
	catch (CExtensionNotFoundException)
	{
		m_pConsole->Println("Erreur inconnue, veuillez contacter l'administrateur pour plus d'information");
	}
	catch (CBadFileFormat)
	{
		m_pConsole->Println("Mauvais format de fichier,essayez de réexporter la scene");
	}
	catch (CEException& e)
	{
		m_pConsole->Println(e.what());
	}
}

void SaveMap(IScriptState* pState)
{
	CScriptFuncArgString* pName = static_cast< CScriptFuncArgString* >(pState->GetArg(0));
	string sName = pName->m_sValue;
	try
	{
		m_pMapEditor->Save(sName);
		m_pConsole->Println("Map sauvegardée");
	}
	catch (CFileException& e) {
		m_pConsole->Println(string("Erreur d'acces au fichier \"") + sName + "\", verifiez que vous disposez des droits suffisants et que votre antivirus ne bloque pas l'operation");
	}
	catch (CEException e)	{
		m_pConsole->Println(e.what());
	}
}

void Advertise(IScriptState* pState)
{
	CScriptFuncArgString* pCharacterId = (CScriptFuncArgString*)pState->GetArg(0);

}

void LoadWorld(IScriptState* pState)
{
	try
	{
		CScriptFuncArgString* pWorldName = (CScriptFuncArgString*)pState->GetArg(0);
		m_pWorldEditor->SetEditionMode(false);
		m_pWorldEditor->Load(pWorldName->m_sValue);
	}
	catch (CFileException& e)
	{
		m_pConsole->Println(string("Erreur d'acces au fichier  \"") + e.what() + "\", verifiez que vous disposez des droits suffisants et que votre antivirus ne bloque pas l'operation");
	}
	catch (CEException& e)
	{
		m_pConsole->Println(e.what());
	}
}

void SaveWorld(IScriptState* pState)
{
	try
	{
		CScriptFuncArgString* pWorldName = (CScriptFuncArgString*)pState->GetArg(0);
		m_pWorldEditor->Save(pWorldName->m_sValue);
		m_pConsole->Println("Monde sauvegardé");
	}
	catch (CFileException& e) {
		m_pConsole->Println(string("Erreur d'acces au fichier  \"") + e.what() + "\", verifiez que vous disposez des droits suffisants et que votre antivirus ne bloque pas l'operation");
	}
	catch (CEException e) {
		m_pConsole->Println(e.what());
	}
}

void SaveGame(IScriptState* pState)
{
	CScriptFuncArgString* pFileName = static_cast< CScriptFuncArgString* >(pState->GetArg(0));
	m_pWorldEditor->SaveGame(pFileName->m_sValue);
}

void LoadGame(IScriptState* pState)
{
	CScriptFuncArgString* pFileName = static_cast< CScriptFuncArgString* >(pState->GetArg(0));
	m_pWorldEditor->Load(pFileName->m_sValue);
}

void Merge( IScriptState* pState )
{
	CScriptFuncArgString* pString = static_cast< CScriptFuncArgString* >( pState->GetArg( 0 ) );
	CScriptFuncArgFloat* px = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 2 ) );
	CScriptFuncArgFloat* py = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 3 ) );
	CScriptFuncArgFloat* pz = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 4 ) );
	
	m_pScene->Merge(pString->m_sValue, px->m_fValue, py->m_fValue, pz->m_fValue);
}

void TestMessageBox( IScriptState* pState )
{
	CScriptFuncArgString* pMessage = static_cast< CScriptFuncArgString* >( pState->GetArg( 0 ) );
	CScriptFuncArgString* pCaption = static_cast< CScriptFuncArgString* >( pState->GetArg( 1 ) );
	CScriptFuncArgInt* pBoxType = static_cast< CScriptFuncArgInt* >( pState->GetArg( 2 ) );
	MessageBox( NULL, pMessage->m_sValue.c_str(), pCaption->m_sValue.c_str(), pBoxType->m_nValue );
}

void Operation( IScriptState* pState )
{
	CScriptFuncArgFloat* p0 = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 0 ) );
	CScriptFuncArgFloat* p1 = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 1 ) );
	CScriptFuncArgFloat* p2 = static_cast< CScriptFuncArgFloat* >( pState->GetArg( 2 ) );
	ostringstream oss;
	oss << "arg 0 = " << p0->m_fValue << "\narg 1 = " << p1->m_fValue << "\narg 2 = " << p2->m_fValue;
	m_pConsole->Println( oss.str() );
}

void Operation3( IScriptState* pState )
{
	CScriptFuncArgInt* p0 = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	CScriptFuncArgInt* p1 = static_cast< CScriptFuncArgInt* >( pState->GetArg( 1 ) );
	CScriptFuncArgInt* p2 = static_cast< CScriptFuncArgInt* >( pState->GetArg( 2 ) );
	ostringstream oss;
	oss << "arg 0 = " << p0->m_nValue << "\narg 1 = " << p1->m_nValue << "\narg 2 = " << p2->m_nValue;
	MessageBox( NULL, oss.str().c_str(), "", MB_OK );
}

void cls( IScriptState* pState )
{
	m_pConsole->Cls();
}

void SetRenderType( IScriptState* pState )
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pID->m_nValue );
	if( pEntity )
	{
		CScriptFuncArgString* pType = static_cast< CScriptFuncArgString* >( pState->GetArg( 1 ) );
		if( pType->m_sValue == "line" )
			pEntity->SetRenderingType( IRenderer::eLine );
		else if( pType->m_sValue == "fill" )
			pEntity->SetRenderingType( IRenderer::eFill );
		else if( pType->m_sValue == "point" )
			pEntity->SetRenderingType( IRenderer::ePoint );
	}
}

void DisplayBoundingSphere( IScriptState* pState )
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pID->m_nValue );
	if( pEntity )
	{
		CScriptFuncArgInt* pBool = static_cast< CScriptFuncArgInt* >( pState->GetArg( 1 ) );
		bool bDraw = pBool->m_nValue == 1 ? true : false;
		pEntity->DrawBoundingSphere( bDraw );
	}
}

void DisplayBoneBoundingSphere( IScriptState* pState )
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pID->m_nValue );
	if( pEntity )
	{
		CScriptFuncArgInt* pBoneID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 1 ) );
		CScriptFuncArgInt* pBool = static_cast< CScriptFuncArgInt* >( pState->GetArg( 2 ) );
		bool bDraw = pBool->m_nValue == 1 ? true : false;
		pEntity->DrawBoneBoundingSphere( pBoneID->m_nValue, bDraw );
	}
}

void Unlink( IScriptState* pState )
{
	CScriptFuncArgInt* pID = static_cast< CScriptFuncArgInt* >( pState->GetArg( 0 ) );
	IEntity* pEntity = m_pEntityManager->GetEntity( pID->m_nValue );
	if( pEntity )
		pEntity->Unlink();
}

void SetCameraMatrix(IScriptState* pState)
{
	m_pRenderer->LockCamera(false);
	vector<float> v;
	for (int i = 0; i < 16; i++) {
		CScriptFuncArgFloat* a = (CScriptFuncArgFloat*)pState->GetArg(i);
		v.push_back(a->m_fValue);
	}

	CMatrix m;
	m.Set(&v[0]);
	m_pRenderer->SetCameraMatrix(m);
	m_pRenderer->LockCamera(true);
}

void LockCamera(IScriptState* pState)
{
	CScriptFuncArgInt* pLock = (CScriptFuncArgInt*)pState->GetArg(0);
	m_pRenderer->LockCamera(pLock->m_nValue);
}

void DisplayMatrix(CMatrix m)
{
	string s;
	m_pDebugTool->SerializeMatrix(m, 0.f, s);
	m_pConsole->Println(s);
}

void DisplayEntityMatrix(IScriptState* pState)
{
	CScriptFuncArgInt* pID = dynamic_cast<CScriptFuncArgInt*>(pState->GetArg(0));
	IEntity* pEntity = m_pEntityManager->GetEntity(pID->m_nValue);
	if (pEntity)
		DisplayMatrix(pEntity->GetWorldMatrix());
}

void DisplayModelViewProjectionMatrix(IScriptState* pState)
{
	CMatrix m;
	m_pRenderer->GetModelViewProjectionMatrix(m);
	DisplayMatrix(m);
}

void DisplayCameraMatrix(IScriptState* pState)
{
	CMatrix m;
	ICamera* pCamera = m_pCameraManager->GetActiveCamera();
	if (pCamera)
		DisplayMatrix(pCamera->GetWorldMatrix());
}

void DisplayProjectionMatrix(IScriptState* pState)
{
	CMatrix m;
	m_pRenderer->GetProjectionMatrix(m);
	DisplayMatrix(m);
}

void SetProjectionMatrixType(IScriptState* pState)
{
	CScriptFuncArgString* pType = (CScriptFuncArgString*)pState->GetArg(0);

	CMatrix m;
	if (pType->m_sValue == "2d") {
		m.SetIdentity();
		unsigned int nWidth, nHeight;
		m_pRenderer->GetResolution(nWidth, nHeight);
		m.m_00 = (float)nHeight / (float)nWidth;
		m_pRenderer->SetProjectionMatrix(m);
	}
	else if (pType->m_sValue == "3d") {
		m_pRenderer->SetFov(60.f);
	}
}

void testCollisionShader(IScriptState* pState)
{
	CScriptFuncArgString* pMode = (CScriptFuncArgString*)(pState->GetArg(0));
	if (pMode) {
		string mode = pMode->m_sValue;
		IMesh* pGroundMesh = dynamic_cast<IMesh*>(m_pScene->GetRessource());
		if (mode == "collision") {
			IShader* pCollisionShader = m_pRenderer->GetShader("collision");
			pCollisionShader->Enable(true);
			pGroundMesh->SetShader(pCollisionShader);
		}
		else if (mode == "normal") {
			IShader* pCollisionShader = m_pRenderer->GetShader("PerPixelLighting");
			pCollisionShader->Enable(true);
			pGroundMesh->SetShader(pCollisionShader);
		}
	}
}

void ReloadShader(IScriptState* pState)
{
	try {
		CScriptFuncArgString* pArg = (CScriptFuncArgString*)pState->GetArg(0);
		m_pRenderer->ReloadShader(pArg->m_sValue);
	}
	catch (exception e)
	{
		m_pConsole->Println(e.what());
	}
}

void CullFace(IScriptState* pState)
{
 	CScriptFuncArgInt* pArg = (CScriptFuncArgInt*)pState->GetArg(0);
	m_pRenderer->CullFace(pArg->m_nValue == 0 ? false : true);
}

void EnableRenderCallback(IScriptState* pState)
{
	CScriptFuncArgString* pName = (CScriptFuncArgString*)pState->GetArg(0);
	CScriptFuncArgInt* pEnable = (CScriptFuncArgInt*)pState->GetArg(1);
	CPlugin* plugin = CPlugin::GetPlugin(pName->m_sValue);
	if (plugin) {
		plugin->EnableRenderEvent(pEnable->m_nValue == 0 ? false : true);
	}
	else
		m_pConsole->Println("Plugin \"" + pName->m_sValue + "\" not found");
}

void SetLineWidth(IScriptState* pState)
{
	CScriptFuncArgInt* pWidth = (CScriptFuncArgInt*)pState->GetArg(0);
	m_pRenderer->SetLineWidth(pWidth->m_nValue);
}

void PatchBMEMeshTextureName(IScriptState* pState)
{
	CScriptFuncArgString* pBMEName = (CScriptFuncArgString*)pState->GetArg(0);
	CScriptFuncArgString* pTextureName = (CScriptFuncArgString*)pState->GetArg(1);

	ILoader::CAnimatableMeshData mi;
	ILoader::CAnimatableMeshData test;
	m_pLoaderManager->Load(pBMEName->m_sValue, mi);
	if (mi.m_vMeshes.size() == 1) {
		ILoader::CMaterialInfos& matInfo = mi.m_vMeshes[0].m_oMaterialInfos;
		matInfo.m_sDiffuseMapName = pTextureName->m_sValue;
		matInfo.m_bExists = true;
		for (int i = 0; i < 4; i++) {
			matInfo.m_vAmbient.push_back(1.f);
			matInfo.m_vDiffuse.push_back(1.f);
			matInfo.m_vSpecular.push_back(1.f);
		}
		
		m_pLoaderManager->Export(pBMEName->m_sValue, mi);
	}
}

void OpenConsole(IScriptState* pState)
{
	CScriptFuncArgInt* pOpen = static_cast<CScriptFuncArgInt*>(pState->GetArg(0));
	m_pConsole->Open(pOpen->m_nValue != 0);
}

void ResetFreeCamera(IScriptState* pState)
{
	ICamera* pFreeCamera = m_pCameraManager->GetCameraFromType(ICameraManager::TFree);
	CMatrix m;
	pFreeCamera->SetLocalMatrix(m);
}

void PrintReg(IScriptState* pState)
{
	CScriptFuncArgString* pReg = (CScriptFuncArgString*)pState->GetArg(0);
	float regValue = m_pScriptManager->GetRegisterValue(pReg->m_sValue);
	m_pConsole->Print(regValue);
}

void DisplayGroundHeight(IScriptState* pState)
{
	CScriptFuncArgFloat* px = (CScriptFuncArgFloat*)pState->GetArg(0);
	CScriptFuncArgFloat* pz = (CScriptFuncArgFloat*)pState->GetArg(1);
	float h = m_pCollisionManager->GetMapHeight(0, px->m_fValue, pz->m_fValue);
	m_pConsole->Println(h);
}

void SetGroundMargin(IScriptState* pState)
{
	CScriptFuncArgFloat* pMargin = (CScriptFuncArgFloat*)pState->GetArg(0);
	IScene* pScene = dynamic_cast<IScene*>(m_pScene);
	pScene->SetGroundMargin(pMargin->m_fValue);
}

void DisplayGroundMargin(IScriptState* pState)
{
	IScene* pScene = dynamic_cast<IScene*>(m_pScene); ;
	m_pConsole->Println(pScene->GetGroundMargin());
}

void CreatePlaneEntity(IScriptState* pState)
{
	CScriptFuncArgInt* pSlices = (CScriptFuncArgInt*)pState->GetArg(0);
	CScriptFuncArgInt* pSize = (CScriptFuncArgInt*)pState->GetArg(1);
	CScriptFuncArgString* pHeightTextureName = (CScriptFuncArgString*)pState->GetArg(2);
	CScriptFuncArgString* pDiffuseTextureName = (CScriptFuncArgString*)pState->GetArg(3);
	IEntity* pEntity = m_pEntityManager->CreatePlaneEntity(pSlices->m_nValue, pSize->m_nValue, pHeightTextureName->m_sValue, pDiffuseTextureName->m_sValue);
	pEntity->Link(m_pScene);
	pState->SetReturnValue(m_pEntityManager->GetEntityID(pEntity));
}

void SetCollisionMapBias(IScriptState* pState)
{
	CScriptFuncArgFloat* pBias = static_cast<CScriptFuncArgFloat*>(pState->GetArg(0));
	m_pMapEditor->SetBias(pBias->m_fValue);
}

void GetTime(IScriptState* pState)
{
	__int64 t, filter = 0xffffff;
	time(&t);
	t = t & filter;
	pState->SetReturnValue(t);
}

void RegisterAllFunctions( IScriptManager* pScriptManager )
{
	vector< TFuncArgType > vType;

	vType.clear();
	vType.push_back(eString);
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("WearCharacterItem", WearCharacterItem, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("GetItemCount", GetItemCount, vType, eInt);

	vType.clear();
	vType.push_back(eString);
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("AddCharacterItem", AddCharacterItem, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("RemoveCharacterItem", RemoveCharacterItem, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("AddItem", AddItem, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("RemoveItem", RemoveItem, vType, eVoid);	

	vType.clear();
	m_pScriptManager->RegisterFunction("LoadTopicFile", LoadTopicFile, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("Choice", Choice, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("Debug", Debug, vType, eInt);

	vType.clear();
	m_pScriptManager->RegisterFunction("GetTime", GetTime, vType, eInt);

	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("SetNPCState", SetNPCState, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("IsIntersect", IsIntersect, vType, eInt);

	vType.clear();
	vType.push_back(eString);
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("AttachScriptToEntity", AttachScriptToEntity, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	vType.push_back(eInt);
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("PrintHUD", PrintHUD, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("RemoveHUDSlot", RemoveHudSlot, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction("Reset", Reset, vType, eVoid);

	vType.clear();
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("SetCollisionMapBias", SetCollisionMapBias, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("TestRegExpr", TestRegExpr, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("IsAbsolutePath", IsAbsolutePath, vType, eInt);

	vType.clear();
	vType.push_back(eInt);
	vType.push_back(eInt);
	vType.push_back(eInt);
	vType.push_back(eInt);
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("EnablePathFindSaving", EnablePathFindSaving, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction("AdaptGroundToAllEntities", AdaptGroundToAllEntities, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("GetCharacterID", GetCharacterID, vType, eInt);
	
	
	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("SaveGame", SaveGame, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("LoadGame", LoadGame, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	vType.push_back(eInt);
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("CreateCollisionMap", CreateCollisionMap, vType, eVoid);
		
	vType.clear();
	m_pScriptManager->RegisterFunction("SaveCloth", SaveCloth, vType, eVoid);

	vType.clear();
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("OffsetCloth", OffsetCloth, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("EditCloth", EditCloth, vType, eVoid);

	vType.clear();
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("OffsetEyes", OffsetEyes, vType, eVoid);

	vType.clear();
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("TurnEyes", TurnEyes, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction("SaveCurrentEditableBody", SaveCurrentEditableBody, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("SetBody", SetBody, vType, eVoid);
	
	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("EnableInstancingMode", EnableInstancingMode, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("SetTexture", SetTexture, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("SetTextureInWorld", SetTextureInWorld, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("DrawCollisionModels", DrawCollisionModels, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("GenerateAssemblerListing", GenerateAssemblerListing, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("ColorizeEntity", ColorizeEntity, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("SetEntityShininess", SetEntityShininess, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("SetEntitySpecular", SetEntitySpecular, vType, eVoid);

	vType.clear();
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("SetSpecular", SetSpecular, vType, eVoid);

	vType.clear();
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("SetShininess", SetShininess, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	vType.push_back(eInt);
	vType.push_back(eString);
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("CreatePlaneEntity", CreatePlaneEntity, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction("GetOpenglVersion", DisplayOpenglVersion, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction("DisplayGlslVersion", DisplayGlslVersion, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("EnableHMHack", EnableHMHack, vType, eVoid);

	vType.clear();
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("SetGroundMargin", SetGroundMargin, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction("GetPlayerId", GetPlayerId, vType, eInt);

	vType.clear();
	m_pScriptManager->RegisterFunction("DisplayGroundMargin", DisplayGroundMargin, vType, eVoid);

	vType.clear();
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("DisplayGroundHeight", DisplayGroundHeight, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("SpawnEntity", SpawnEntity, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("SpawnCharacter", SpawnCharacter, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("SpawnArea", SpawnArea, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("EditCharacter", EditCharacter, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("ChangeCharacterName", ChangeCharacterName, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction("NormalizeCharacterDatabase", NormalizeCharacterDatabase, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("EditWorld", EditWorld, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("EditMap", EditMap, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("SetHairs", SetHairs, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("PrintReg", PrintReg, vType, eVoid);
	
	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("ShowGUICursor", ShowGUICursor, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("Kill", Kill, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("DisplayCamera", DisplayCamera, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("InitCamera", InitCamera, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("PatchBMEMeshTextureName", PatchBMEMeshTextureName, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction("ResetFreeCamera", ResetFreeCamera, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("SetLineWidth", SetLineWidth, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eInt );
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "DisplayBoneBoundingSphere", DisplayBoneBoundingSphere, vType, eVoid);

	vType.push_back( eString );
	vType.push_back( eFloat);
	vType.push_back( eFloat );
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "Merge", Merge, vType, eVoid);

	vType.clear();
	vType.push_back( eString );
	vType.push_back( eString );
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "MessageBox", TestMessageBox, vType, eVoid);

	vType.clear();
	vType.push_back( eFloat );
	vType.push_back( eFloat );
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "Operation", Operation, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eInt );
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "Operation3", Operation3, vType, eVoid);

	vType.clear();
	vType.push_back( eString );
	m_pScriptManager->RegisterFunction( "LoadMap", LoadMap, vType, eVoid);

	vType.clear();
	vType.push_back( eString );
	m_pScriptManager->RegisterFunction( "SaveMap", SaveMap, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("LoadWorld", LoadWorld, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("SaveWorld", SaveWorld, vType, eVoid);

	vType.clear();
	vType.push_back( eString );
	m_pScriptManager->RegisterFunction( "CreateEntity", CreateEntity, vType, eInt);

	vType.clear();
	m_pScriptManager->RegisterFunction( "cls", cls, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction( "exit", Exit, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "DisplayEntityPosition", DisplayEntityPosition, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eFloat );
	vType.push_back( eFloat );
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "SetEntityPos", SetEntityPos, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("SetEntityDummyRootPos", SetEntityDummyRootPos, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction( "DisplayCamPos", DisplayCamPos, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eInt );
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "SetBkgColor", SetBkgColor, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction( "DisplayBkgColor", DisplayBkgColor, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "DisplayBBox", DisplayBBox, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction( "DisplayEntities", DisplayEntities, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("DisplayCollisionEntities", DisplayCollisionEntities, vType, eVoid);
	

	vType.clear();
	m_pScriptManager->RegisterFunction("DisplayMobileEntities", DisplayMobileEntities, vType, eVoid);
	
	vType.clear();
	vType.push_back( eString );
	m_pScriptManager->RegisterFunction( "GetEntityID", GetEntityID, vType, eInt );

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eString );
	m_pScriptManager->RegisterFunction( "SetEntityShader", SetEntityShader, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "YawEntity", YawEntity, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "PitchEntity", PitchEntity, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "RollEntity", RollEntity, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eInt );
	vType.push_back( eInt );
	vType.push_back( eString );
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "CreateLight", CreateLight, vType, eInt);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "SetLightIntensity", SetLightIntensity, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("SetLightAmbient", SetLightAmbient, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("DisplayLightIntensity", DisplayLightIntensity, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "DisplayBBoxInfos", DisplayBBoxInfos, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction( "ScreenCapture", ScreenCapture, vType, eVoid);

	vType.clear();
	vType.push_back( eString );
	m_pScriptManager->RegisterFunction( "CreateHM", CreateHM, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("CreateHMFromFile", CreateHMFromFile, vType, eVoid);

	vType.clear();
	vType.push_back( eString );
	m_pScriptManager->RegisterFunction( "LoadHM", LoadHM, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "SetEntityWeight", SetEntityWeight, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction( "ClearScene", ClearScene, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	vType.push_back(eString);
	vType.push_back(eInt);
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction( "SetSceneMap", SetSceneMap, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction( "ExportBMEToAscii", ExportBMEToAscii, vType, eVoid);

	vType.clear();
	vType.push_back( eString );
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "ExportBKEToAscii", ExportBKEToAscii, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("ExportBSEToAscii", ExportBSEToAscii, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "DisplayRepere", DisplayRepere, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("DisplayPickingRaySelected", DisplayPickingRaySelected, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("DisplayPickingRayMouseMove", DisplayPickingRayMouseMove, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("DisplayPickingIntersectPlane", DisplayPickingIntersectPlane, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction("DisplayCharacters", DisplayCharacters, vType, eVoid);

	vType.clear();
	vType.push_back( eString );
	m_pScriptManager->RegisterFunction( "LoadImage", LoadImage, vType, eVoid);

	vType.clear();
	vType.push_back( eString );
	m_pScriptManager->RegisterFunction( "run", run, vType, eVoid);

	vType.clear();
	vType.push_back( eFloat );
	vType.push_back( eFloat );
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "SetCamPos", SetCamPos, vType, eVoid);

	vType.clear();
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("YawCamera", YawCamera, vType, eVoid);

	vType.clear();
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("PitchCamera", PitchCamera, vType, eVoid);

	vType.clear();
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("RollCamera", RollCamera, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("WatchEntityPosition", WatchEntityPosition, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("StopWatchEntityPosition", StopWatchEntityPosition, vType, eVoid);

	vType.clear();
	vType.push_back( eString );
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "CreateLightw", CreateLightw, vType, eInt );

	vType.clear();
	vType.push_back( eString );
	m_pScriptManager->RegisterFunction( "flist", flist, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction( "DisplaySceneChilds", DisplaySceneChilds, vType, eVoid);

	vType.clear();
	vType.push_back( eString );
	m_pScriptManager->RegisterFunction( "DisplayHM", DisplayHM, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction( "StopDisplayHM", StopDisplayHM, vType, eVoid);
	
	vType.clear();
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "SetHMPrecision", SetHMPrecision, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "DisplayShaderName", DisplayShaderName, vType, eVoid);

	vType.clear();
	vType.push_back( eString );
	m_pScriptManager->RegisterFunction( "LoadShader", LoadShader, vType, eVoid);
	
	//vType.clear();
	//m_pScriptManager->RegisterFunction( "ClearRessources", ClearRessources, vType );

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eString );
	m_pScriptManager->RegisterFunction( "SetAnimation", SetAnimation, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction( "PlayCurrentAnimation", PlayCurrentAnimation, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction( "reset", reset, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "DisplayEntitySkeletonInfos", DisplayEntitySkeletonInfos, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eString );
	m_pScriptManager->RegisterFunction( "SelectBone", SelectBone, vType, eVoid);

	vType.clear();
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "Yaw", Yaw, vType, eVoid);

	vType.clear();
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "Pitch", Pitch, vType, eVoid);

	vType.clear();
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "Roll", Roll, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "PauseAnimation", PauseAnimation, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "DetachAnimation", DetachAnimation, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "StopAnimation", StopAnimation, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "Sleep", Sleep, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "HideEntity", HideEntity, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eInt );
	vType.push_back( eInt );
	vType.push_back( eInt );
	vType.push_back( eString );
	m_pScriptManager->RegisterFunction( "LinkToId", LinkToId, vType, eVoid);
	
	vType.clear();
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "SetZCollisionError", SetZCollisionError, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eFloat );
	vType.push_back( eFloat );
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "SetConstantLocalTranslate", SetConstantLocalTranslate, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "NextAnimationFrame", NextAnimationFrame, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction( "NextAnimationKey", NextAnimationKey, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "SetAnimationTime", SetAnimationTime, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "DisplayAnimationTime", DisplayAnimationTime, vType, eVoid);

	vType.clear();
	vType.push_back( eString );
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction( "CreateMobileEntity", CreateMobileEntity, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("CreatePlayer", CreatePlayer, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction( "CreateNPC", CreateNPC, vType, eInt);

	vType.clear();
	m_pScriptManager->RegisterFunction("SaveCharacter", SaveCharacter, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("SaveCharacterInWorld", SaveCharacterInWorld, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("RemoveCharacterFromWorld", RemoveCharacterFromWorld, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("RemoveCharacterFromDB", RemoveCharacterFromDB, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("CreateMinimapEntity", CreateMinimapEntity, vType, eVoid);
	
	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("CreateTestEntity", CreateTestEntity, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "Walk", Walk, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "Stand", Stand, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "Run", Run, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "DisplayNodeInfos", DisplayNodeInfos, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "StopRender", StopRender, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eString );
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "SetAnimationSpeed", SetAnimationSpeed, vType, eVoid);

	vType.clear();
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "SetGravity", SetGravity, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "SetCurrentPlayer", SetCurrentPlayer, vType, eVoid);

	vType.clear();
	vType.push_back( eString );
	m_pScriptManager->RegisterFunction( "SetCameraType", SetCameraType, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eFloat );
	vType.push_back( eFloat );
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "LocalTranslate", LocalTranslate, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eString );
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "RunAction", RunAction, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eFloat );
	vType.push_back( eFloat );
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "SetScale", SetScale, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eString );
	m_pScriptManager->RegisterFunction( "SetRenderType", SetRenderType, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "DisplayBoundingSphere", DisplayBoundingSphere, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "Unlink", Unlink, vType, eVoid);

	vType.clear();
	vType.push_back( eString );
	vType.push_back( eString );
	vType.push_back( eString );
	m_pScriptManager->RegisterFunction( "ComputeKeysBoundingBoxes", ComputeKeysBoundingBoxes, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("Attack", Attack, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	vType.push_back(eInt);
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("TalkTo", TalkTo, vType, eVoid);

	vType.clear();
	vType.push_back( eString );
	vType.push_back( eString );
	vType.push_back( eString );
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "SetPreferedKeyBBox", SetPreferedKeyBBox, vType, eVoid);

	vType.clear();
	vType.push_back( eFloat );
	vType.push_back( eFloat );
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "CreateBox", CreateBox, vType, eVoid);

	vType.clear();
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "CreateSphere", CreateSphere, vType, eVoid);

	vType.clear();
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("CreateQuad", CreateQuad, vType, eVoid);

	vType.clear();
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("RayTrace", RayTrace, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "ChangeBase", ChangeBase, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eInt );
	m_pScriptManager->RegisterFunction( "DisplayAnimationBBox", DisplayAnimationBBox, vType, eVoid);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eFloat );
	vType.push_back( eFloat );
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "Goto", Goto, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction( "CreateRepere", CreateRepere, vType, eInt);

	vType.clear();
	vType.push_back( eInt );
	vType.push_back( eString );
	m_pScriptManager->RegisterFunction( "SetEntityName", SetEntityName, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction( "DisplayFov", DisplayFov, vType, eVoid);

	vType.clear();
	vType.push_back( eFloat );
	m_pScriptManager->RegisterFunction( "SetFov", SetFov, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("print", print, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("GetCameraID", GetCameraID, vType, eInt);


	vType.clear();
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("SetCameraMatrix", SetCameraMatrix, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("LockCamera", LockCamera, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("SetProjectionMatrixType", SetProjectionMatrixType, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction("DisplayProjectionMatrix", DisplayProjectionMatrix, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction("DisplayModelViewProjectionMatrix", DisplayModelViewProjectionMatrix, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction("DisplayCameraMatrix", DisplayCameraMatrix, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("DisplayEntityMatrix", DisplayEntityMatrix, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("testCollisionShader", testCollisionShader, vType, eVoid);
	
	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("ReloadShader", ReloadShader, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("CullFace", CullFace, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("EnableRenderCallback", EnableRenderCallback, vType, eVoid);

	vType.clear();
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("CreateLineEntity", CreateLineEntity, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("OpenConsole", OpenConsole, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("GetNodeId", GetNodeId, vType, eInt);

	vType.clear();
	vType.push_back(eInt);
	vType.push_back(eFloat);
	m_pScriptManager->RegisterFunction("SetCurrentAnimationSpeed", SetCurrentAnimationSpeed, vType, eVoid);


	vType.clear();
	vType.push_back(eInt);
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("WearArmorToDummy", WearArmorToDummy, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("WearShoes", WearShoes, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("UnWearShoes", UnWearShoes, vType, eVoid);

	vType.clear();
	m_pScriptManager->RegisterFunction("UnWearAllShoes", UnWearAllShoes, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	vType.push_back(eString);
	m_pScriptManager->RegisterFunction("WearCloth", WearCloth, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("UnwearAllClothes", UnwearAllClothes, vType, eVoid);

	vType.clear();
	vType.push_back(eString);
	vType.push_back(eString);
	vType.push_back(eInt);
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("GenerateRandomNPC", GenerateRandomNPC, vType, eVoid);

	vType.clear();
	vType.push_back(eInt);
	vType.push_back(eInt);
	m_pScriptManager->RegisterFunction("SetLife", SetLife, vType, eVoid);
}