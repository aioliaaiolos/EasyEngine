#ifndef RESSOURCE_MANAGER_
#define RESSOURCE_MANAGER_H

#define RESSOURCEMANAGER_CPP
#include "RessourceManager.h"

// Engine
#include "../Utils2/EasyFile.h"
#include "../Utils2/Dimension.h"
#include "../Utils2/StringUtils.h"

// Ressources
#include "Mesh.h"
#include "AnimatableMesh.h"
#include "Animation.h"
#include "Material.h"
#include "Texture.h"
#include "Light.h"
#include "CollisionMesh.h"
#include "Exception.h"

// Interfaces
#include "Interface.h"
#include "ILoader.h"
#include "IGeometry.h"
#include "ICollisionManager.h"
#include "IFileSystem.h"

// stl
#include <algorithm>

using namespace std;

CRessourceManager::CRessourceManager(EEInterface& oInterface) :
m_oInterface(oInterface),
m_nLightCount(0),
m_oLoaderManager(*static_cast<ILoaderManager*>(oInterface.GetPlugin("LoaderManager"))),
m_oRenderer(*static_cast<IRenderer*>(oInterface.GetPlugin("Renderer"))),
m_oGeometryManager(*static_cast<IGeometryManager*>(oInterface.GetPlugin("GeometryManager"))),
m_pCurrentRenderer( NULL ),
m_pDrawTool( NULL ),
m_bCatchException( true ),
m_pEntityManager(nullptr),
m_pCollisionManager(nullptr)
{
	m_mRessourceCreation[ "ase" ] = CreateMesh;
	m_mRessourceCreation[ "ame" ] = CreateMesh;
	m_mRessourceCreation[ "bme" ] = CreateMesh;
	m_mRessourceCreation[ "bke" ] = CreateAnimation;
	m_mRessourceCreation[ "ale" ] = CreateLight;
	m_mRessourceCreation[ "tga" ] = CreateTexture;
	m_mRessourceCreation[ "bmp" ] = CreateTexture;
	m_mRessourceCreation[ "col" ] = CreateCollisionMesh;
}


//-------------------------------------------------------------------------------------------
//										destructor
//-------------------------------------------------------------------------------------------
CRessourceManager::~CRessourceManager(void)
{
}

bool CRessourceManager::IsCatchingExceptionEnabled()
{
	return m_bCatchException;
}

void CRessourceManager::EnableCatchingException( bool bEnable )
{
	m_bCatchException = bEnable;
}

ITexture* CRessourceManager::CreateTexture2D( IShader* pShader, int nUnitTexture, vector< unsigned char >& vData, int nWidth, int nHeight, IRenderer::TPixelFormat eFormat )
{
	CTexture2D::CDesc oDesc(m_oRenderer, pShader, nUnitTexture );
	oDesc.m_vTexels.insert( oDesc.m_vTexels.begin(), vData.begin(), vData.end() );
	oDesc.m_nWidth = nWidth;
	oDesc.m_nHeight = nHeight;
	oDesc.m_eFormat = eFormat;
	oDesc.m_nUnitTexture = nUnitTexture;
	CTexture2D* pTexture = new CTexture2D( oDesc );
	return pTexture;
}

ITexture* CRessourceManager::CreateTexture2D(string sFileName, bool bGenerateMipmaps)
{
	CTexture2D::CDesc desc(m_oRenderer, NULL, 0);
	CreateTextureDesc(sFileName, desc);
	desc.m_bGenerateMipmaps = bGenerateMipmaps;
	CTexture2D* pTexture = new CTexture2D(desc);
	return static_cast< ITexture* > (pTexture);
}

//-------------------------------------------------------------------------------------------
//										_GetRessource
//-------------------------------------------------------------------------------------------
IRessource* CRessourceManager::GetRessource( const string& sRessourceFileName, bool bDuplicate ) 
{	
	IRessource* pRessource = NULL;
	if ( m_mRessource.count( sRessourceFileName ) > 0 && !bDuplicate )
	{
		map< string, IRessource* >::iterator itFind = m_mRessource.find( sRessourceFileName );
		pRessource = itFind->second;		
	}
	else
	{
		pRessource = GetRessourceByExtension(sRessourceFileName);
		if ( pRessource )
		{
			m_mRessource[ sRessourceFileName ] = pRessource;
			pRessource->SetFileName( sRessourceFileName );
		}
	}
	return pRessource;
}

ITestMesh* CRessourceManager::GetTestRessource( const std::string& sRessourceFileName, ITestShaderManager& oTestShaderManager)
{
	string sExtension;
	CStringUtils::GetExtension( sRessourceFileName, sExtension );
	map< string, TTestRessourceCreation >::iterator itRessourceCreation = m_mTestRessourceCreation.find( sExtension );
	if( itRessourceCreation == m_mTestRessourceCreation.end() )
	{
		string sMessage = string( "\"" ) + sRessourceFileName + "\" n'existe pas ou n'est pas prise en charge par le gestionnaire de ressources";
		exception e( sMessage.c_str() );
		throw e;
	}
	ITestMesh* pRessource = itRessourceCreation->second( sRessourceFileName, this, oTestShaderManager, m_oRenderer );
	return pRessource;
}

