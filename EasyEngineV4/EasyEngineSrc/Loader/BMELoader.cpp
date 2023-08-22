#include "BMEloader.h"

// stl
#include <vector>
#include <sstream>
#include <iomanip>

// Engine
#include "IFileSystem.h"
#include "Exception.h"
#include "IGeometry.h"

#include "../Utils2/Chunk.h"
#include "../Utils2/StringUtils.h"


using namespace std;

CBMELoader::CBMELoader( IFileSystem& oFileSystem, IGeometryManager& oGeometryManager ):
m_oFileSystem( oFileSystem ),
m_oGeometryManager( oGeometryManager )
{
}

typedef const TCHAR* (*TotherMessage)();

void CBMELoader::Load(string sFileName, ILoader::IRessourceInfos& ri, IFileSystem& oFileSystem)
{
	if (m_sExportPluginVersion.size() == 0)
	{
		string sVersionFile = "version.ver";
		FILE* pVersionFile = oFileSystem.OpenFile(sVersionFile, "r");
		if (pVersionFile)
		{
			m_sExportPluginVersion.resize(19);
			fread(&m_sExportPluginVersion[0], sizeof(char), m_sExportPluginVersion.size(), pVersionFile);
			fclose(pVersionFile);
		}
		else
		{
			string sMessage = string("Erreur : fichier \"") + sVersionFile + "\" manquant";
			CFileNotFoundException e(sMessage);
			e.m_sFileName = sVersionFile;
			throw e;
		}
	}

	CAnimatableMeshData* pData = dynamic_cast<CAnimatableMeshData*>(&ri);
	if (!pData)
	{
		string sMessage = string("Erreur :  CBMELoader::Load(\"") + sFileName + "\") requiert un conteneur de type CAnimatableMeshData";
		exception e(sMessage.c_str());
		throw e;
	}
	m_nFileOffset = 0;
	FILE* pFile = oFileSystem.OpenFile(sFileName, "rb");
	if (!pFile)
	{
		string sMessage = string("Erreur : fichier \"") + sFileName + "\" manquant";
		CFileNotFoundException e(sMessage);
		e.m_sFileName = sFileName;
		throw e;
	}
	fclose(pFile);
	string sFolder;
	oFileSystem.GetLastDirectory(sFolder);

	CBinaryFileStorage fs;
	if (fs.OpenFile(sFolder + "\\" + sFileName, CBinaryFileStorage::eRead)) {
		fs >> pData->m_sFileVersion;
		if (m_sExportPluginVersion != pData->m_sFileVersion)
		{
			ostringstream oss;
			oss << "\"" << sFileName << "\" : Avertissement : la version de votre fichier est antérieure à la version de l'exporteur MAX";
			pData->m_vMessages.push_back(oss.str());
		}

		LoadSkeleton(fs, *pData);
		LoadBonesBoundingBoxes(fs, pData->m_mBonesBoundingBoxes);
		int nObjectCount;
		fs >> nObjectCount;

		for (int iMesh = 0; iMesh < nObjectCount; iMesh++)
		{
			CMeshInfos mi;
			mi.m_sFileName = sFileName;
			LoadMesh(fs, mi);
			pData->m_vMeshes.push_back(mi);
		}

		fs.CloseFile();																							
	}
}

void CBMELoader::LoadMesh( CBinaryFileStorage& fs, CMeshInfos& mi )
{
	fs >> mi.m_nParentBoneID;
	fs >> mi.m_sName;
	fs >> mi.m_oOrgMaxPosition;
	LoadMaterial( fs, mi.m_oMaterialInfos );
	if( mi.m_oMaterialInfos.m_nID == -1 )
		mi.m_oMaterialInfos.m_nID = 0;

	LoadGeometry( fs, mi );
	mi.m_pBoundingBox = m_oGeometryManager.CreateBox();
	int geomType;
	fs >> geomType;
	if (geomType != IGeometry::eBox) {
		CEException e("Error in CBMELoader::LoadMesh() : Bad file format for '" + mi.m_sFileName + "', box type expected");
		throw e;
	}
	fs >> *mi.m_pBoundingBox;
	LoadVertexWeight( fs, mi );
	LoadKeyBoundingBoxes( fs, mi );
	fs >> mi.m_oPreferedKeyBBox;
	
	int nCanBeIndexed = 0;
	fs >> nCanBeIndexed;
	mi.m_bCanBeIndexed = ( nCanBeIndexed == 1 );
}

