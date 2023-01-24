#include "BinaryMeshMaxExporter.h"
#include "IFileSystem.h"
#include "EEPlugin.h"
#include "Utils2/RenderUtils.h"

// stl
#include <algorithm>

// MAX
#include "3dsmaxport.h"


#define BINARYMESHPLUGIN_CLASS_ID	Class_ID(0x85548e0c, 0x4a26450d)

CBinaryMeshMaxExporter* CBinaryMeshMaxExporter::s_pCurrentInstance = NULL;


void* CBinaryMeshMaxExporterClassDesc::Create( BOOL loading )
{
	return new CBinaryMeshMaxExporter;
}

Class_ID CBinaryMeshMaxExporterClassDesc::ClassID() 
{ 
	return BINARYMESHPLUGIN_CLASS_ID; 
}

CBinaryMeshMaxExporter::CBinaryMeshMaxExporter()
{
	m_vExtension.push_back( L"bme" );
	s_pCurrentInstance = this;
}

CBinaryMeshMaxExporter::~CBinaryMeshMaxExporter()
{
	s_pCurrentInstance = NULL;
}

int CBinaryMeshMaxExporter::ExtCount()
{
	return (int)m_vExtension.size();
}

const TCHAR* CBinaryMeshMaxExporter::Ext( int n )
{
	return m_vExtension[n].c_str();
}

INT_PTR CALLBACK CBinaryMeshMaxExporter::ExportDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int nExportSkinningCheck = -1;
	int nExportBoundingBox = -1;
	int nExportBBoxAtKey = -1;
	switch (msg) 
	{
	case WM_INITDIALOG:
		CenterWindow( hWnd, GetParent( hWnd ) );
		nExportSkinningCheck = s_pCurrentInstance->m_bExportSkinning ? BST_CHECKED : BST_UNCHECKED;
		SendMessageA( GetDlgItem(hWnd, IDC_CHECKSKINNING), BM_SETCHECK, nExportSkinningCheck, 0 );
		nExportBoundingBox = s_pCurrentInstance->m_bExportBoundingBox ? BST_CHECKED : BST_UNCHECKED;
		nExportBBoxAtKey = s_pCurrentInstance->m_bExportBBoxAtKey ? BST_CHECKED : BST_UNCHECKED;
		SendMessageA( GetDlgItem( hWnd, IDC_CHECKBOUNDINGBOX ), BM_SETCHECK, nExportBoundingBox, 0 );
		SendMessageA( GetDlgItem( hWnd, IDC_CHECKEXPORTBBOXATKEY ), BM_SETCHECK, nExportBBoxAtKey, 0 );
		break;
	case WM_CLOSE:
		EndDialog( hWnd, 0 );
		break;
	case WM_COMMAND:
		switch( wParam )
		{
		case IDC_CHECKOPENGLCOORD:
			s_pCurrentInstance->m_bOpenglCoord = SendMessageA(GetDlgItem(hWnd, IDC_CHECKOPENGLCOORD), BM_GETCHECK, 0, 0);
			if(s_pCurrentInstance->m_bOpenglCoord)
				s_pCurrentInstance->m_bOpenglCoord2 = SendMessageA(GetDlgItem(hWnd, IDC_CHECKOPENGLCOORD2), BM_SETCHECK, false, 0);
			break;
		case IDC_CHECKOPENGLCOORD2:
			s_pCurrentInstance->m_bOpenglCoord2 = SendMessageA(GetDlgItem(hWnd, IDC_CHECKOPENGLCOORD2), BM_GETCHECK, 0, 0);
			if(s_pCurrentInstance->m_bOpenglCoord2)
				s_pCurrentInstance->m_bOpenglCoord = SendMessageA(GetDlgItem(hWnd, IDC_CHECKOPENGLCOORD), BM_SETCHECK, false, 0);
			break;
		case IDEXPORT:
			s_pCurrentInstance->m_bFlipNormals = SendMessageA( GetDlgItem(hWnd, IDC_CHECKFLIPNORMALS), BM_GETCHECK, 0, 0 );
			s_pCurrentInstance->m_bOpenglCoord = SendMessageA( GetDlgItem(hWnd, IDC_CHECKOPENGLCOORD), BM_GETCHECK, 0, 0 );
			s_pCurrentInstance->m_bOpenglCoord2 = SendMessageA(GetDlgItem(hWnd, IDC_CHECKOPENGLCOORD2), BM_GETCHECK, 0, 0);
			s_pCurrentInstance->m_bExportSkinning = SendMessageA( GetDlgItem(hWnd, IDC_CHECKSKINNING), BM_GETCHECK, 0, 0 );
			s_pCurrentInstance->m_bLog = SendMessageA( GetDlgItem(hWnd, IDC_CHECK_LOG), BM_GETCHECK, 0, 0 );
			s_pCurrentInstance->m_bExportBoundingBox = SendMessageA( GetDlgItem(hWnd, IDC_CHECKBOUNDINGBOX ), BM_GETCHECK, 0, 0 );
			s_pCurrentInstance->m_bExportBBoxAtKey = SendMessageA( GetDlgItem(hWnd, IDC_CHECKEXPORTBBOXATKEY ), BM_GETCHECK, 0, 0 );
			if( s_pCurrentInstance->m_bLog )
			{
				s_pCurrentInstance->m_pLogFile = fopen( "c:\\easyEngineExport.log", "a" );
				SYSTEMTIME st;
				GetSystemTime( &st );
				ostringstream oss;
				oss << "\n-------------------------------------------------------------------------------------\n";
				oss << st.wDay << "/" << st.wMonth << "/" << st.wYear << " " << st.wHour << ":" << st.wMinute << ":" << st.wSecond << "\n";
				fwrite( s_pCurrentInstance->m_pLogFile, sizeof( char ), oss.str().size(), s_pCurrentInstance->m_pLogFile );
			}
			EndDialog( hWnd, 1 );
			break;
		case IDCANCEL:
			EndDialog( hWnd, 0 );
			break;
		}
		break;
	}
	return 0;
}       

