#include "RenderUtils.h"
#include "IRenderer.h"
#include "IShader.h"
#include "ILoader.h"
#include "IEntity.h"
#include "IGUIManager.h"
#include "Interface.h"
#include <algorithm>

void CRenderUtils::DrawBox( const CVector& oMinPoint, const CVector& oDimension, IRenderer& oRenderer )
{
	IShader* pShader = oRenderer.GetShader( "color" );
	pShader->Enable( true );
	oRenderer.DrawBox( oMinPoint, oDimension );
}

void CRenderUtils::ScreenCapture( string sFileName, EEInterface* pInterface, int x, int y, unsigned int w, unsigned int h )
{
	vector< unsigned char > vPixels;

	IRenderer* pRenderer = static_cast<IRenderer*>(pInterface->GetPlugin("Renderer"));
	ISceneManager* pSceneManager = static_cast<ISceneManager*>(pInterface->GetPlugin("SceneManager"));
	pRenderer->BeginRender();
	pSceneManager->GetScene("Game")->Update();
	IGUIManager* pGUIManager = static_cast<IGUIManager*>(pInterface->GetPlugin("GUIManager"));
	ILoaderManager* pLoaderManager = static_cast<ILoaderManager*>(pInterface->GetPlugin("LoaderManager"));

	int width = w; 
	int height = h;
	if( pGUIManager )
		pGUIManager->OnRender();	
	pRenderer->GetResolution( w, h );

	if (width == -1)
		width = w;
	if (height == -1)
		height = h;

	pRenderer->ReadPixels( x, y, width, height, vPixels, IRenderer::T_BGR );
	pRenderer->EndRender();

	ILoader::CTextureInfos ti;
	ti.m_ePixelFormat = ILoader::eRGB;
	ti.m_vTexels.swap( vPixels );
	ti.m_nWidth = width;
	ti.m_nHeight = height;
	if( sFileName.find( ".bmp" ) == -1 )
		sFileName += ".bmp";
	ti.m_sFileName = sFileName;
	pLoaderManager->Export( sFileName, ti );
}

