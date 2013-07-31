#include "Scene.h"
#include "IRenderer.h"
#include "FreeCamera.h"
#include "ICameraManager.h"
#include "IRessource.h"
#include "Shape.h"
#include "IEntity.h"
#include "LightEntity.h"
#include "ICollisionManager.h"
#include "ISystems.h"
#include "StringUtils.h"
#include "TimeManager.h"

using namespace std;

CScene::Desc::Desc( IRessourceManager& oRessourceManager, IRenderer& oRenderer, IEntityManager* pEntityManager, ICamera* pCamera, ICameraManager& oCameraManager, ILoaderManager& oLoaderManager, ICollisionManager& oCollisionManager, IGeometryManager& oGeometryManager ):
m_oRessourceManager( oRessourceManager ),
m_oRenderer( oRenderer ),
m_pCamera( pCamera ),
m_oCameraManager( oCameraManager ),
m_oLoaderManager( oLoaderManager ),
m_oCollisionManager( oCollisionManager ),
m_oGeometryManager( oGeometryManager )
{
	m_pEntityManager = pEntityManager;
}

CScene::CScene( const Desc& oDesc ):
CEntity( oDesc.m_sFileName, oDesc.m_oRessourceManager, oDesc.m_oRenderer, NULL, oDesc.m_oGeometryManager, oDesc.m_oCollisionManager ),
m_pCamera( oDesc.m_pCamera ),
m_oCameraManager( oDesc.m_oCameraManager ),
m_oLoaderManager( oDesc.m_oLoaderManager ),
m_oCollisionManager( oDesc.m_oCollisionManager ),
m_nHeightMapID( -1 )
{
	SetName( "Scene" );
	m_pRessource = NULL;
	m_pEntityManager = oDesc.m_pEntityManager;
}

CScene::~CScene()
{
}

void CScene::SetRessource( string sFileName, IRessourceManager& oRessourceManager, IRenderer& oRenderer, bool bDuplicate )
{
	CEntity::SetRessource( sFileName, oRessourceManager, oRenderer, bDuplicate );
	int nDotPos = (int)sFileName.find( '.' );
	string sHMFileName = string( "hm_" ) + sFileName.substr( 0, nDotPos ) + ".bmp";
	IMesh* pMesh = static_cast< IMesh* >( m_pRessource );
	try
	{
		m_nHeightMapID = m_oCollisionManager.LoadHeightMap( sHMFileName, pMesh );
	}
	catch( CFileNotFoundException& )
	{
		ILoader::CTextureInfos ti;
		m_oCollisionManager.CreateHeightMap( pMesh, ti, IRenderer::T_BGR );
		ti.m_ePixelFormat = ILoader::eBGR;
		string sFileNameWithoutExt;
		CStringUtils::GetFileNameWithoutExtension( sFileName, sFileNameWithoutExt );
		string sTextureFileName = string( "HM_" ) + sFileNameWithoutExt + ".bmp";
		m_oLoaderManager.Export( sTextureFileName, ti );
		m_nHeightMapID = m_oCollisionManager.LoadHeightMap( sHMFileName, pMesh );
	}
}

IEntity* CScene::Merge( string sRessourceName, string sEntityType, float x, float y, float z )
{
	IEntity* pEntity = m_pEntityManager->CreateEntity( sRessourceName, sEntityType, m_oRenderer );
	pEntity->Link( this );
	pEntity->SetWorldPosition( x, y, z );
	return pEntity;
}

IEntity* CScene::Merge( string sRessourceName, string sEntityType, CMatrix& oXForm )
{
	IEntity* pEntity = m_pEntityManager->CreateEntity( sRessourceName, sEntityType, m_oRenderer );
	pEntity->SetLocalMatrix( oXForm );
	pEntity->Link( this );
	return pEntity;
}

void CScene::Update()
{
	CTimeManager::Instance()->Update();
	CMatrix oCamMatrix;
	m_oCameraManager.GetActiveCamera()->Update();
	m_oCameraManager.GetActiveCamera()->GetWorldMatrix( oCamMatrix );
	m_oRenderer.SetCameraMatrix( oCamMatrix );
	CNode::Update();
	m_oRenderer.SetObjectMatrix( m_oWorldMatrix );
	if ( m_pRessource )
		m_pRessource->Update();
}