int	CBinaryMeshMaxExporter::DoExport(const TCHAR *pName, ExpInterface *ei, Interface *pInterface, BOOL suppressPrompts, DWORD options )
{
	try
	{
		m_pMaxInterface = pInterface;
		wstring wFileName = pName;
		int nDotPos = (int)wFileName.find_last_of( L"." );
		wstring sExtension = wFileName.substr( nDotPos + 1, wFileName.size() - nDotPos - 1 );
		wstring sExtensionLower = sExtension;
		transform( sExtension.begin(), sExtension.end(), sExtensionLower.begin(), tolower );
		sExtension = sExtensionLower;

		map< string, INode* > mBones;
		
		map< int, INode* > mBoneByID;
		INode* pRoot = pInterface->GetRootNode();
		GetSkeleton( pRoot, mBones );
		GetBonesIDByName( pRoot, m_mBoneIDByName );
		GetBoneByID( mBones, m_mBoneIDByName, mBoneByID );
		HWND hMaxDlg = pInterface->GetMAXHWnd();
		INT_PTR iRet = DialogBoxParam( hInstance, MAKEINTRESOURCE( IDD_EXPORT ), hMaxDlg, ExportDlgProc, (LPARAM)this );
		if( iRet == 1 )
		{
			ILoader::CAnimatableMeshData mi;
			StoreSkeletonToSkeletonMap( mBoneByID, mi );
			GetGeometry( pInterface, mi.m_vMeshes, pInterface->GetRootNode() );
			if( !g_bInterruptExport )
			{
				mi.m_mBonesBoundingBoxes = m_mBoneBox;
				string sFileName(wFileName.begin(), wFileName.end());
				if( DumpModels(sFileName, mi ) )
					MessageBox( NULL, L"Export terminé", L"Export", MB_OK );
				if( m_pLogFile )
					fclose( m_pLogFile );
			}
		}
	}
	catch( exception& e )
	{
		MessageBoxA( NULL, e.what(), "", MB_ICONERROR );
	}
	return TRUE;
}