void CBMELoader::LoadBonesBoundingBoxes( CBinaryFileStorage& fs, map< int, IBox* >& mBonesBoundingBoxes )
{
	int nBoxesCount = 0;
	fs >> nBoxesCount;
	for( int i = 0; i < nBoxesCount; i++ )
	{
		int nBoneID = 0;
		fs >> nBoneID;
		int geomType;
		fs >> geomType;
		IBox* pBox = m_oGeometryManager.CreateBox();
		fs >> *pBox;
		mBonesBoundingBoxes[ nBoneID ] = pBox;
	}
}

void CBMELoader::LoadGeometry( CBinaryFileStorage& fs, CMeshInfos& mi )
{
	fs >> mi.m_vVertex >> mi.m_vIndex;
	int nMultiMaterial;	
	fs >> nMultiMaterial;
	int buff;
	if( nMultiMaterial == 1 )
	{
		int nFaceCount = mi.m_vIndex.size() / 3;
		fs >> mi.m_vFaceMaterialID;
	}	
	fs >> mi.m_vNormalFace >> mi.m_vNormalVertex;
	if ( mi.m_oMaterialInfos.m_sDiffuseMapName != "NONE" || mi.m_oMaterialInfos.m_bExists)
		fs >> mi.m_vUVVertex >> mi.m_vUVIndex;
}

void CBMELoader::LoadSkeleton( CBinaryFileStorage& fs, CAnimatableMeshData& oData )
{
	int nBoneCount;
	fs >> nBoneCount;
	for ( int iBone = 0; iBone < nBoneCount; iBone++ )
	{
		int nBoneID, nParentID;
		fs >> nBoneID >> nParentID;
		oData.m_mHierarchyBones[ nBoneID ] = nParentID;
		int nBoneNameSize = 0;
		string sBoneName;
		fs >> sBoneName;
		oData.m_mBones[ nBoneID ].first = sBoneName;
		fs >> oData.m_mBones[ nBoneID ].second;
	}
}

void CBMELoader::LoadKeyBoundingBoxes( CBinaryFileStorage& fs, CMeshInfos& mi )
{
	int nAnimationCount;
	fs >> nAnimationCount;
	for( int i = 0; i < nAnimationCount; i++ )
	{
		string sAnimationName;
		fs >> sAnimationName;
		int nBoxCount;
		fs >> nBoxCount;
		for( int i = 0; i < nBoxCount; i++ )
		{
			int iKeyIndex;
			fs >> iKeyIndex;
			int type = 0;
			fs >> type;
			IBox* pBox = m_oGeometryManager.CreateBox();
			fs >> *pBox;
			mi.m_oKeyBoundingBoxes[ sAnimationName ][ iKeyIndex ] = pBox;
		}
	}
}

void CBMELoader::LoadVertexWeight( CBinaryFileStorage& fs, CMeshInfos& mi )
{
	fs >> mi.m_vWeigtedVertexID >> mi.m_vWeightVertex;
}

