#ifndef LOADERMANAGER_CPP
#error
#endif

#ifndef LOADERMANAGER_H
#define LOADERMANAGER_H

// Engine
#include "ILoader.h"
#include "../Utils2/Chunk.h"

class ILoader;
class IFileSystem;
struct CMeshInfos;

class CLoaderManager : public ILoaderManager
{
	std::map< std::string, ILoader* >	m_mLoaderByExtension;
	IFileSystem&						m_oFileSystem;

public:

	CLoaderManager( const Desc& oDesc );
	virtual ~CLoaderManager();
	void			LoadTexture( string sFileName, ILoader::CTextureInfos& ti );
	void			Load( string sFileName, ILoader::IRessourceInfos& ri );
	void			ExportAHMO( const std::string& sFileName, CChunk& chunk);
	void			Export( string sFileName, const ILoader::IRessourceInfos& ri );
	ILoader*		GetLoader( std::string sExtension );
	void			CreateBMPFromData( const vector< unsigned char >& vData, int nWidth, int nHeight, int nBitPerPixel, string sFileName );
};

extern "C" _declspec(dllexport) ILoaderManager* CreateLoaderManager( const ILoaderManager::Desc& oDesc );

#endif //LOADERMANAGER_H