void CBinaryMeshMaxExporter::GetBonesIDByName(INode* pRoot, map< string, int >& mBoneIDByName) const
{
	Object* pObject = pRoot->EvalWorldState(0).obj;
	if (pObject && IsBone(pObject)) {//( pObject->CanConvertToType( Class_ID( BONE_CLASS_ID, 0 ) ) == TRUE || pObject->CanConvertToType( BONE_OBJ_CLASSID ) == TRUE ) )
		wstring wname(pRoot->GetName());
		string name(wname.begin(), wname.end());
		mBoneIDByName[name] = (int)mBoneIDByName.size();
	}
	for( int i = 0; i < pRoot->NumberOfChildren(); i++ )
		GetBonesIDByName( pRoot->GetChildNode( i ), mBoneIDByName );
}

void CBinaryMeshMaxExporter::GetBoneByID( const map< string, INode* >& mBoneByName, const map< string, int >& mBoneIDByName, map< int, INode* >& mBoneByID )
{
	for ( map< string, int >::const_iterator itNameID = mBoneIDByName.begin(); itNameID != mBoneIDByName.end(); ++itNameID )
	{
		const map< string, INode* >::const_iterator itNode = mBoneByName.find( itNameID->first );
		if(itNode != mBoneByName.end())
			mBoneByID[ itNameID->second ] = itNode->second;
		else
			throw CEException("Erreur : CBinaryMeshMaxExporter::GetBoneByID() -> Bone \"" + itNameID->first + "\n introuvable dans la map de bones par noms (mBones)");
	}
}