IRessource*	CRessourceManager::CreateMaterial( ILoader::CMaterialInfos& mi, ITexture* pAlternative )
{
	IShader* pShader = m_oRenderer.GetShader( mi.m_sShaderName );
	CMaterial::Desc oDesc(m_oRenderer, pShader );
	oDesc.m_fShininess = mi.m_fShininess;
	if( !pAlternative && !mi.m_sDiffuseMapName.empty())
		oDesc.m_pDiffuseTexture = static_cast< ITexture* >( GetRessource(mi.m_sDiffuseMapName) );
	else
		oDesc.m_pDiffuseTexture = pAlternative;
	oDesc.m_sName = mi.m_sFileName;
	std::copy( mi.m_vAmbient.begin(), mi.m_vAmbient.end(), oDesc.m_vAmbient.begin() );
	std::copy( mi.m_vDiffuse.begin(), mi.m_vDiffuse.end(), oDesc.m_vDiffuse.begin() );
	std::copy( mi.m_vEmissive.begin(), mi.m_vEmissive.end(), oDesc.m_vEmissive.begin() );
	std::copy( mi.m_vSpecular.begin(), mi.m_vSpecular.end(), oDesc.m_vSpecular.begin() );
	return new CMaterial( oDesc );
}

IRessource* CRessourceManager::GetRessourceByExtension( string sRessourceFileName)
{
	string sExtension;
	CStringUtils::GetExtension( sRessourceFileName, sExtension );
	map< string, TRessourceCreation >::iterator itRessourceCreation = m_mRessourceCreation.find( sExtension );
	if( itRessourceCreation == m_mRessourceCreation.end() )
	{
		string sMessage = string( "\"" ) + sRessourceFileName + "\" n'existe pas ou n'est pas prise en charge par le gestionnaire de ressources";
		CExtensionNotFoundException e( sMessage.c_str() );
		throw e;
	}
	IRessource* pRessource = itRessourceCreation->second( sRessourceFileName, this, m_oRenderer );
	return pRessource;
}

int CRessourceManager::GetLightCount()
{
	return m_nLightCount;
}

IRessource* CRessourceManager::CreateMesh( string sFileName, CRessourceManager* pRessourceManager, IRenderer& oRenderer)
{
	ILoader::CAnimatableMeshData oData;
	oData.m_sFileName = sFileName;
	pRessourceManager->m_oLoaderManager.Load( sFileName, oData );
	return pRessourceManager->CreateMesh(oData);
}