void CScene::GetSkeletonEntities( CNode* pRoot, vector< IEntity* >& vEntity, string sFileFilter )
{
	for( unsigned int iChild = 0; iChild < pRoot->GetChildCount(); iChild++ )
	{
		IEntity* pEntity = dynamic_cast< IEntity* >( pRoot->GetChild( iChild ) );
		if( pEntity )
		{
			string sFileName;
			pEntity->GetRessource()->GetFileName( sFileName );
			if( sFileFilter != sFileName )
				vEntity.push_back( pEntity );
		}
		else
			GetSkeletonEntities( pRoot->GetChild( iChild ), vEntity, sFileFilter );
	}
}

ILoader::CSceneObjInfos* CScene::GetEntityInfos( IEntity* pEntity )
{
	ILoader::CSceneObjInfos* pInfos = NULL;
	CLightEntity* pLe = dynamic_cast< CLightEntity* >( pEntity );
	if( pLe )
	{
		pInfos = new ILoader::CLightEntityInfos;
		ILoader::CLightEntityInfos* pLightInfos = static_cast< ILoader::CLightEntityInfos* >( pInfos );
		pLightInfos->m_fIntensity = pLe->GetIntensity();
		pLightInfos->m_oColor = pLe->GetColor();
		ILoader::CLightInfos::TLight type;
		switch( pLe->GetType() )
		{
		case IRessource::DIRECTIONAL:
			type = ILoader::CLightInfos::eDirectionnelle;
			break;
		case IRessource::OMNI:
			type = ILoader::CLightInfos::eOmni;
			break;
		case IRessource::SPOT:
			type = ILoader::CLightInfos::eTarget;
			break;
		}
		pLightInfos->m_eType = type;
	}
	else
	{
		CShape* pRepere = dynamic_cast< CShape* >( pEntity );
		if( pRepere )
			return NULL;
		else
		{
			pInfos = new ILoader::CEntityInfos;
			ILoader::CEntityInfos* pEntityInfos = static_cast< ILoader::CEntityInfos* >( pInfos );
			IAnimation* pAnimation = pEntity->GetCurrentAnimation();
			if( pAnimation )
				pAnimation->GetFileName( pEntityInfos->m_sAnimationFileName );
			pEntityInfos->m_fWeight = pEntity->GetWeight();
			pEntity->GetTypeName( pEntityInfos->m_sTypeName );
			IBone* pSkeleton = pEntity->GetSkeletonRoot();
			if( pSkeleton )
			{
				vector< IEntity* > vSubEntity;
				string sRessourceFileName;
				pEntity->GetRessource()->GetFileName( sRessourceFileName );
				GetSkeletonEntities( pSkeleton, vSubEntity, sRessourceFileName );
				for( unsigned int iSubEntity = 0; iSubEntity < vSubEntity.size(); iSubEntity++ )
				{
					ILoader::CEntityInfos* pSubEntityInfo = dynamic_cast< ILoader::CEntityInfos* >( GetEntityInfos( vSubEntity[ iSubEntity ] ) );
					if( pSubEntityInfo )
						pEntityInfos->m_vSubEntityInfos.push_back( pSubEntityInfo );
				}
			}
		}
	}
	IRessource* pRessource = pEntity->GetRessource();
	if( pRessource )
		pRessource->GetFileName( pInfos->m_sRessourceFileName );
	CMatrix oMat;
	pEntity->GetLocalMatrix( oMat );
	pInfos->m_oXForm = oMat;
	pEntity->GetParent()->GetName( pInfos->m_sParentName  );
	IBone* pParentBone = dynamic_cast< IBone* >( pEntity->GetParent() );
	if( pParentBone )
		pInfos->m_nParentBoneID = pParentBone->GetID();
	string sName;
	pEntity->GetName( sName );
	pInfos->m_sObjectName = sName;
	pEntity->GetRessource()->GetFileName( pInfos->m_sRessourceFileName );
	pEntity->GetRessource()->GetName( pInfos->m_sRessourceName );
	return pInfos;
}

void CScene::GetInfos( ILoader::CSceneInfos& si )
{
	m_pRessource->GetFileName( si.m_sSceneFileName );
	GetName( si.m_sName );
	for( unsigned int i= 0; i < m_vChild.size(); i++ )
	{		
		IEntity* pEntity = static_cast< IEntity* >( m_vChild[ i ] );		
		ILoader::CSceneObjInfos* pInfos = GetEntityInfos( pEntity );
		if( pInfos )
			si.m_vObject.push_back( pInfos );
	}
	IEntity* pPerso = m_pEntityManager->GetPerso();
	string sPersoName;
	if( pPerso )
		pPerso->GetName( sPersoName );
	si.m_sPersoName = sPersoName;
}