void CBinaryMeshMaxExporter::StoreMeshToMeshInfos( Interface* pInterface, INode* pMesh, ILoader::CMeshInfos& mi )
{	
	WriteLog( "\nCBinaryMeshMaxExporter::StoreMeshToChunk() : debut" );

	Mesh& oMesh = GetMeshFromNode(pMesh);
	wstring wName(pMesh->GetName());
	string sName(wName.begin(), wName.end());
	mi.m_sName = sName;
	bool bIsTextured = false;

	if( m_bLog )
	{
		string sMessage = "\nCBinaryMeshMaxExporter::StoreMeshToChunk() : objet \"" + mi.m_sName + "\"";
		WriteLog( sMessage );
	}

	Matrix3 oTM = pMesh->GetObjTMAfterWSM(0);

	IWeightTable* pWeightTable = NULL;
	if( m_bExportSkinning )
	{
		pWeightTable =  m_pGeometryManager->CreateWeightTable();
		GetWeightTable( *pWeightTable, m_mBoneIDByName, mi.m_sName );
		pWeightTable->GetArrays( mi.m_vWeightVertex, mi.m_vWeigtedVertexID );
	}


	if( m_bExportBoundingBox && pWeightTable && pWeightTable->GetVertexCount() > 0 )
		GetBonesBoundingBoxes( oMesh, *pWeightTable, oTM, m_mBoneBox );

	oMesh.buildNormals();
	mi.m_nParentBoneID = -1;
	INode* pParent = pMesh->GetParentNode();
	if( pParent && IsBone( pParent ) )
	{
		wstring wName(pParent->GetName());
		string sName(wName.begin(), wName.end());
		map< string, int >::iterator itBone = m_mBoneIDByName.find(sName);
		if( itBone == m_mBoneIDByName.end() )
		{
			CEException e( "Erreur dans l'exporteur, tous les nodes de la hiérarchy ne sont pas récupérés correctement" );
			throw e;
		}
		mi.m_nParentBoneID = itBone->second;
	}

	Mtl* pMaterial = pMesh->GetMtl();
	if ( pMaterial )
	{
		StoreMaxMaterialToMaterialInfos( pMaterial, mi.m_oMaterialInfos );
		if( m_nMaterialCount > 1 ) 
			mi.m_bMultiMaterial = true;
		mi.m_oMaterialInfos.m_bExists = true;
		string sDiffuseMapName;
		bIsTextured = ( mi.m_oMaterialInfos.m_sDiffuseMapName != "NONE" );
	}
	else
		mi.m_oMaterialInfos.m_bExists = false;

	CMatrix mTransform( -1, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1 );
	if(m_bOpenglCoord2)
		mTransform = CMatrix(1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1);
	
	for ( int iVertex = 0; iVertex < oMesh.getNumVerts(); iVertex ++ )
	{
		Point3& oVertex = oTM * oMesh.verts[ iVertex ];
		mi.m_vVertex.push_back( oVertex.x );
		mi.m_vVertex.push_back( oVertex.y );
		mi.m_vVertex.push_back( oVertex.z );
	}

	for ( int iFace = 0; iFace < oMesh.getNumFaces(); iFace++ )
	{
		if( m_bFlipNormals )
		{
			mi.m_vIndex.push_back( oMesh.faces[ iFace ].v[ 1 ] );
			mi.m_vIndex.push_back( oMesh.faces[ iFace ].v[ 0 ] );
			mi.m_vIndex.push_back( oMesh.faces[ iFace ].v[ 2 ] );
		}
		else
		{
			for ( int iIndex = 0; iIndex < 3; iIndex++ )
				mi.m_vIndex.push_back( oMesh.faces[ iFace ].v[ iIndex ] );
		}
	}

	GetFacesMtlArray( oMesh, mi.m_vFaceMaterialID );
	GetNormals( oMesh, mi.m_vNormalFace, mi.m_vNormalVertex );
	if ( bIsTextured )
	{
		Texmap* pTexmap = pMaterial ? pMaterial->GetSubTexmap(1) : NULL;
		BitmapTex *bmt = (BitmapTex*)pTexmap;
		StdUVGen *uv = bmt->GetUVGen();
		int utile = uv->GetUScl(0);
		int vtile = uv->GetVScl(0);
		for ( int iTVertex = 0; iTVertex < oMesh.getNumTVerts(); iTVertex++ )
		{					
			UVVert& oTVert = oMesh.getTVert( iTVertex );
			mi.m_vUVVertex.push_back( oTVert.x * utile );
			mi.m_vUVVertex.push_back( oTVert.y * vtile);
		}				
		for ( int iTFace = 0; iTFace < oMesh.getNumFaces(); iTFace++ )
			for ( int iTIndex = 0; iTIndex < 3; iTIndex++ )
				mi.m_vUVIndex.push_back( oMesh.tvFace[ iTFace ].t[ iTIndex ] );
	}

	vector< float > vIndexedNormal;
	bool bIsolatedVertex = true;
	CRenderUtils::IndexGeometry( mi.m_vIndex, mi.m_vVertex, mi.m_vUVIndex, mi.m_vUVVertex, mi.m_vNormalVertex, 
								vIndexedNormal, mi.m_vWeightVertex, mi.m_vWeigtedVertexID, bIsolatedVertex );
	if( bIsolatedVertex )
	{
		ostringstream oss;
		oss << "Le modèle \"" << mi.m_sName << "\" contient des vertex isolés, voulez-vous continuer l'export ?";
		if( MessageBoxA( NULL, oss.str().c_str(), "Avertissement", MB_ICONWARNING ) == IDCANCEL )
		{
			g_bInterruptExport = true;
			return;
		}
	}

	mi.m_vNormalVertex.clear();
	mi.m_vNormalVertex.resize( vIndexedNormal.size() );
	copy( vIndexedNormal.begin(), vIndexedNormal.end(), mi.m_vNormalVertex.begin() );

	mi.m_pBoundingBox = m_pGeometryManager->CreateBox();
	if( m_bOpenglCoord || m_bOpenglCoord2)
	{
		// Vertex
		for( int i = 0; i < mi.m_vVertex.size() / 3; i++ ) {
			CVector v( mi.m_vVertex[ 3 * i ], mi.m_vVertex[ 3 * i + 1 ], mi.m_vVertex[ 3 * i + 2 ] );
			CVector v2 = mTransform * v;
			mi.m_vVertex[ 3 * i ] = v2.m_x;
			mi.m_vVertex[ 3 * i + 1 ] = v2.m_y;
			mi.m_vVertex[ 3 * i + 2 ] = v2.m_z;
			mi.m_pBoundingBox->AddPoint( v2 );
		}
	}
	else {
		for( int i = 0; i < mi.m_vVertex.size() / 3; i++ ) {
			CVector v( mi.m_vVertex[ 3 * i ], mi.m_vVertex[ 3 * i + 1 ], mi.m_vVertex[ 3 * i + 2 ] );
			mi.m_pBoundingBox->AddPoint( v );
		}
	}

	CVector oBBoxCenter;
	mi.m_pBoundingBox->GetCenter( oBBoxCenter );
	mi.m_oOrgMaxPosition = oBBoxCenter;

#ifdef USE_RELATIVE_MAX_POSITION
	CMatrix oObjLocalTM, oObjLocalTMInv;
	oObjLocalTM.SetPosition( oBBoxCenter.m_x, oBBoxCenter.m_y, oBBoxCenter.m_z );
	oObjLocalTM.GetInverse( oObjLocalTMInv );

	for( int iVertex = 0; iVertex < mi.m_vVertex.size() / 3; iVertex++ ) {
		CVector vOrg = CVector( mi.m_vVertex[ 3 * iVertex ], mi.m_vVertex[ 3 * iVertex + 1 ], mi.m_vVertex[ 3 * iVertex + 2 ] );
		CVector v = oObjLocalTMInv * vOrg;
		mi.m_vVertex[ 3 * iVertex ] = v.m_x;
		mi.m_vVertex[ 3 * iVertex + 1 ] = v.m_y;
		mi.m_vVertex[ 3 * iVertex + 2 ] = v.m_z;
	}
#endif // USE_RELATIVE_MAX_POSITION

	if( m_bOpenglCoord || m_bOpenglCoord2) {
		// index
		for( int i = 0; i < mi.m_vIndex.size() / 3; i++ ) {
			unsigned int nTemp = mi.m_vIndex[ 3 * i ];
			mi.m_vIndex[ 3 * i ] = mi.m_vIndex[ 3 * i + 2 ];
			mi.m_vIndex[ 3 * i + 2 ] = nTemp;
		}

		// Normals
		for( int i = 0; i < mi.m_vNormalVertex.size() / 3; i++ ) {
			CVector n( mi.m_vNormalVertex[ 3 * i ], mi.m_vNormalVertex[ 3 * i + 1 ], mi.m_vNormalVertex[ 3 * i + 2 ] );
			CVector n2 = mTransform * n;
			mi.m_vNormalVertex[ 3 * i ] = n2.m_x;
			mi.m_vNormalVertex[ 3 * i + 1 ] = n2.m_y;
			mi.m_vNormalVertex[ 3 * i + 2 ] = n2.m_z;
		}
	}
	
	mi.m_bCanBeIndexed = m_nMaterialCount <= 1;
	m_nMaterialCount = 0;
	m_bMultipleSmGroup = false;
	WriteLog( "\nCBinaryMeshMaxExporter::StoreMeshToChunk() : fin" );


	if (m_bExportBBoxAtKey && pWeightTable && pWeightTable->GetVertexCount() > 0)
	{
		Box3 box;
		pMesh->EvalWorldState(160).obj->GetDeformBBox(0, box);
		IBox* pBox = m_pGeometryManager->CreateBox();
		CVector dim = CVector(box.pmax.x - box.pmin.x, box.pmax.y - box.pmin.y, box.pmax.z - box.pmin.z);
		CVector vmin(-dim.m_x / 2.f, -dim.m_y / 2.f, -dim.m_z / 2.f);
		pBox->Set(vmin, dim);
		mi.m_oKeyBoundingBoxes["stand"].insert(map< int, IBox* >::value_type(160, pBox));
	}
}