IAnimatableMesh* CRessourceManager::CreateMesh( ILoader::CAnimatableMeshData& oData, IRessource* pMaterial )
{
	IRessource::Desc oResDesc( m_oRenderer, NULL );
	CAnimatableMesh* pAMesh = new CAnimatableMesh( oResDesc );
	IBone* pSkeleton = LoadSkeleton( oData );
	pAMesh->SetSkeleton( pSkeleton );
	for( unsigned int i = 0; i < oData.m_vMeshes.size(); i++ )
	{
		ILoader::CMeshInfos& mi = oData.m_vMeshes[ i ];

		vector< float > vFinalUVVertexArray;
		vector< unsigned int > vFinalUVIndexArray;
		if ( mi.m_vUVIndex.size() > 0 )
		{
			vFinalUVIndexArray.resize( mi.m_vUVIndex.size() );
			memcpy( &vFinalUVIndexArray[ 0 ], &mi.m_vUVIndex[ 0 ], mi.m_vUVIndex.size() * sizeof( unsigned int ) );
			vFinalUVVertexArray.resize( mi.m_vUVVertex.size() );
			memcpy( &vFinalUVVertexArray[ 0 ], &mi.m_vUVVertex[ 0 ], mi.m_vUVVertex.size() * sizeof( float ) );
		}
		IShader* pShader = NULL;	
		string sShaderName;

		CMesh::Desc oDesc( mi.m_vVertex, mi.m_vIndex, vFinalUVVertexArray, vFinalUVIndexArray, mi.m_vNormalVertex, m_oRenderer, pShader );
		oDesc.m_vNormalFaceArray.swap( mi.m_vNormalFace );
		oDesc.m_vNormalVertexArray.swap( mi.m_vNormalVertex );
		oDesc.m_pBbox = mi.m_pBoundingBox;

		if ( mi.m_vWeightVertex.size() > 0 )
		{
			pShader = m_oRenderer.GetShader( "skinning" );
			oDesc.m_vVertexWeight.resize( mi.m_vWeightVertex.size() );
			memcpy( &oDesc.m_vVertexWeight[ 0 ], &mi.m_vWeightVertex[ 0 ], mi.m_vWeightVertex.size() * sizeof( float ) );
			oDesc.m_vWeightedVertexID.resize( mi.m_vWeigtedVertexID.size() );
			memcpy( &oDesc.m_vWeightedVertexID[ 0 ], &mi.m_vWeigtedVertexID[ 0 ], mi.m_vWeigtedVertexID.size() * sizeof( int ) );
		}
		oDesc.m_bIndexed = mi.m_bCanBeIndexed;

		if( !pShader )
		{
			pShader = m_oRenderer.GetShader( mi.m_sShaderName );
			if( !pShader )
				pShader = m_oRenderer.GetShader( "perpixellighting" );
		}
		
		if ( !pMaterial )
			CollectMaterials( mi.m_oMaterialInfos, m_oRenderer, pShader, this, oDesc.m_mMaterials );
		else
			oDesc.m_mMaterials[ 0 ] = static_cast< CMaterial* >( pMaterial );
		if (pMaterial && !static_cast< CMaterial* >(pMaterial)->GetShader())
			pMaterial->SetShader(pShader);
		if( oData.m_bMultiMaterialActivated )
			oDesc.m_vFaceMaterialID.swap( mi.m_vFaceMaterialID );

		if (mi.m_oPreferedKeyBBox.size() == 0) {
			string animationName = "stand";
			map<string, map<int, IBox*>>::iterator it = mi.m_oKeyBoundingBoxes.find(animationName);
			if (it != mi.m_oKeyBoundingBoxes.end()) {
				map<int, IBox*>::iterator itBox = it->second.begin();
				oDesc.m_mAnimationKeyBox[animationName] = itBox->second;
			}
		}

		for( map< string, int >::iterator itPrefAnim = mi.m_oPreferedKeyBBox.begin(); itPrefAnim != mi.m_oPreferedKeyBBox.end(); itPrefAnim++ )
		{
			map< string, map< int, IBox* > >::const_iterator itAnimation = mi.m_oKeyBoundingBoxes.find( itPrefAnim->first );
			if( itAnimation != mi.m_oKeyBoundingBoxes.end() )
			{
				map< int, IBox* >::const_iterator itBone = itAnimation->second.find( itPrefAnim->second );
				if( itBone != itAnimation->second.end() )
				{
					IBox* pBox = itBone->second;
					string sAnimNameLow = itPrefAnim->first;
					std::transform( itPrefAnim->first.begin(), itPrefAnim->first.end(), sAnimNameLow.begin(), tolower );
					oDesc.m_mAnimationKeyBox[ sAnimNameLow ] = pBox;
				}
			}
		}
		
		oDesc.m_pShader = pShader;
		oDesc.m_nParentBoneID = mi.m_nParentBoneID;
		oDesc.m_sName = mi.m_sName;
		oDesc.m_sFileName = mi.m_sFileName;
		oDesc.m_oOrgMaxPosition = mi.m_oOrgMaxPosition;
		
		CMesh* pMesh = new CMesh( oDesc );
		pAMesh->AddMesh( pMesh );
	}
	return pAMesh;
}

