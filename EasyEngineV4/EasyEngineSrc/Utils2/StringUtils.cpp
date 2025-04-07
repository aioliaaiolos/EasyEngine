#include "StringUtils.h"
#include <algorithm>

#include <codecvt>
#include <regex>

using namespace std;


void CStringUtils::DecodeString(string& sIn, string& sOut)
{
	sOut = sIn;
	int idx = sIn.find("Ã©");
	while (idx != -1) {
		sOut = sIn.replace(sIn.begin() + idx, sIn.begin() + idx + 2, "é");
		idx = sOut.find("Ã©");
	}
	idx = sIn.find("Ã");
	while (idx != -1) {
		sOut = sIn.replace(sIn.begin() + idx, sIn.begin() + idx + 2, "à");
		idx = sOut.find("Ã");
	}
}

void CStringUtils::ExtractFloatFromString( const string& sString, vector< float >& vFloat, unsigned int nCount )
{
	vFloat.clear();
	size_t nIndiceWord = 0;
	unsigned int nCurrentIndex=0;
	while (nIndiceWord < sString.size() )
	{
		while( ( !IsFloat( sString[ nIndiceWord ] ) ) && ( nIndiceWord < sString.size() ) )
			nIndiceWord++;
		string sSubString;
		while ( IsFloat( sString[ nIndiceWord ] ) )
		{
			sSubString.push_back( sString[ nIndiceWord ] );			
			nIndiceWord++;
		} 		
		float fVal = static_cast<float> ( atof( sSubString.c_str() ) );		
		nIndiceWord++;
		vFloat.push_back( fVal );
		nCurrentIndex++;
		if ( nCount <= nCurrentIndex )
			return;
	}
}


bool CStringUtils::IsFloat(char c)
{
	bool res = false;
	if ( (c == '.') || (c == '-'))
		res = true;
	else
	{
		if (IsInteger(c))
			res = true;
	}
	return res;
}


bool CStringUtils::IsInteger(char c)
{
	return ( (c >= '0') && (c <= '9') );
}

bool CStringUtils::IsInteger(string s)
{
	std::regex r("^\\d+$");
	return std::regex_match(s, r);
}

bool CStringUtils::IsFloat(string s)
{
	std::regex r("^[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?$");
	return std::regex_match(s, r);
}

int CStringUtils::FindEndOf( const string& sString, const string& sWord )
{
	int nRet = (int)sString.find_first_of( sWord );
	if (nRet != -1)	
		nRet += (int)sWord.size();
	return nRet;
}


void CStringUtils::GetWordByIndex( const string& sString, const unsigned int nNumWord, string& sRetWord )
{
	unsigned int nIndexString=0, nIndexRetWord=0, nCurrentNumWord=0;
	bool bBeginByBlank=false;
	do
	{
		if ( ( sString[ nIndexString ] == ' ') || ( sString[ nIndexString ] ==  '	' ) )
		{
			if (nIndexString ==0)
				bBeginByBlank = true;
			else
				bBeginByBlank = false;
			do
			{
				nIndexString++;
			}
			while ( ( sString[ nIndexString ] == ' ' ) || ( sString[ nIndexString ] ==  '	' ) );
			if ( !bBeginByBlank )
				nCurrentNumWord++;			
		}
		
		if (nNumWord == nCurrentNumWord)
		{
			while ( sString[ nIndexString ] != ' ' && sString[ nIndexString ] != '	' )
			{
				sRetWord.push_back( sString[ nIndexString ] );
				nIndexRetWord++;
				nIndexString++;
			}
			return;
		}
		nIndexString ++;
	} 
	while( nIndexString < sString.size() );
}

void CStringUtils::GetExtension( std::string sFileName, std::string& sExtension )
{
	int iDotPos = (int)sFileName.find_last_of( "." );
	string sExt = sFileName.substr( iDotPos + 1, sFileName.size() - iDotPos - 1 );
	sExtension = sExt;
	transform( sExt.begin(), sExt.end(), sExtension.begin(), tolower );
}

void CStringUtils::GetFileNameWithoutExtension( string sFileName, string& sOut )
{
	int nDotPos = sFileName.find_last_of( "." );
	sOut = sFileName.substr( 0, nDotPos );
}

void CStringUtils::ConvertStringToWString(const string& s, wstring& w)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;
	w = converterX.from_bytes(s);
}

void CStringUtils::ConvertWStringToString(const wstring& w, string& s)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;
	s = converterX.to_bytes(w);
}

void CStringUtils::GetFolderPathFromCompleteFileName(string sFileName, string& sPath)
{
	int idx = sFileName.find_last_of('/');
	if (idx == -1) {
		idx = sFileName.find_last_of('\\');
	}
	if (idx != -1) {
		sPath = sFileName.substr(0, idx);
	}
}

void CStringUtils::GetShortFileName(string sPathFile, string& sFileName)
{
	int idx = sFileName.find_last_of('/');
	if (idx == -1) {
		idx = sFileName.find_last_of('\\');
	}
	if(idx >= 0)
		sFileName = sPathFile.substr(idx + 1);
}

void CStringUtils::GetFileNameWithExtension(string sFileName, string sExtension, string& output)
{
	if (sFileName.find(sExtension) == -1)
		output = sFileName + "." + sExtension;
}

void CStringUtils::Truncate(string sText, int nMaxCharacterPerLine, vector<string>& vLines, bool bSkipSpace)
{
	string subText = sText;

	while (!subText.empty()) {
		int nTextIndex = 0;
		int nLastSpaceIndex = 0;
		while (nTextIndex < nMaxCharacterPerLine && nTextIndex < subText.size()) {
			if (subText[nTextIndex] == ' ')
				nLastSpaceIndex = nTextIndex;
			nTextIndex++;
		}
		if (subText[nTextIndex] == ' ' || subText[nTextIndex] == '\0')
			nLastSpaceIndex = nTextIndex;
		string s = subText.substr(0, nLastSpaceIndex);
		if (bSkipSpace) {
			if (s[0] == ' ')
				s = s.substr(1);
		}
		subText = subText.substr(nLastSpaceIndex);
		vLines.push_back(s);
	} 
}