void CBinaryMeshMaxExporter::UpdateVersionFile( string sVersion )
{	
	string sFileVersionPath = "version.ver";
	FILE* pVersionFile = NULL;
	int nDepth = 0;
	do
	{
		pVersionFile = fopen( sFileVersionPath.c_str(), "w" );
		sFileVersionPath = string( "..\\" ) + sFileVersionPath;
		nDepth++;
	}
	while( !pVersionFile && nDepth < 10 );
 	fwrite( sVersion.c_str(), sizeof( char ), sVersion.size(), pVersionFile );
	fclose( pVersionFile );
}

bool CBinaryMeshMaxExporter::DumpModels( const string& sFilePath, ILoader::CAnimatableMeshData& amd )
{
	bool bRet = true;
	wstring wVersion = OtherMessage1();
	string sVersion(wVersion.begin(), wVersion.end());
	UpdateVersionFile( sVersion );
	amd.m_sFileVersion = sVersion;
	m_pLoaderManager->Export( sFilePath, amd );
	return bRet;
}

void CBinaryMeshMaxExporter::StoreSkeletonToSkeletonMap( const map< int, INode* >& mNodeID, ILoader::CAnimatableMeshData& ami )
{
	int nRootCount = 0;
	for ( map< int, INode* >::const_iterator itNode = mNodeID.begin(); itNode != mNodeID.end(); ++itNode )
	{
		ami.m_mBones.insert( ILoader::TSkeletonMap::value_type( itNode->first, pair< string, CMatrix >() ) );
		
		int nParentID = -1;
		INode* pParent = itNode->second->GetParentNode();
		CMatrix mLocal, mParentLocal;

		for ( map< int, INode* >::const_iterator itParent = mNodeID.begin(); itParent != mNodeID.end(); ++itParent )
		{
			if ( pParent == itParent->second )
			{
				nParentID = itParent->first;
				break;
			}
		}

		ami.m_mHierarchyBones[ itNode->first ] = nParentID;
		if ( nParentID == -1 )
			nRootCount++;

		wstring wName(itNode->second->GetName());
		string sName(wName.begin(), wName.end());
		ami.m_mBones[ itNode->first ].first = sName;
		Matrix3 oMaxMatrix = itNode->second->GetNodeTM(0);
		CMatrix oMatrix;
		MaxMatrixToEngineMatrix( oMaxMatrix, ami.m_mBones[ itNode->first ].second );

	}

	if ( nRootCount > 1 )
		MessageBox( NULL, L"Attention, le squelette de votre modèle comporte plus de 1 bone racine.", L"", MB_ICONWARNING );
}

void CBinaryMeshMaxExporter::StoreSkinningToMeshInfos( const IWeightTable& wt, ILoader::CMeshInfos& mi )
{
	int nWeightVertexCount = wt.GetVertexCount();
	for ( int iVertex = 0; iVertex < nWeightVertexCount; iVertex++ )
	{
		map< int, float > mBone;
		if( wt.Get( iVertex, mBone ) )
		{
			for ( map< int, float >::const_iterator itBone = mBone.begin(); itBone != mBone.end(); ++itBone )
			{
				mi.m_vWeigtedVertexID.push_back( itBone->first );
				mi.m_vWeightVertex.push_back( itBone->second );
			}
			for ( int iSupBone = (int)mBone.size(); iSupBone < 4; iSupBone++ )
			{
				mi.m_vWeigtedVertexID.push_back( -1.f );
				mi.m_vWeightVertex.push_back( 0.f );
			}
		}
		else
		{
			CEException e("Erreur : Tous les vertex de votre modèle ne sont pas skinnés");
			throw e;
		}
	}
}