IMesh* CRessourceManager::CreatePlane(int slices, int size, string diffuseTexture)
{
	ILoader::CMeshInfos mi;
	float quadSize = (float)size / (float)slices;
	for (int l = 0; l < slices; l++) {
		float z =  (float)l * quadSize - 1.f/2.f * size;
		float zuv = (float)l * quadSize;

		for (int c = 0; c < slices; c++) {
			float x = (float)c * quadSize - 1.f / 2.f * size;
			float xuv = (float)c * quadSize;

			mi.m_vVertex.push_back(x + quadSize);
			mi.m_vVertex.push_back(0);
			mi.m_vVertex.push_back(z);

			mi.m_vNormalVertex.push_back(0.f);
			mi.m_vNormalVertex.push_back(1.f);
			mi.m_vNormalVertex.push_back(0.f);

			mi.m_vUVVertex.push_back((xuv + quadSize) / size);
			mi.m_vUVVertex.push_back(zuv / size);

			mi.m_vVertex.push_back(x);
			mi.m_vVertex.push_back(0);
			mi.m_vVertex.push_back(z);

			mi.m_vNormalVertex.push_back(0.f);
			mi.m_vNormalVertex.push_back(1.f);
			mi.m_vNormalVertex.push_back(0.f);

			mi.m_vUVVertex.push_back(xuv / size);
			mi.m_vUVVertex.push_back(zuv / size);

			mi.m_vVertex.push_back(x + quadSize);
			mi.m_vVertex.push_back(0);
			mi.m_vVertex.push_back(z + quadSize);

			mi.m_vNormalVertex.push_back(0.f);
			mi.m_vNormalVertex.push_back(1.f);
			mi.m_vNormalVertex.push_back(0.f);

			mi.m_vUVVertex.push_back((xuv + quadSize) / size);
			mi.m_vUVVertex.push_back((zuv + quadSize) / size);

			mi.m_vVertex.push_back(x);
			mi.m_vVertex.push_back(0);
			mi.m_vVertex.push_back(z + quadSize);

			mi.m_vNormalVertex.push_back(0.f);
			mi.m_vNormalVertex.push_back(1.f);
			mi.m_vNormalVertex.push_back(0.f);

			mi.m_vUVVertex.push_back(xuv / size);
			mi.m_vUVVertex.push_back((zuv + quadSize) / size);
		}
	}

	for (int l = 0; l < slices; l++) {
		for (int c = 0; c < slices; c++) {
			int offset = 4 * c + 4 * l * slices;
			mi.m_vIndex.push_back(offset + 3);			
			mi.m_vIndex.push_back(offset + 2);
			mi.m_vIndex.push_back(offset + 0);

			mi.m_vIndex.push_back(offset + 0);			
			mi.m_vIndex.push_back(offset + 1);
			mi.m_vIndex.push_back(offset + 3);
			
			mi.m_vNormalFace.push_back(0.f);
			mi.m_vNormalFace.push_back(0.f);
			mi.m_vNormalFace.push_back(1.f);

			mi.m_vNormalFace.push_back(0.f);
			mi.m_vNormalFace.push_back(0.f);
			mi.m_vNormalFace.push_back(1.f);

			mi.m_vUVIndex.push_back(offset + 3);			
			mi.m_vUVIndex.push_back(offset + 2);
			mi.m_vUVIndex.push_back(offset + 0);

			mi.m_vUVIndex.push_back(offset + 0);			
			mi.m_vUVIndex.push_back(offset + 1);
			mi.m_vUVIndex.push_back(offset + 3);
		}
	}

	mi.m_bCanBeIndexed = true;
	mi.m_pBoundingBox = m_oGeometryManager.CreateBox();
	mi.m_pBoundingBox->Set(CVector(-(float)size / 2.f, 0.f, -(float)size / 2.f), CVector(size, 0, size));

	mi.m_oMaterialInfos.m_sDiffuseMapName = diffuseTexture;

	mi.m_oMaterialInfos.m_vAmbient.push_back(1.f);
	mi.m_oMaterialInfos.m_vAmbient.push_back(1.f);
	mi.m_oMaterialInfos.m_vAmbient.push_back(0.f);
	mi.m_oMaterialInfos.m_vAmbient.push_back(1.f);

	mi.m_oMaterialInfos.m_vDiffuse.push_back(1.f);
	mi.m_oMaterialInfos.m_vDiffuse.push_back(1.f);
	mi.m_oMaterialInfos.m_vDiffuse.push_back(0.f);
	mi.m_oMaterialInfos.m_vDiffuse.push_back(1.f);

	mi.m_oMaterialInfos.m_vSpecular.push_back(0.f);
	mi.m_oMaterialInfos.m_vSpecular.push_back(0.f);
	mi.m_oMaterialInfos.m_vSpecular.push_back(0.f);
	mi.m_oMaterialInfos.m_vSpecular.push_back(0.f);
	mi.m_oMaterialInfos.m_nID = 0;
	mi.m_oMaterialInfos.m_fShininess = 1.f;
	

	ILoader::CAnimatableMeshData ami;
	ami.m_bMultiMaterialActivated = false;
	ami.m_vMeshes.push_back(mi);
	IAnimatableMesh* pAMesh = CreateMesh(ami, nullptr);
	IMesh* pMesh = pAMesh->GetMesh(0);
	pMesh->SetFileName(diffuseTexture);
	return pMesh;
}


IRessource* CRessourceManager::CreateCollisionMesh(string sFileName, CRessourceManager* pRessourceManager, IRenderer& oRenderer)
{
	ILoader::CCollisionModelInfos cmi;
	pRessourceManager->m_oLoaderManager.Load(sFileName, cmi);
	CCollisionMesh::Desc oDesc(oRenderer, cmi);
	return new CCollisionMesh(oDesc);
}