void CBMELoader::LoadMaterial( CBinaryFileStorage& fs, CMaterialInfos& mi )
{
	int nMat = 0;
	fs >> nMat;
	mi.m_bExists = false;
	if ( nMat == 1 )
	{
		mi.m_bExists = true;
		int nSubMat;
		fs >> nSubMat;
		if( nSubMat == 1 )
			fs >> mi.m_nID;
		fs >> mi.m_vAmbient >> mi.m_vDiffuse >> mi.m_vSpecular >> mi.m_fShininess >> mi.m_sName >> mi.m_sDiffuseMapName;
		//mi.m_fShininess *= 128.f;

		unsigned int nSubMaterialSize;
		fs >> nSubMaterialSize;
		mi.m_vSubMaterials.resize( nSubMaterialSize );
		for( unsigned int iSubMat = 0; iSubMat < nSubMaterialSize; iSubMat++ )
		{
			mi.m_vSubMaterials.resize( iSubMat + 1 );
			LoadMaterial( fs, mi.m_vSubMaterials[ iSubMat ] );
		}
	}
	if ( mi.m_sDiffuseMapName.size() == 0 )
		mi.m_sDiffuseMapName = "NONE";
}

void CBMELoader::Export( string sFileName, IRessourceInfos& ri )
{
	string sExt;
	CStringUtils::GetExtension( sFileName, sExt );
	const CAnimatableMeshData* pData = static_cast< const CAnimatableMeshData* >( &ri );
	
	if( sExt == "bme" )
	{
		CBinaryFileStorage* pStorage = new CBinaryFileStorage;
		if( !pStorage->OpenFile( sFileName, IFileStorage::eWrite ) )
		{
			CFileNotFoundException e( sFileName );
			throw e;
		}
		ExportAnimatableMeshInfos( *pStorage, *pData );
	}
	else {
		CAsciiFileStorage* pStorage = new CAsciiFileStorage;
		if (!pStorage->OpenFile(sFileName, IFileStorage::eWrite))
		{
			CFileNotFoundException e(sFileName);
			throw e;
		}
		ExportAnimatableMeshInfos(*pStorage, *pData);
	}
}

void CBMELoader::ExportAscii( string sFileName, const CAnimatableMeshData& oData )
{
	FILE* pFile = m_oFileSystem.OpenFile( sFileName, "w" );
	ExportSkeletonAscii( oData, pFile );
	for( unsigned int i = 0; i < oData.m_vMeshes.size(); i++ )
	{
		ExportMaterialAscii( oData.m_vMeshes[ i ], pFile );
		ExportGeometryAscii( oData.m_vMeshes[ i ], pFile );
		ExportVertexWeightAscii( oData.m_vMeshes[ i ], pFile );
		
		string sMessage = "\n\n";
		if( oData.m_vMeshes[ i ].m_bCanBeIndexed )
			sMessage += "Le modèle peut être indexé";
		else
			sMessage += "Le modèle ne peut pas être indexé";
		fwrite( sMessage.c_str(), sizeof( char ), sMessage.size(), pFile );
	}
	fclose( pFile );
}
	
void CBMELoader::ExportMaterialAscii( const CMeshInfos& oData, FILE* pOutFile )
{
	string s = "Material\n\n";
	fwrite( s.c_str(), sizeof( char ), s.size(), pOutFile );
}

void CBMELoader::ExportMaterialInfos( const ILoader::CMaterialInfos& mi, CAsciiFileStorage& store )
{
	store << "\n\nMaterial\n\n";
	store.SetWidth( 0 );
	store.DisplayVectorInLine( true );
	store << "Ambient = ( ";
	store << mi.m_vAmbient << " )\nDiffuse = ( ";
	store << mi.m_vDiffuse << " )\nSpecular = ( ";
	store << mi.m_vSpecular << " )";
	store.DisplayVectorInLine( false );
	store << "\nShininess = " << mi.m_fShininess;
	store << "\nDiffuse map = " << mi.m_sDiffuseMapName;
	store << "\nShader = " << mi.m_sShaderName;
}