void CRenderUtils::IndexGeometry( vector< unsigned int >& vIndexArray, vector< float >& vVertexArray, vector< unsigned int >& vUVIndexArray,
										 vector< float >& vUVVertexArray, vector< float >& vNormalVertexArray, vector< float >& vIndexedNormalVertexArray,
										 vector< float >& vWeightVertex, vector< float >& vWeigtedVertexID, bool& bHasIsolatedVertex )
{

	// test
	vector< float > vOldUVVertexArray;
	vOldUVVertexArray.resize( vUVVertexArray.size() );
	copy( vUVVertexArray.begin(), vUVVertexArray.end(), vOldUVVertexArray.begin() );

	vector< float > vOldNonIndexedNormal;
	vOldNonIndexedNormal.resize( vNormalVertexArray.size() );
	copy( vNormalVertexArray.begin(), vNormalVertexArray.end(), vOldNonIndexedNormal.begin() );
	// fin test

	CreateIndexedNormalArray( vVertexArray, vIndexArray, vNormalVertexArray, vIndexedNormalVertexArray, vWeightVertex, vWeigtedVertexID,  bHasIsolatedVertex );
	if( vUVIndexArray.size() > 0 )
	{
		map< int, CVector > mUVVertexArray;
		map< int, vector< CVector > > mDuplicatedUV;
		for( int i = 0; i < vIndexArray.size(); i++ )
		{
			int nIndex = vIndexArray[ i ];
			int nUVIndex = vUVIndexArray[ i ];
			CVector oUVVertex = CVector( vUVVertexArray[ 2 * nUVIndex ], vUVVertexArray[ 2 * nUVIndex + 1 ], 0 );
			map< int, CVector >::iterator itUVIndex = mUVVertexArray.find( nIndex );
			if( itUVIndex == mUVVertexArray.end() )
				mUVVertexArray[ nIndex ] = oUVVertex;
			else
			{
				CVector& vTempUV = mUVVertexArray[ nIndex ];
				if( vTempUV != oUVVertex )
				{
					int nNewVertexIndex = (int)vVertexArray.size() / 3;
					for( int j = 0; j < 3; j++ )
					{
						vVertexArray.push_back( vVertexArray[ 3 * nIndex + j ] );
						vIndexedNormalVertexArray.push_back( vIndexedNormalVertexArray[ 3 * nIndex + j ] );
					}
					if( vWeightVertex.size() > 0 )
					{
						for( int j = 0; j < 4; j++ )
						{
							vWeightVertex.push_back( vWeightVertex[ 4 * nIndex + j ] );
							vWeigtedVertexID.push_back( vWeigtedVertexID[ 4 * nIndex + j ] );
						}
					}

					vIndexArray[ i ] = nNewVertexIndex;
					mUVVertexArray[ nNewVertexIndex ] = oUVVertex;

					// on recherche tous les anciens index qui ont le même uv pour les remplacer par le nouvel index
					for( int k = i; k < vIndexArray.size(); k++ )
					{
						if( vIndexArray[ k ] == nIndex && vUVIndexArray[ k ] == nUVIndex )
							vIndexArray[ k ] = nNewVertexIndex;
					}
				}
			}
		}
		vUVVertexArray.clear();
		for( map< int, CVector >::iterator itVec = mUVVertexArray.begin(); itVec != mUVVertexArray.end(); itVec++ )
		{
			vUVVertexArray.push_back( itVec->second.m_x );
			vUVVertexArray.push_back( itVec->second.m_y );
		}

		if( bHasIsolatedVertex )
			return;
		// test
		vector< float > vNonIndexedUVVertex, vNonIndexedUVVertex2;
		CreateNonIndexedVertexArray( vIndexArray, vUVVertexArray, 2, vNonIndexedUVVertex );
		CreateNonIndexedVertexArray( vUVIndexArray, vOldUVVertexArray, 2, vNonIndexedUVVertex2 );
		bool bEqual = equal( vNonIndexedUVVertex.begin(), vNonIndexedUVVertex.end(), vNonIndexedUVVertex2.begin() );
		if( !bEqual )
			MessageBox( NULL, "Les uv ne sont pas egaux", "", MB_ICONERROR );

		vector< float > vNewNonIndexedNormal;
		CreateNonIndexedVertexArray( vIndexArray, vIndexedNormalVertexArray, 3, vNewNonIndexedNormal );
		vector< int > vWrongIndex;
		if( !equal( vOldNonIndexedNormal.begin(), vOldNonIndexedNormal.end(), vNewNonIndexedNormal.begin() ) )
		{
			MessageBoxA( NULL, "Problème avec les normales", "", MB_ICONERROR );
			for( int i = 0; i < vOldNonIndexedNormal.size(); i++ )
			{
				if( vOldNonIndexedNormal[ i ] != vNewNonIndexedNormal[ i ] )
					vWrongIndex.push_back( i );
			}
		}
		// fin test

		std::copy( vIndexArray.begin(), vIndexArray.end(), vUVIndexArray.begin() );
	}
}