IBone* CRessourceManager::LoadSkeleton( ILoader::CAnimatableMeshData& oData )
{
	IBone* pRoot = NULL;
	if (!m_pEntityManager)
		m_pEntityManager = static_cast<IEntityManager*>(m_oInterface.GetPlugin("EntityManager"));
	if ( oData.m_mHierarchyBones.size() > 0 )
	{
		map< int, IBone* > mBone;
		map< int, int >::iterator itHierarchyBone = oData.m_mHierarchyBones.begin();
		for (  itHierarchyBone; itHierarchyBone != oData.m_mHierarchyBones.end(); ++itHierarchyBone )
		{
			int nBoneID = itHierarchyBone->first;
			IBone* pBone = m_pEntityManager->CreateBone();
			pBone->SetID( nBoneID );
			pBone->SetName( oData.m_mBones[ nBoneID ].first );
			CMatrix& m = oData.m_mBones[ itHierarchyBone->first ].second;
			pBone->SetWorldMatrix( m );
			map< int, IBox* >::const_iterator itBone = oData.m_mBonesBoundingBoxes.find( nBoneID );
			if( itBone != oData.m_mBonesBoundingBoxes.end() )
				pBone->SetBoundingBox( itBone->second );
			mBone[ itHierarchyBone->first ] = pBone;
		}
		for (  itHierarchyBone = oData.m_mHierarchyBones.begin(); itHierarchyBone != oData.m_mHierarchyBones.end(); ++itHierarchyBone )
		{
			if ( itHierarchyBone->second == -1 )
				pRoot = mBone[ itHierarchyBone->first ];
			else
			{
				IBone* pBone = mBone[ itHierarchyBone->first ];
				IBone* pParent = mBone[ itHierarchyBone->second ];
				pBone->Link( pParent );
			}
		}
		pRoot->SetLocalTMByWorldTM();
	}
	return pRoot;
}

void CRessourceManager::PopErrorMessage( string& sMessage )
{
	if( m_vErrorMessage.size() > 0 )
	{
		sMessage = m_vErrorMessage.back();
		m_vErrorMessage.pop_back();
	}
}

void CRessourceManager::DestroyAllRessources()
{
	for( map< string, IRessource* >::iterator itRessource = m_mRessource.begin(); itRessource != m_mRessource.end(); itRessource++ )
	{
		delete itRessource->second;
	}
	m_mRessource.clear();
}

ITexture* CRessourceManager::CreateRenderTexture(int width, int height, string sShaderName)
{
	unsigned int nTextureId, nFBOId;
	m_oRenderer.CreateFrameBufferObject(width, height, nFBOId, nTextureId);

	IShader* pShader = m_oRenderer.GetShader(sShaderName);
	CTexture2D* pTexture = NULL;	
	CTexture2D::CDesc desc(m_oRenderer, pShader, 0);
	desc.m_nWidth = width;
	desc.m_nHeight = height;
	desc.m_eFormat = IRenderer::T_RGB;
	desc.m_sName = "Map render texture";
	desc.m_nUnitTexture = 3;
	desc.m_bGenerateMipmaps = false;
	desc.m_bRenderTexture = true;
	desc.m_nTextureId = nTextureId;
	desc.m_nFrameBufferObjectId = nFBOId;
	pTexture = new CTexture2D(desc);
	return pTexture;
}

string CRessourceManager::GetName()
{
	return "RessourceManager";
}

void CRessourceManager::RemoveAllLights()
{
	CLight::RemoveAllLights();
}

void CRessourceManager::Reset()
{
	m_mRessource.clear();
}

void CRessourceManager::CollectMaterials( const ILoader::CMaterialInfos& oMaterialInfos, IRenderer& oRenderer, IShader* pShader, IRessourceManager* pRessourceManager, std::map< int, CMaterial* >& mMaterials )
{
	mMaterials[ oMaterialInfos.m_nID ] = CreateMaterial( &oMaterialInfos, oRenderer, pShader, pRessourceManager );
	for ( unsigned int i = 0; i < oMaterialInfos.m_vSubMaterials.size(); i++ )
		CollectMaterials( oMaterialInfos.m_vSubMaterials[ i ], oRenderer, pShader, pRessourceManager, mMaterials );

}