void CBMELoader::ExportSkinningInfos( const ILoader::CMeshInfos& mi , CAsciiFileStorage& fs )
{
	if( mi.m_vWeigtedVertexID.size() > 0 )
	{
		fs << "\n\nWeight table : \n";
		fs.SetWidth( 20 );
		fs << "\n\t\t\t\tVertex id\t\t\t" << "Weight\t\t\t\t" << "Bone id\n\n";
		for( int i = 0; i < mi.m_vWeigtedVertexID.size(); i++ )
			fs << i / 4 << mi.m_vWeightVertex[ i ] << mi.m_vWeigtedVertexID[ i ] << "\n";
	}
}

void CBMELoader::ExportGeometryAscii( const CMeshInfos& mi, FILE* pOutFile )
{
	string s = "Géométrie\n\n";
	fwrite( s.c_str(), sizeof( char ), s.size(), pOutFile );

	ExportArrayAscii<float>( "Vertex", "Vertex Array", 3, mi.m_vVertex, pOutFile );
	ExportArrayAscii<unsigned int>( "Face", "Index Array", 3, mi.m_vIndex, pOutFile );
	ExportArrayAscii<float>( "Normal face", "Normal face Array", 3, mi.m_vNormalFace, pOutFile );
	ExportArrayAscii<float>( "Normal vertex", "Normal vertex Array", 3, mi.m_vNormalVertex, pOutFile );
	string sMessage = string( "\n\n\nMaterial\n\n " ) + "Texture name : " +  mi.m_oMaterialInfos.m_sDiffuseMapName + "\n";	
	fwrite( sMessage.c_str(), sizeof( char ), sMessage.size(), pOutFile );
}

void CBMELoader::ExportSkeletonAscii( const CAnimatableMeshData& oData, FILE* pOutFile )
{
	if( oData.m_mHierarchyBones.size() > 0 )
	{
		string sMessage = "\n\n\nHierarchy Bones\n\n";
		fwrite( sMessage.c_str(), sizeof( char ), sMessage.size(), pOutFile );
		ostringstream oss;
		
		for( map<int, int>::const_iterator itBone = oData.m_mHierarchyBones.begin(); itBone != oData.m_mHierarchyBones.end(); ++itBone )
			oss << "Parent du bone " << itBone->first << " : bone " << itBone->second << "\n";

		for( map< int, pair< string, CMatrix > >::const_iterator itBone = oData.m_mBones.begin(); itBone != oData.m_mBones.end(); ++itBone )
		{
			oss << "\n\nBone " << itBone->first << "\n";
			vector< float > vData;
			itBone->second.second.Get( vData );
			for( unsigned int i = 0; i < vData.size(); i++ )
			{
				if( i%4 == 0 )
					oss << "\n";
				oss << setprecision(7) << setw(18) << vData[ i ];
			}
		}
		fwrite( oss.str().c_str(), sizeof( char ), oss.str().size(), pOutFile );
	}
	else
	{
		string sMessage = "\n\n\nCe modèle ne contient pas de squelette\n\n";
		fwrite( sMessage.c_str(), sizeof( char ), sMessage.size(), pOutFile );
	}
}

void CBMELoader::ExportSkeleton( const CAnimatableMeshData& oData, CAsciiFileStorage& store )
{
	if( oData.m_mHierarchyBones.size() > 0 )
	{
		store << "\n\n\nHierarchy Bones\n\n";
		store.SetCurrentMapNames( "Parent du bone", "bone" );
		store << oData.m_mHierarchyBones;
		store.SetPrecision( 7 );
		for( map< int, pair< string, CMatrix > >::const_iterator itBone = oData.m_mBones.begin(); itBone != oData.m_mBones.end(); ++itBone )
		{
			store.SetWidth( 0 );
			store << "\n\nBone " << itBone->first << " (" << itBone->second.first << ")" << "\n";
			store.SetWidth( 18 );
			store << itBone->second.second;
		}
	}
	else
		store << "\n\n\nCe modèle ne contient pas de squelette\n\n";
}

