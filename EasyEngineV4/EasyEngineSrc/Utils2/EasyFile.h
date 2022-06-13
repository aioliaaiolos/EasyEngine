#ifndef EAYSYFILE_H
#define EAYSYFILE_H

#include <string>
#include <fstream>

using namespace std;

class IFileSystem;

class  CEasyFile 
{
	FILE*							m_pFile;
	std::string						m_sName;
	bool							m_bOpen;
	bool							m_bEof;



public:
	
	CEasyFile();									
	virtual							~CEasyFile();
	void							Open( const std::string& sName, IFileSystem& oFileSystem );
	void		 					GetLine( std::string& sBuff,char End = '\n');	
	void 							SetPointerNext( const std::string& sWord );	
	void 							GetLineNext( const std::string& sKeyWord, std::string& sBuffer );	
	void							GetWord(char szBuffer[], unsigned int nWordLen );
	bool 							Eof();
	void 							operator++();			
	void 							Close();
	void							Reopen( IFileSystem& oFileSystem );
	void							GetName( std::string& sName );
	static bool						IsAbsolutePath(string sPath);

	static void 					_SearchFile( std::string sFileName, const char* szDir, std::string& szFoundDir, bool& bFound );
	static void 					SearchFile(const char* szRoot="",const char* szFileName="", std::string& szFoundDir = std::string(""));
	static std::string				GetValidDirectoryName(const char* szDirectoryName);
};


#endif