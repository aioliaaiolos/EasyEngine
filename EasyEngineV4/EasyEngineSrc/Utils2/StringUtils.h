#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <vector>
#include <string>

using namespace std;

class  CStringUtils
{
public:
	static void  	ExtractFloatFromString( const std::string& sString, std::vector< float >& vFloat, unsigned int nCount );
	static bool	 	IsFloat( char c );
	static bool	 	IsInteger( char c );
	static bool		IsFloat(string s);
	static bool	 	IsInteger(string s);
	static int		FindEndOf( const std::string& sString, const std::string& sWord );
	static void 	GetWordByIndex( const std::string& sString, const unsigned int nIndex, std::string& sRetWord );
	static void		GetExtension( std::string sFileName, std::string& sExtension );
	static void		GetFileNameWithoutExtension( string sFileName, string& sOut );
	static void		GetFolderPathFromCompleteFileName(string sFileName, string& sPath);
	static void		GetShortFileName(string sPathFile, string& sFileName);
	static void		GetFileNameWithExtension(string sFileName, string sExtension, string& output);
	static void		ConvertStringToWString(const std::string& s, std::wstring& w);
	static void		ConvertWStringToString(const std::wstring& w, std::string& s);
	static void		Truncate(string sText, int nMaxCharacterPerLine, vector<string>& vLines, bool bSkipSpace = true);
	static void		DecodeString(string& sIn, string& sOut);
};


#endif // STRINGUTILS_H