CMaterial* CRessourceManager::CreateMaterial( const ILoader::CMaterialInfos* pMaterialInfos, IRenderer& oRenderer, IShader* pShader, IRessourceManager* pRessourceManager )
{
	CMaterial::Desc oMatDesc( oRenderer, pShader );
	if ( pMaterialInfos && pMaterialInfos->m_vAmbient.size() > 0 )
	{
		oMatDesc.m_vAmbient.resize( pMaterialInfos->m_vAmbient.size() );
		memcpy( &oMatDesc.m_vAmbient[ 0 ], &pMaterialInfos->m_vAmbient[ 0 ], pMaterialInfos->m_vAmbient.size() * sizeof( float ) );		
		oMatDesc.m_vDiffuse.resize( pMaterialInfos->m_vDiffuse.size() );
		memcpy( &oMatDesc.m_vDiffuse[ 0 ], &pMaterialInfos->m_vDiffuse[ 0 ], pMaterialInfos->m_vDiffuse.size() * sizeof( float ) );
		if( pMaterialInfos->m_vEmissive.size() > 0 )
		{
			oMatDesc.m_vEmissive.resize( pMaterialInfos->m_vEmissive.size() );
			memcpy( &oMatDesc.m_vEmissive[ 0 ], &pMaterialInfos->m_vEmissive[ 0 ], pMaterialInfos->m_vEmissive.size() * sizeof( float ) );
		}
		oMatDesc.m_fShininess = pMaterialInfos->m_fShininess;
		oMatDesc.m_vSpecular.resize( pMaterialInfos->m_vSpecular.size() );
		memcpy( &oMatDesc.m_vSpecular[ 0 ], &pMaterialInfos->m_vSpecular[ 0 ], pMaterialInfos->m_vSpecular.size() * sizeof( float ) );

		if ( pMaterialInfos->m_sDiffuseMapName != "NONE" && pMaterialInfos->m_sDiffuseMapName != "")
		{
			ITexture* pTexture = static_cast< ITexture* > ( pRessourceManager->GetRessource( pMaterialInfos->m_sDiffuseMapName) );
			if ( !pTexture )
			{
				string sMessage = string( "Texture " ) + pMaterialInfos->m_sDiffuseMapName + " not found";
				exception e( sMessage.c_str() );
				throw e;
			}
			pTexture->SetShader( pShader );
			oMatDesc.m_pDiffuseTexture = pTexture;
		}		
	}
	return new CMaterial( oMatDesc );
}

IRessource* CRessourceManager::CreateAnimation( string sFileName, CRessourceManager* pRessourceManager, IRenderer& oRenderer )
{	
	ILoader::CAnimationInfos ai;
	pRessourceManager->m_oLoaderManager.Load( sFileName, ai );
	
	CAnimation::Desc oRessourceDesc ( oRenderer, NULL );
	CAnimation* pAnimation = new CAnimation( oRessourceDesc );
	pAnimation->SetStartAnimationTime( ai.m_nStartTime );
	pAnimation->SetEndAnimationTime( ai.m_nEndTime );
	pAnimation->SetName( ai.m_sName );
	
	unsigned int iKeyIndex = 0;
	for ( int i = 0; i < ai.m_nBoneCount; i++ )
	{
		unsigned int nBoneID = ai.m_vBonesIDArray[ i ];
		pAnimation->AddBone( nBoneID);
		int nKeyCount = ai.m_vKeyCountArray[ i ];
		for ( int iKey = 0; iKey < nKeyCount; iKey++ )
		{
			int nTimeValue = ( int )ai.m_vTimeValueArray[ iKeyIndex ];
			int nKeyType = ( int )ai.m_vKeyTypeArray[ iKeyIndex ];
			pAnimation->AddKey( nBoneID, nTimeValue, (CKey::TKey)nKeyType, ai.m_vLocalTMArray[ iKeyIndex ], ai.m_vWorldTMArray[ iKeyIndex ], ai.m_vLocalQuatArray[ iKeyIndex ] );
			iKeyIndex ++;
		}
	}
	return static_cast< IAnimation* > ( pAnimation );
}


void CRessourceManager::CreateTextureDesc(string sFileName, CTexture2D::CDesc& desc)
{
	ILoader::CTextureInfos ti;
	try {
		m_oLoaderManager.LoadTexture(sFileName, ti);
	}
	catch (CFileNotFoundException& e) {
		string sPrefix = "Textures";
		if (sFileName.empty() || (sFileName[0] != '/' && sFileName[0] != '\\') )
			sPrefix += "/";
		sFileName = sPrefix + sFileName;
		m_oLoaderManager.LoadTexture(sFileName, ti);
	}
	ti.m_sFileName = sFileName;
	IRenderer::TPixelFormat format = IRenderer::T_FormatNone;
	switch (ti.m_ePixelFormat)
	{
	case ILoader::eRGB:
		format = IRenderer::T_RGB;
		break;
	case ILoader::eRGBA:
		format = IRenderer::T_RGBA;
		break;
	case ILoader::eBGR:
		format = IRenderer::T_BGR;
		break;
	case ILoader::eBGRA:
		format = IRenderer::T_BGRA;
		break;
	default:
	{
		ostringstream oss;
		oss << "Error : \"" << sFileName << "\" : Bad Texture format";
		CRessourceException e(oss.str());
		throw e;
	}
	}
	
	desc.m_nWidth = ti.m_nWidth;
	desc.m_nHeight = ti.m_nHeight;
	desc.m_eFormat = format;
	desc.m_vTexels.swap(ti.m_vTexels);
	desc.m_sFileName = sFileName;
	desc.m_nUnitTexture = 3;
}

IRessource* CRessourceManager::CreateTexture( string sFileName, CRessourceManager* pRessourceManager, IRenderer& oRenderer)
{
	CTexture2D::CDesc desc(oRenderer, NULL, 0);
	pRessourceManager->CreateTextureDesc(sFileName, desc);
	CTexture2D* pTexture = new CTexture2D( desc );
	return static_cast< ITexture* > ( pTexture );
}