void CRenderUtils::CreateIndexedNormalArray( vector< float >& vVertexArray, vector< unsigned int >& vIndexArray, const vector< float >& vNonIndexedNormalArray, vector< float >& vIndexedNormalArray,  
											vector< float >& vWeightVertex, vector< float >& vWeigtedVertexID, bool& bHasIsolatedVertex )
{
	map< int, CVector > mIndexedVertexArray;
	map< int, vector< CVector > > mDuplicatedNormal;
	for( int i = 0; i < vIndexArray.size(); i++ )
	{
		int nIndex = vIndexArray[ i ];
		CVector& oCurrentVector = CVector( vNonIndexedNormalArray[ 3 * i ], vNonIndexedNormalArray[ 3 * i + 1 ], vNonIndexedNormalArray[ 3 * i + 2 ] );		

		map< int, CVector >::iterator itVertex = mIndexedVertexArray.find( nIndex );
		if( itVertex != mIndexedVertexArray.end() )
		{
			if( itVertex->second != oCurrentVector )
			{
				vector< CVector >::iterator itDup = find( mDuplicatedNormal[ nIndex ].begin(), mDuplicatedNormal[ nIndex ].end(), oCurrentVector );
				if( itDup == mDuplicatedNormal[ nIndex ].end() )
				{
					int nNewIndex = (int)vVertexArray.size() / 3;
					//vVertexArray.push_back( vVertexArray[ 3 * nIndex ] );
					//vVertexArray.push_back( vVertexArray[ 3 * nIndex + 1 ] );
					//vVertexArray.push_back( vVertexArray[ 3 * nIndex + 2 ] );
					for( int j = 0; j < 3; j++ )
						vVertexArray.push_back( vVertexArray[ 3 * nIndex + j ] );
					if( vWeightVertex.size() > 0 )
					{
						for( int j = 0; j < 4; j++ )
						{
							vWeightVertex.push_back( vWeightVertex[ 4 * nIndex + j ] );
							vWeigtedVertexID.push_back( vWeigtedVertexID[ 4 * nIndex + j ] );
						}
					}

					vIndexArray[ i ] = nNewIndex;
					mIndexedVertexArray[ nNewIndex ] = oCurrentVector;
					// on recherche tous les anciens index qui ont le même uv pour les remplacer par le nouvel index
					for( int k = i; k < vIndexArray.size(); k++ )
					{
						CVector oVertex = CVector( vNonIndexedNormalArray[ 3 * k ], vNonIndexedNormalArray[ 3 * k + 1 ], vNonIndexedNormalArray[ 3 * k + 2 ] );
						if( ( vIndexArray[ k ] == nIndex ) && ( oVertex == oCurrentVector ) )
							vIndexArray[ k ] = nNewIndex;
					}
				}
			}
		}
		else
			mIndexedVertexArray[ nIndex  ] = oCurrentVector;

		vector< CVector >::iterator itDup = find( mDuplicatedNormal[ nIndex ].begin(), mDuplicatedNormal[ nIndex ].end(), oCurrentVector );
		if( itDup == mDuplicatedNormal[ nIndex ].end() )
			mDuplicatedNormal[ nIndex ].push_back( oCurrentVector );
	}

	int nIsolatedVertexIndex = 0;
	vector< int > vIsolatedVertex;
	for( map< int, CVector >::iterator itVertex = mIndexedVertexArray.begin(); itVertex != mIndexedVertexArray.end(); itVertex++, nIsolatedVertexIndex++ )
	{
		if( nIsolatedVertexIndex == itVertex->first )
		{
			vIndexedNormalArray.push_back( itVertex->second.m_x );
			vIndexedNormalArray.push_back( itVertex->second.m_y );
			vIndexedNormalArray.push_back( itVertex->second.m_z );
		}
		else
		{
			for( int i = 0; i < 2; i ++ )
			{
				vIndexedNormalArray.push_back( itVertex->second.m_x );
				vIndexedNormalArray.push_back( itVertex->second.m_y );
				vIndexedNormalArray.push_back( itVertex->second.m_y );
			}
			vIsolatedVertex.push_back( nIsolatedVertexIndex );
			++nIsolatedVertexIndex ;
		}
	}

	bHasIsolatedVertex = ( vIsolatedVertex.size() > 0 );
	if( bHasIsolatedVertex )
		return;

	// test
	int nValue = 0, nIndex = -1;
	for( int i = 0; i < vIndexArray.size(); i++ )
	{
		if( vIndexArray[ i ] == nValue )
		{
			nIndex = i;
			break;
		}
	}
	vector< float > vTestNonIndexedNormalArray;
	CreateNonIndexedVertexArray( vIndexArray, vIndexedNormalArray, 3, vTestNonIndexedNormalArray );
	bool bEqual = equal( vNonIndexedNormalArray.begin(), vNonIndexedNormalArray.end(), vTestNonIndexedNormalArray.begin() );
	if( !bEqual )
		MessageBox( NULL, "Probleme avec les normales", "", MB_ICONERROR );
	for( int i = 0; i < vNonIndexedNormalArray.size(); i++ )
	{
		if( vNonIndexedNormalArray[ i ] != vTestNonIndexedNormalArray[ i ] )
		{
			int test = 0;
			break;
		}
	}

	
}

void CRenderUtils::CreateNonIndexedVertexArray( const vector< unsigned int >& vIndexArray, const vector< float >& vVertexArray, int nComposantCount, vector< float >& vOutVertexArray )
{
	vOutVertexArray.clear();
	for ( unsigned int i = 0; i < vIndexArray.size() ; i++ )
	{
		int nIndex = vIndexArray[ i ];
		for ( int j = 0; j < nComposantCount; j++ )
			vOutVertexArray.push_back( vVertexArray[ nIndex * nComposantCount + j ] );
	}
}