void CScene::LoadSceneObject( const ILoader::CSceneObjInfos* pSceneObjInfos, IEntity* pParent )
{
	string sRessourceFileName = pSceneObjInfos->m_sRessourceFileName;
	CMatrix oXForm = pSceneObjInfos->m_oXForm;
	if( sRessourceFileName == "EE_Repere_19051978" )
	{
		IEntity* pRepere = m_pEntityManager->CreateRepere( m_oRenderer );
		pRepere->Link( this );
	}
	else
	{
		const ILoader::CLightEntityInfos* pLightEntityInfos = dynamic_cast< const ILoader::CLightEntityInfos* >( pSceneObjInfos );
		if( pLightEntityInfos )
		{
			if( sRessourceFileName.size() == 0 )
			{
				IRessource::TLight type;
				switch( pLightEntityInfos->m_eType )
				{
				case ILoader::CLightInfos::eDirectionnelle:
					type = IRessource::DIRECTIONAL;
					break;
				case ILoader::CLightInfos::eOmni:
					type = IRessource::OMNI;
					break;
				case ILoader::CLightInfos::eTarget:
					type = IRessource::SPOT;
					break;
				default:
					throw 1;
				}
				IEntity* pEntity = m_pEntityManager->CreateLightEntity( pLightEntityInfos->m_oColor, type, pLightEntityInfos->m_fIntensity );
				pEntity->SetLocalMatrix( oXForm );
				pEntity->Link( this );
			}
			else
				throw 1;
		}
		else
		{
			const ILoader::CEntityInfos* pEntityInfos = dynamic_cast< const ILoader::CEntityInfos* >( pSceneObjInfos );
			IEntity* pEntity = m_pEntityManager->CreateEntity( sRessourceFileName, pEntityInfos->m_sTypeName, m_oRenderer );
			pEntity->SetLocalMatrix( oXForm );

			if( pEntityInfos->m_nParentBoneID != -1 )
			{
				if( pParent->GetSkeletonRoot() )
				{
					IBone* pBone = dynamic_cast< IBone* >( pParent->GetSkeletonRoot()->GetChildBoneByID( pEntityInfos->m_nParentBoneID ) );
					if( pBone )
						pParent->LinkEntityToBone( pEntity, pBone );
					else
					{
						ostringstream oss;
						oss << "Erreur, ";
						CEException e( oss.str() );
						throw e;
					}
				}
			}
			else
				pEntity->Link( pParent );
			if( pEntity->GetSkeletonRoot() )
			{
				pEntity->AddAnimation( pEntityInfos->m_sAnimationFileName );
				pEntity->SetCurrentAnimation( pEntityInfos->m_sAnimationFileName );
				pEntity->GetCurrentAnimation()->Play( true );
			}
			pEntity->SetWeight( pEntityInfos->m_fWeight );
			for( unsigned int iChild = 0; iChild < pEntityInfos->m_vSubEntityInfos.size(); iChild++ )
				LoadSceneObject( pEntityInfos->m_vSubEntityInfos[ iChild ], pEntity );
		}
	}
}

void CScene::Load( const ILoader::CSceneInfos& si )
{
	SetRessource( si.m_sSceneFileName, m_oRessourceManager, m_oRenderer );
	for( unsigned int i = 0; i < si.m_vObject.size(); i++ )
	{
		const ILoader::CSceneObjInfos* pSceneObjInfos = si.m_vObject.at( i );
		LoadSceneObject( pSceneObjInfos, this );
	}
	m_pEntityManager->SetPerso( m_pEntityManager->GetEntity( si.m_sPersoName ) );
}

void CScene::Load( string sFileName )
{
	ILoader::CSceneInfos si;
	m_oLoaderManager.Load( sFileName, si );
	Load( si );
}

void CScene::Export( string sFileName )
{
	ILoader::CSceneInfos si;
	GetInfos( si );
	m_oLoaderManager.Export( sFileName, si );
}

void CScene::Clear()
{
	int nChildCount = (int)m_vChild.size();
	for( int i = 0; i < nChildCount; i++ )
	{
		IEntity* pChild = static_cast< IEntity* >( m_vChild[ 0 ] );
		pChild->Unlink();
		m_pEntityManager->DestroyEntity( pChild );
	}
	m_pRessource = NULL;
	m_pEntityManager->Clear();
}

float CScene::GetHeight( float x, float z )
{
	if( m_nHeightMapID != -1 )
		return m_oCollisionManager.GetMapHeight( m_nHeightMapID, x, z );
	return -1000000.f;
}