IRessource* CRessourceManager::CreateLight( CVector Color, IRessource::TLight type, float fIntensity)
{
	CLight::Desc oDesc( m_oRenderer, NULL );
	oDesc.Color = Color;
	oDesc.fIntensity = fIntensity;
	oDesc.type = type;
	return new CLight( oDesc );
}

void CRessourceManager::SetLightIntensity( IRessource* pRessource, float fIntensity )
{
	CLight* pLight = dynamic_cast< CLight* >( pRessource );
	pLight->SetIntensity( fIntensity );
}

float CRessourceManager::GetLightIntensity( IRessource* pRessource )
{
	CLight* pLight = dynamic_cast< CLight* >( pRessource );
	return pLight->GetIntensity();
}

CVector CRessourceManager::GetLightColor( IRessource* pRessource )
{
	CLight* pLight = dynamic_cast< CLight* >( pRessource );
	return pLight->GetColor();
}

IRessource* CRessourceManager::CreateLight( string sFileName, CRessourceManager* pRessourceManager, IRenderer& oRenderer )
{
	ILoader::CLightInfos li;
	li.m_sFileName = sFileName;
	pRessourceManager->m_oLoaderManager.Load( sFileName, li );
	CLight::TLight type;
	switch( li.m_eLightType )
	{
	case ILoader::CLightInfos::eDirectionnelle:
		type = CLight::DIRECTIONAL;
		break;
	case ILoader::CLightInfos::eOmni:
		type = CLight::OMNI;
		break;
	case ILoader::CLightInfos::eTarget:
		type = CLight::SPOT;
		break;
	}
	CLight::Desc desc( oRenderer, NULL );
	desc.type = type;
	desc.Color = CVector( li.m_oColor.m_x, li.m_oColor.m_y, li.m_oColor.m_z, li.m_oColor.m_w );
	desc.fIntensity = li.m_fIntensity;
	CLight* pLight = new CLight( desc );
	pRessourceManager->m_nLightCount++;
	return pLight;
}

IRessource::TLight	CRessourceManager::GetLightType( IRessource* pRessource )
{
	CLight* pLight = dynamic_cast< CLight* >( pRessource );
	return pLight->GetType();
}

void CRessourceManager::DisableLight( IRessource* pRessource )
{
	CLight* pLight = dynamic_cast< CLight* >( pRessource );
	pLight->Disable();
}

void CRessourceManager::SetCurrentRenderer( IRenderer* pRenderer )
{
	m_pCurrentRenderer = pRenderer;
}

