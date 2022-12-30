#include "CSVReader.h"
#include <sstream>
//#include <string>

using namespace std;

CCSVReader::CCSVReader( char cSeparator ) : m_cSeparator( cSeparator ), m_pFile( NULL )
{
}

CCSVReader::~CCSVReader(void)
{
}

void CCSVReader::OpenFile( string sCVSFileName )
{
	errno_t err = fopen_s( &m_pFile, sCVSFileName.c_str(), "r" );
	if ( !m_pFile )
	{
		CCSVReader::Exception e;
		ostringstream ssMsg;
		ssMsg << "Erreur : Impossible d'ouvrir \""<< sCVSFileName << "\"";
		e.m_sMsg = ssMsg.str();
		throw e;
	}
	m_sFileName = sCVSFileName;
	CalculLineCount();
}

bool CCSVReader::ReadCell( string& sCellValue )
{
	unsigned char c;
	do
	{
		int nReadCount = fread( &c, sizeof( char ), 1, m_pFile );
		if( nReadCount != 1 )
			return false;
		sCellValue.push_back( c );
	}
	while ( ( c != m_cSeparator ) && ( c != '\n' ) && ( c != -52 ) );
	if ( c == -52 )
	{
		return false;
	}

	sCellValue = sCellValue.substr( 0, sCellValue.size() - 1 );
	if ( c == '\n' )
	{
		if ( sCellValue.size() > 0 )
		{
			fseek( m_pFile, -1, SEEK_CUR );
			return true;
		}		
		return false;
	}
	if ( c == 0 )
	{
		CEOFException e;
		throw e;
	}
	return true;
}

void CCSVReader::ReadColumnStates( string sStateName )
{
	char szBuffer[ 32 ];
	string sBuffer;
	memset( szBuffer, 0, 32 );
	fread( szBuffer, sizeof(char), sStateName.size() + 3, m_pFile );
	sBuffer = szBuffer;
	sBuffer = sBuffer.substr( 1, sBuffer.size() - 3 );
	if ( sBuffer != sStateName )
	{
		Exception e;
		e.m_sMsg = string( "Column \"" ) + sStateName + ("\" does not exist in \"tableau lexical.csv\"" );
		throw e;
	}
	char c = szBuffer[ sStateName.size() + 2 ];
	if (  c != m_cSeparator )
		throw 1;
}

void CCSVReader::ReadRow( std::vector< std::string >& vRow, unsigned int nMaxColumn )
{
	unsigned int iColumnNum = 0;
	bool bEnd = false;
	
	try
	{
		while( ( bEnd == false ) && ( iColumnNum < nMaxColumn ) )
		{
			string sCellValue;
			ReadCell( sCellValue );
			int nStateValue = atoi( sCellValue.c_str() );
			vRow.push_back( sCellValue );
			iColumnNum++;			
		}
		NextLine();
	}
	catch( CCSVReader::Exception e )
	{
		bEnd = true;
	}
}

void CCSVReader::ReadRows( const vector< string >& vColumnName, std::vector< std::vector< std::string > >& vRows )
{
	for ( unsigned int i = 0; i < vColumnName.size(); i++ )
		ReadColumnStates( vColumnName[ i ] );
	NextLine();

	bool bEnd = false;
	while( bEnd == false )
	{
		vector< string > vRow;
		try
		{
			ReadRow( vRow, (unsigned int)vColumnName.size() );
			vRows.push_back( vRow );
		}
		catch( CCSVReader::Exception e )
		{
			e = e;
			bEnd = true;
		}
		catch ( CCSVReader::CEOFException e )
		{
			e = e;
			bEnd = true;
		}
	}
}

bool CCSVReader::NextLine()
{
	char c;
	bool bEof = false;
	do
	{
		int nReadCount = fread( &c, 1, 1, m_pFile );
		if( nReadCount == 0 )
			return false;
	}
	while( c != '\n' && !(bEof = feof( m_pFile) ) );
	return !bEof;
}

void CCSVReader::Close()
{
	fclose(m_pFile);
	m_sFileName = "";
}

void CCSVReader::CalculLineCount()
{
	fseek( m_pFile, 0, SEEK_SET );
	m_nLineCount = 0;
	while( NextLine() ) m_nLineCount++;
	fclose( m_pFile );
	m_pFile = NULL;
	fopen_s( &m_pFile, m_sFileName.c_str(), "r" );
	if( !m_pFile )
	{
		string s = string( "Impossible d'ouvrir " ) + m_sFileName + "\"";
		exception e( s.c_str() );
		throw e;
	}
}

int CCSVReader::GetLineCount()const
{
	return m_nLineCount;
}

FILE* CCSVReader::GetFile()
{
	return m_pFile;
}

void CCSVReader::Rewind()
{
	fseek( m_pFile, 0, SEEK_SET );
}