void CBMELoader::ExportVertexWeightAscii( const CMeshInfos& mi, FILE* pOutFile )
{
	ExportArrayAscii<float>( "", "Vertex weights", 4, mi.m_vWeightVertex, pOutFile );
	ExportArrayAscii<float>( "", "Weighted vertex IDs", 4, mi.m_vWeigtedVertexID, pOutFile );
}

template<class T>
void CBMELoader::ExportArrayAscii( string sTypeName, string sMessage, int nColCount, const vector< T >& vArray, FILE* pFile )
{
	string sOut;
	SerializeAscii<T>( vArray, sTypeName, nColCount, sOut );
	string sMessageTotal = string( "\n\n\n\n" ) + sMessage + "\n";
	fwrite( sMessageTotal.c_str(), sizeof( char ), sMessageTotal.size(), pFile );
	fwrite( sOut.c_str(), sizeof(char), sOut.size(), pFile );
}

//
//void CBMELoader::ExportMaterialAscii( const CChunk& oInChunk, FILE* pOutFile )
//{
//	ExportArrayAscii<float>( "Ambient", "Ambient ", "Ambient", 4, oInChunk, pOutFile );
//	ExportArrayAscii<float>( "Diffuse", "Diffuse ", "Diffuse", 4, oInChunk, pOutFile );
//	ExportArrayAscii<float>( "Specular", "Specular ", "Specular", 4, oInChunk, pOutFile );
//	ExportArrayAscii<float>( "Shininess", "Shininess ", "Shininess", 1, oInChunk, pOutFile );
//	CMaterialInfos* pMaterialInfo = (CMaterialInfos*)oInChunk.Get( "MaterialInfos" );
//	string sMessage = string( "\n\nShader name : " ) + pMaterialInfo->m_sMaterialName + "\n";
//	fwrite( sMessage.c_str(), sizeof(char), sMessage.size(), pOutFile );
//}
//
//void CBMELoader::ExportGeometryAscii( const CChunk& oInChunk, FILE* pOutFile )
//{
//	ExportArrayAscii<float>( "VertexArray", "Vertex", "Vertex Array", 3, oInChunk, pOutFile );
//	ExportArrayAscii<unsigned int>( "IndexArray", "Face", "Index Array", 3, oInChunk, pOutFile );
//	ExportArrayAscii<float>( "NormalFacesArray", "Normal face", "Normal face Array", 3, oInChunk, pOutFile );
//	ExportArrayAscii<float>( "NormalVertexArray", "Normal vertex", "Normal vertex Array", 3, oInChunk, pOutFile );
//	CMaterialInfos* pMaterialInfo = reinterpret_cast< CMaterialInfos* >( oInChunk.Get( "MaterialInfos" ) );
//	string sMessage = string( "\n\n\nMaterial\n\n " ) + "Texture name : " +  pMaterialInfo->m_sDiffuseMapName + "\n";
//	fwrite( sMessage.c_str(), sizeof( char ), sMessage.size(), pOutFile );
//}
//

//void CBMELoader::ExportVertexWeightAscii( const CChunk& oInChunk, FILE* pOutFile )
//{
//	ExportArrayAscii<float>( "VertexWeight", "", "Vertex weights", 4, oInChunk, pOutFile );
//	ExportArrayAscii<float>( "WeightedVertexID", "", "Weighted vertex IDs", 4, oInChunk, pOutFile );
//}
//
//template<class T>
//void CBMELoader::ExportArrayAscii( string sArrayName, string sTypeName, string sMessage, int nColCount, const CChunk& oChunk, FILE* pFile )
//{
//	string sOut;
//	vector<T>* pArray = static_cast<vector<T>*> ( oChunk.Get(sArrayName) );
//	if( pArray )
//	{
//		SerializeAscii<T>( *pArray, sTypeName, nColCount, sOut );
//		string sMessageTotal = string( "\n\n\n\n" ) + sMessage + "\n";
//		fwrite( sMessageTotal.c_str(), sizeof( char ), sMessageTotal.size(), pFile );
//		fwrite( sOut.c_str(), sizeof(char), sOut.size(), pFile );
//	}
//}

