#ifndef	ASEMESHLOADER_H
#define ASEMESHLOADER_H

#include <vector>
#include "MeshLoader.h"


typedef std::vector< float > VectorFloat;


class CAseLoader : public IMeshLoader
{
	enum ENUM_ARRAY
	{
		VERTEX_ARRAY,
		FACE_ARRAY
	};		
	
	void				LoadIndexArray( int nIndexCount, std::vector< unsigned int >& vIndexArray );	


public:
						CAseLoader(void);								
	virtual 			~CAseLoader(void);		
	void				Export( const std::string& sFileName, const CChunk& chunk );
	void				Load( string sFileName, ILoader::IRessourceInfos& ri, IFileSystem& ){throw 1;}
	void				Export(string sFileName, ILoader::IRessourceInfos& ri);
};
#endif