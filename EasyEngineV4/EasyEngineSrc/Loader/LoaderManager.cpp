#define LOADERMANAGER_CPP
#include "loadermanager.h"

// stl
#include <map>
#include <algorithm>

// Engine
#include "../utils2/chunk.h"
#include "../utils2/StringUtils.h"
#include "Interface.h"
#include "TextureLoader.h"
#include "AseLoader.h"
#include "ASMELoader.h"
#include "ahmoLoader.h"
#include "LightLoader.h"
#include "BMELoader.h"
#include "BKELoader.h"
#include "BSELoader.h"
#include "ColLoader.h"
#include "BBoxLoader.h"
#include "IFileSystem.h"
#include "IGeometry.h"

using namespace std;

CLoaderManager::CLoaderManager(EEInterface& oInterface):
ILoaderManager(),
m_oFileSystem(static_cast<IFileSystem&>(*oInterface.GetPlugin("FileSystem")))
{
	m_mLoaderByExtension[ "ase" ] = new CAseLoader;
	m_mLoaderByExtension["asav"] = m_mLoaderByExtension["ase"];
	m_mLoaderByExtension[ "ale" ] = new CLightLoader;
	m_mLoaderByExtension[ "bmp" ] = new CBMPLoader;
	m_mLoaderByExtension[ "tga" ] = new CTGALoader;
	m_mLoaderByExtension[ "bme" ] = new CBMELoader(m_oFileSystem, static_cast<IGeometryManager&>(*oInterface.GetPlugin("GeometryManager")));
	m_mLoaderByExtension[ "bke" ] = new CBKELoader(m_oFileSystem);
	m_mLoaderByExtension[ "bse" ] = new CBSELoader(m_oFileSystem);
	m_mLoaderByExtension[ "sav" ] = m_mLoaderByExtension["bse"];
	m_mLoaderByExtension[ "col" ] = new CColLoader(static_cast<IGeometryManager&>(*oInterface.GetPlugin("GeometryManager")));
	m_mLoaderByExtension[ "bbox"] = new CBBoxLoader(m_oFileSystem, static_cast<IGeometryManager&>(*oInterface.GetPlugin("GeometryManager")));
}

CLoaderManager::~CLoaderManager()
{
	map< string, ILoader* >::iterator itLoader = m_mLoaderByExtension.begin();
	for (; itLoader != m_mLoaderByExtension.end(); itLoader++) {
		if(itLoader->first != "asav" && itLoader->first != "sav")
			delete itLoader->second;
	}
}

ILoader* CLoaderManager::GetLoader( std::string sExtension )
{
	string sLowExtension = sExtension;
	transform( sExtension.begin(), sExtension.end(), sLowExtension.begin(), tolower );
	map< string, ILoader* >::iterator itLoader = m_mLoaderByExtension.find( sLowExtension );
	if ( itLoader != m_mLoaderByExtension.end() )
		return itLoader->second;
	else
		return NULL;
}

void CLoaderManager::LoadTexture( string sFileName, ILoader::CTextureInfos& ti )
{
	string sExtension;
	CStringUtils::GetExtension( sFileName, sExtension );
	std::transform( sExtension.begin(), sExtension.end(), sExtension.begin(), tolower );
	if( sExtension == "tga" )
	{
		CTGALoader* pLoader = static_cast< CTGALoader* >(  m_mLoaderByExtension[ "bme" ] );
		pLoader->Load( sFileName, ti, m_oFileSystem );
	}
	else if( sExtension == "bmp" )
	{
		CBMPLoader* pLoader = static_cast< CBMPLoader* >(  m_mLoaderByExtension[ "bmp" ] );
		pLoader->Load( sFileName, ti, m_oFileSystem );
	}
}

void CLoaderManager::Load( string sFileName, ILoader::IRessourceInfos& ri )
{
	string sExtension;
	CStringUtils::GetExtension( sFileName, sExtension );
	map< string, ILoader* >::iterator itLoader = m_mLoaderByExtension.find( sExtension );
	if ( itLoader != m_mLoaderByExtension.end() )
	{
		itLoader->second->Load(sFileName, ri, m_oFileSystem );
		ri.m_sFileName = sFileName;
	}
	else
	{
		string sMessage = "Impossible de charger " + sFileName + " : L'extension \"" + sExtension + "\" n'est pas gérée par le loader";
		ILoaderManager::CBadExtension e( sMessage );
		throw e;
	}
}

void CLoaderManager::Export( string sFileName, ILoader::IRessourceInfos& ri )
{
	string sExtension;
	CStringUtils::GetExtension( sFileName, sExtension );
	map< string, ILoader* >::iterator itLoader = m_mLoaderByExtension.find( sExtension );
	if ( itLoader != m_mLoaderByExtension.end() )
	{
		string sDir;
		m_oFileSystem.GetLastDirectory( sDir );
		string sFilePath;
		string sPrefix = sFileName.substr( 0, 3 );
		if(!CEasyFile::IsAbsolutePath(sFileName))
			sFilePath = sDir + "\\" + sFileName;
		else
			sFilePath = sFileName;
		try
		{
			itLoader->second->Export( sFilePath, ri );
		}
		catch( CFileNotFoundException&  )
		{
			itLoader->second->Export( sFileName, ri );
		}
	}
	else
	{
		string sMessage = "Impossible de charger \"" + sFileName + "\" : L'extension \"" + sExtension + "\" n'est pas gérée par le loader";
		exception e( sMessage.c_str() );
		throw e;
	}
}

void CLoaderManager::ExportAHMO( const string& sFileName, CChunk& oChunk )
{
	//CAHMOLoader::GetInstance()->Export( sFileName.c_str(), oChunk );
}

void CLoaderManager::CreateBMPFromData( const vector< unsigned char >& vData, int nWidth, int nHeight, int nBitPerPixel, string sFileName )
{
	CBMPLoader* pLoader = static_cast< CBMPLoader* >( m_mLoaderByExtension[ "bmp" ] );
	pLoader->WriteBMP(sFileName , vData, nWidth, nHeight, nBitPerPixel);
}

string CLoaderManager::GetName()
{
	return "LoaderManager";
}

extern "C" _declspec(dllexport) ILoaderManager* CreateLoaderManager(EEInterface& oInterface)
{
	return new CLoaderManager(oInterface);
}