//template<class T>
//void CBMELoader::ExportArrayAscii( string sTypeName, string sMessage, int nColCount, const vector< T >& vArray, FILE* pFile )
//{
//	string sOut;
//	SerializeAscii<T>( vArray, sTypeName, nColCount, sOut );
//	string sMessageTotal = string( "\n\n\n\n" ) + sMessage + "\n";
//	fwrite( sMessageTotal.c_str(), sizeof( char ), sMessageTotal.size(), pFile );
//	fwrite( sOut.c_str(), sizeof(char), sOut.size(), pFile );
//}

template<class T> 
void CBMELoader::SerializeAscii( const vector<T>& oVector, string sTypeName, int nColCount, string& sOut )
{
	int i = 0, n = 0;
	ostringstream oss, ossNumber;
	for( vector<T>::const_iterator itVector = oVector.begin(); itVector != oVector.end(); itVector++,i++ )
	{
		unsigned int nSpace = 0;
		if( i % nColCount == 0 )
		{
			oss << "\n" << sTypeName << " " << n;
			ossNumber << n;
			nSpace = 6 - static_cast< unsigned int >( ossNumber.str().size() - 1 );
			ossNumber.str( "" );
			i = 0;
			n++;
		}		
		for( unsigned int j = 0; j < nSpace; j++ )
			oss << " ";
		oss << setprecision( m_nAsciiExportPrecision ) << setw(20) << *itVector;
	}
	sOut = oss.str();
}

void CBMELoader::ExportAnimatableMeshInfos( CBinaryFileStorage& store, const ILoader::CAnimatableMeshData& mi )
{
	store << mi.m_sFileVersion;
	ExportSkeleton( mi, store );
	ExportBonesBoundingBoxes( mi.m_mBonesBoundingBoxes, store );
	store << (int)mi.m_vMeshes.size();
	for( unsigned int i = 0; i < mi.m_vMeshes.size(); i++ )
		ExportMeshInfos( mi.m_vMeshes[ i ], store );
	store.CloseFile();
}

void CBMELoader::ExportBonesBoundingBoxes( const map< int, IBox* >& m, CBinaryFileStorage& fs )
{
	fs << (int)m.size();
	for( map< int, IBox* >::const_iterator itBox = m.begin(); itBox != m.end(); itBox++ )
		fs << itBox->first << *itBox->second;
}


void CBMELoader::ExportAnimatableMeshInfos( CAsciiFileStorage& store, const ILoader::CAnimatableMeshData& ami )
{
	store << "File version : " << ami.m_sFileVersion << "\n";
	store.SetWidth( 0 );
	ExportSkeleton( ami, store );
	ExportBonesBoundingBoxes( ami.m_mBonesBoundingBoxes, store );
	store << "\n\nMesh count = " << (unsigned int) ami.m_vMeshes.size();

	for( unsigned int i = 0; i < ami.m_vMeshes.size(); i++ )
		ExportMeshInfos( ami.m_vMeshes[ i ], store );
	store.CloseFile();
}

template< class T > 
void ExportVector( const vector< T >& vData, int nComponantCount, string sName, CAsciiFileStorage& fs )
{
	fs.SetWidth( 0 );
	fs << "\n\n" << (int)vData.size() / nComponantCount << " " << sName << "\n\n";
	for( unsigned int i = 0; i < vData.size() / nComponantCount; i++ )
	{
		float n4 = 0;
		if( nComponantCount > 3 )
			n4 = (float)vData.at( nComponantCount * i + 4 );
		
		if (nComponantCount >= 3) {
			CVector v(vData.at(nComponantCount * i), vData.at(nComponantCount * i + 1), vData.at(nComponantCount * i + 2), n4);
			fs << sName << " " << i << " : " << v << "\n";
		}
		else if (nComponantCount == 2) {
			CVector2D v(vData.at(nComponantCount * i), vData.at(nComponantCount * i + 1));
			fs << sName << " " << i << " : " << v << "\n";
		}
		
	}
}