IMesh* CRessourceManager::CreatePlane2(int slices, int size, float height, string heightTexture, string diffuseTexture)
{
	if (!m_pCollisionManager)
		m_pCollisionManager = static_cast<ICollisionManager*>(m_oInterface.GetPlugin("CollisionManager"));
	IBox* pBox = m_oGeometryManager.CreateBox();
	pBox->Set(CVector(-size / 2.f, -height /2.f, -size / 2.f), CVector(size, height, size));
	int hmID = m_pCollisionManager->LoadHeightMap(heightTexture, pBox);
	IHeightMap* pHeightMap = m_pCollisionManager->GetHeightMap(hmID);
	ILoader::CMeshInfos mi;
	float quadSize = (float)size / (float)slices;

	for (int l = 0; l < slices + 1; l++) {
		for (int c = 0; c < slices + 1; c++) {
			mi.m_vVertex.push_back(c * quadSize - size / 2);
			mi.m_vVertex.push_back(0);
			mi.m_vVertex.push_back(l * quadSize - size / 2);

			mi.m_vUVVertex.push_back((float)c * quadSize / size);
			mi.m_vUVVertex.push_back((float)l * quadSize / size);
		}
	}

	for (int l = 0; l < slices; l++) {
		for (int c = 0; c < slices; c++) {
			mi.m_vIndex.push_back(l * (slices + 1) + c + 1);
			mi.m_vIndex.push_back((l + 1) * (slices + 1) + c);
			mi.m_vIndex.push_back((l + 1) * (slices + 1) + c + 1);

			mi.m_vIndex.push_back(l * (slices + 1) + c + 1);
			mi.m_vIndex.push_back(l * (slices + 1) + c);
			mi.m_vIndex.push_back((l + 1) * (slices + 1) + c);

			mi.m_vUVIndex.push_back(l * (slices + 1) + c + 1);
			mi.m_vUVIndex.push_back((l + 1) * (slices + 1) + c);
			mi.m_vUVIndex.push_back((l + 1) * (slices + 1) + c + 1);

			mi.m_vUVIndex.push_back(l * (slices + 1) + c + 1);
			mi.m_vUVIndex.push_back(l * (slices + 1) + c);
			mi.m_vUVIndex.push_back((l + 1) * (slices + 1) + c);
		}
	}

	ComputeNormals(mi, slices, pHeightMap);
	
	mi.m_bCanBeIndexed = true;
	mi.m_pBoundingBox = m_oGeometryManager.CreateBox();
	mi.m_pBoundingBox = pBox;

	mi.m_oMaterialInfos.m_sDiffuseMapName = diffuseTexture;

	mi.m_oMaterialInfos.m_vAmbient.push_back(1.f);
	mi.m_oMaterialInfos.m_vAmbient.push_back(1.f);
	mi.m_oMaterialInfos.m_vAmbient.push_back(1.f);
	mi.m_oMaterialInfos.m_vAmbient.push_back(1.f);

	mi.m_oMaterialInfos.m_vDiffuse.push_back(1.f);
	mi.m_oMaterialInfos.m_vDiffuse.push_back(1.f);
	mi.m_oMaterialInfos.m_vDiffuse.push_back(1.f);
	mi.m_oMaterialInfos.m_vDiffuse.push_back(1.f);

	mi.m_oMaterialInfos.m_vSpecular.push_back(0.f);
	mi.m_oMaterialInfos.m_vSpecular.push_back(0.f);
	mi.m_oMaterialInfos.m_vSpecular.push_back(0.f);
	mi.m_oMaterialInfos.m_vSpecular.push_back(0.f);
	mi.m_oMaterialInfos.m_nID = 0;
	mi.m_oMaterialInfos.m_fShininess = 1.f;
	mi.m_oMaterialInfos.m_bExists = true;

	ILoader::CAnimatableMeshData ami;
	ami.m_bMultiMaterialActivated = false;
	ami.m_vMeshes.push_back(mi);

	IFileSystem* pFileSystem = static_cast<IFileSystem*>(m_oInterface.GetPlugin("FileSystem"));
	string root;
	pFileSystem->GetLastDirectory(root);
	WIN32_FIND_DATA wfd;
	string tmpFolder = "/tmp";
	string tmpPath = root + tmpFolder;
	HANDLE hLevelFolder = pFileSystem->FindFirstFile_EE(tmpPath, wfd);
	bool folderCreated = hLevelFolder != INVALID_HANDLE_VALUE;
	if ((!folderCreated) || !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
		folderCreated = CreateDirectoryA(tmpPath.c_str(), nullptr);
	}

	m_oLoaderManager.Export("tmp/ground.bme", ami);

	IAnimatableMesh* pAMesh = CreateMesh(ami, nullptr);
	IMesh* pMesh = pAMesh->GetMesh(0);
	pMesh->SetFileName(diffuseTexture);
	return pMesh;
}

void CRessourceManager::ComputeNormals(ILoader::CMeshInfos& mi, int slices, IHeightMap* pHeightMap)
{
	// Compute face normals
	int num = 0;
	map<int, vector<CVector>> mIndexToNormals;
	for (int l = 0; l < slices; l++) {
		for (int c = 0; c < slices; c++) {
			for (int tri = 0; tri < 2; tri++) {
				vector<int> indices;
				vector<CVector> face;
				for (int i = 0; i < 3; i++) {
					int idx = mi.m_vIndex[num++];
					indices.push_back(idx);
					float vx = mi.m_vVertex[3 * idx];
					float vz = mi.m_vVertex[3 * idx + 2];
					float vy = pHeightMap->GetHeight(vx, vz);
					CVector v = CVector(vx, vy, vz);
					face.push_back(v);
				}
				CVector t = face[1] - face[0];
				CVector b = face[2] - face[0];
				CVector n = (t ^ b);
				n.Normalize();
				mi.m_vNormalFace.push_back(n.m_x);
				mi.m_vNormalFace.push_back(n.m_y);
				mi.m_vNormalFace.push_back(n.m_z);
				for (int k = 0; k < indices.size(); k++)
					mIndexToNormals[indices[k]].push_back(n);
			}
		}
	}

	// compute vertex normals
	mi.m_vNormalVertex.resize(mi.m_vVertex.size());
	for (map<int, vector<CVector>>::iterator it = mIndexToNormals.begin(); it != mIndexToNormals.end(); it++) {
		CVector n;
		for (int i = 0; i < it->second.size(); i++) {
			n += it->second[i];
		}
		n.Normalize();
		mi.m_vNormalVertex[3 * it->first] = n.m_x;
		mi.m_vNormalVertex[3 * it->first + 1] = n.m_y;
		mi.m_vNormalVertex[3 * it->first + 2] = n.m_z;
	}
}

extern "C" _declspec(dllexport) IRessourceManager* CreateRessourceManager(EEInterface& oInterface)
{
	return new CRessourceManager(oInterface);
}


#endif	//RESSOURCE_MANAGER_H