void CBMELoader::ExportMeshInfos( const CMeshInfos& mi, CAsciiFileStorage& fs )
{
	try
	{
		fs.SetWidth( 0 );
		fs << "\n\n\nMesh name = " << mi.m_sName << "\n\nWorldTM = \n" <<mi.m_oOrgMaxPosition;
		fs << "\nParent bone id = " << mi.m_nParentBoneID;
		ExportMaterialInfos( mi.m_oMaterialInfos, fs );

		fs.SetVectorElementsName( "" );
		fs.EnableVectorEnumeration( false );
		ExportVector( mi.m_vVertex, 3, "vertex", fs );
		ExportVector( mi.m_vIndex, 3, "index", fs );

		int nMultiMaterial = 0;
		if( mi.m_bMultiMaterial )
		{
			nMultiMaterial = 1;
			fs << nMultiMaterial;
			fs << mi.m_vFaceMaterialID;
		}
		else
			fs << "\nPas de multi matériaux\n";
		ExportVector( mi.m_vNormalFace, 3, "Face normale", fs );
		ExportVector( mi.m_vNormalVertex, 3, "Vertex normale", fs );

		//if ( mi.m_oMaterialInfos.m_sDiffuseMapName != "NONE" && mi.m_oMaterialInfos.m_bExists )
		{
			ExportVector( mi.m_vUVVertex, 2, "UVVertex", fs );
			ExportVector( mi.m_vUVIndex, 3, "UVIndex", fs );
		}
		fs << "\n\nBounding box du modèle : \n" << *mi.m_pBoundingBox;
		ExportSkinningInfos( mi, fs );
		ExportKeyBoundingBoxesInfos( mi.m_oKeyBoundingBoxes, fs );
		if( mi.m_bCanBeIndexed )
			fs << "Ce modèle peut-être indexé";
		else
			fs << "Ce modèle ne peut pas être indexé";
	}
	catch( exception& e )
	{
		MessageBoxA( NULL, e.what(), "Erreur", MB_ICONERROR );
	}
}

void CBMELoader::ExportBonesBoundingBoxes( const map< int, IBox* >& m, CAsciiFileStorage& fs )
{
	fs.SetWidth( 0 );
	fs << "\n\n" << (int)m.size() << " boites englobantes : \n\n";
	fs.EnableVectorEnumeration( false );
	for( map< int, IBox* >::const_iterator itBox = m.begin(); itBox != m.end(); itBox++ )
		fs << itBox->first << *itBox->second;
}

void CBMELoader::ExportSkeleton( const CAnimatableMeshData& mi, CBinaryFileStorage& fs )
{
	int nBoneCount = (int)mi.m_mBones.size();
	fs << nBoneCount;
	for ( TSkeletonMap::const_iterator itBone = mi.m_mBones.begin(); itBone != mi.m_mBones.end(); itBone++ )
	{
		int nBoneID = itBone->first;
		map< int, int >::const_iterator itHierarchyID = mi.m_mHierarchyBones.find( nBoneID );
		fs << nBoneID << itHierarchyID->second << itBone->second.first;
		fs << itBone->second.second;
	}
}

void CBMELoader::ExportMeshInfos( const ILoader::CMeshInfos& mi, CBinaryFileStorage& fs )
{
	try
	{
		fs << mi.m_nParentBoneID;
		fs << mi.m_sName << mi.m_oOrgMaxPosition;
		ExportMaterialInfos( mi.m_oMaterialInfos, fs );
		fs << mi.m_vVertex << mi.m_vIndex;

		int nMultiMaterial = 0;
		if( mi.m_bMultiMaterial )
		{
			nMultiMaterial = 1;
			fs << nMultiMaterial;
			fs << mi.m_vFaceMaterialID;
		}
		else
			fs << nMultiMaterial;
		fs << mi.m_vNormalFace << mi.m_vNormalVertex;

		if ( mi.m_oMaterialInfos.m_sDiffuseMapName != "NONE" || mi.m_oMaterialInfos.m_bExists )
			fs << mi.m_vUVVertex << mi.m_vUVIndex;
		fs << *mi.m_pBoundingBox;
		ExportSkinningInfos( mi, fs );
		ExportKeyBoundingBoxesInfos( mi.m_oKeyBoundingBoxes, fs );
		fs << mi.m_oPreferedKeyBBox;
		fs << mi.m_bCanBeIndexed;
	}
	catch( exception& e )
	{
		MessageBoxA( NULL, e.what(), "Erreur", MB_ICONERROR );
	}
}

void CBMELoader::ExportMaterialInfos( const ILoader::CMaterialInfos& mi, CBinaryFileStorage& fs )
{
	int nMat = 0;
	if ( !mi.m_bExists )
		fs << nMat;
	else
	{
		nMat = 1;
		fs << nMat;
		if( mi.m_vSubMaterials.size() > 0 )
			fs << 1 << mi.m_nID;
		else
			fs << 1 << mi.m_nID;
		fs << mi.m_vAmbient << mi.m_vDiffuse << mi.m_vSpecular << mi.m_fShininess;
		fs <<  mi.m_sName << mi.m_sDiffuseMapName;
		fs << (unsigned int)mi.m_vSubMaterials.size();

		for( unsigned int iMtl = 0; iMtl < mi.m_vSubMaterials.size(); iMtl++ )
			ExportMaterialInfos( mi.m_vSubMaterials[ iMtl ], fs );
	}
}

void CBMELoader::ExportSkinningInfos( const ILoader::CMeshInfos& mi , CBinaryFileStorage& fs )
{
	fs << mi.m_vWeigtedVertexID << mi.m_vWeightVertex;
}

void CBMELoader::ExportKeyBoundingBoxesInfos( const map< string, map< int, IBox* > >& mKeyBoundingBoxes, CBinaryFileStorage& fs )
{
	int nAnimationCount = (int)mKeyBoundingBoxes.size();
	fs << nAnimationCount;
	if( nAnimationCount > 0 )
	{
		for( map< string, map< int, IBox* > >::const_iterator itAnim = mKeyBoundingBoxes.begin(); itAnim != mKeyBoundingBoxes.end(); itAnim++ )
		{
			fs << itAnim->first;
			const map< int, IBox* >& oKeyBoxes = itAnim->second;
			fs << (unsigned int) oKeyBoxes.size();
			for( map< int, IBox* >::const_iterator itBox = oKeyBoxes.begin(); itBox != oKeyBoxes.end(); itBox++ )
			{
				fs << itBox->first;
				fs << *itBox->second;
			}
		}
	}
}

void CBMELoader::ExportKeyBoundingBoxesInfos( const map< string, map< int, IBox* > >& mKeyBoundingBoxes, CAsciiFileStorage& fs )
{
	fs.SetWidth( 0 );
	int nAnimationCount = (int)mKeyBoundingBoxes.size();
	fs << "Animation count = " << nAnimationCount << "\n";
	if( nAnimationCount > 0 )
	{
		for( map< string, map< int, IBox* > >::const_iterator itAnim = mKeyBoundingBoxes.begin(); itAnim != mKeyBoundingBoxes.end(); itAnim++ )
		{
			fs << "\nName = " << itAnim->first << "\n";
			const map< int, IBox* >& oKeyBoxes = itAnim->second;
			for( map< int, IBox* >::const_iterator itBox = oKeyBoxes.begin(); itBox != oKeyBoxes.end(); itBox++ )
			{
				fs << "Bone id = " << itBox->first << "\n";
				fs << "Box : \n" << *itBox->second;
			}
		}
	}
}