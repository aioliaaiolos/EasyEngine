#include <windows.h>
#include "IStorage.h"
#include <sstream>
#include <iomanip>
#include <errno.h>

using namespace std;

IFileStorage::IFileStorage():
m_pFile( NULL )
{
}


void IFileStorage::CloseFile()
{
	fclose( m_pFile );
	m_pFile = NULL;
}

IFileStorage::~IFileStorage()
{
	if( m_pFile )
	{
		fclose( m_pFile );
		m_pFile = NULL;
	}
}

bool CBinaryFileStorage::OpenFile( string sFileName, TOpenMode mode )
{
	if( m_pFile )
		fclose( m_pFile );
	string sMode;
	if (mode == eRead)
		sMode = "r";
	else if (mode == eWrite)
		sMode = "w";
	else if (mode == eAppend)
		sMode = "a";
	else if (mode == eReadWrite)
		sMode = "r+";
	sMode += "b";
	m_pFile = fopen( sFileName.c_str(), sMode.c_str() );
	return m_pFile != NULL;
}

void CBinaryFileStorage::Seek(int offset)
{
	fseek(m_pFile, offset, SEEK_CUR);
}

IBaseStorage& CBinaryFileStorage::operator<<( int i )
{
	fwrite( &i, sizeof( int ), 1, m_pFile );
	return *this;
}

IBaseStorage& CBinaryFileStorage::operator << (bool b)
{
	int i = b ? 1 : 0;
	fwrite(&i, sizeof(int), 1, m_pFile);
	return *this;
}

IBaseStorage& CBinaryFileStorage::operator<<( unsigned int i )
{
	fwrite( &i, sizeof( unsigned int ), 1, m_pFile );
	return *this;
}

IBaseStorage& CBinaryFileStorage::operator<<( float f )
{
	fwrite( &f, sizeof( float ), 1, m_pFile );
	return *this;
}

IBaseStorage& CBinaryFileStorage::operator<<( string s )
{
	int nSize = (int)s.size();
	fwrite( &nSize, sizeof( int ), 1, m_pFile );
	fwrite( s.c_str(), sizeof( char ), s.size(), m_pFile );
	return *this;
}

IBaseStorage& CBinaryFileStorage::operator<<(char* sz)
{
	int nSize = strlen(sz);
	fwrite(&nSize, sizeof(int), 1, m_pFile);
	fwrite(sz, sizeof(char), nSize, m_pFile);
	return *this;
}

const IBaseStorage& CBinaryFileStorage::operator >> (char& c) const
{
	fread(&c, sizeof(char), 1, m_pFile);
	return *this;
}

const IBaseStorage& CBinaryFileStorage::operator>>( int& i ) const
{
	fread( &i, sizeof( int ), 1, m_pFile );
	return *this;
}

const IBaseStorage& CBinaryFileStorage::operator >> (bool& b) const
{
	int i = 0;
	fread(&i, sizeof(int), 1, m_pFile);
	b = (i > 0);
	return *this;
}

IBaseStorage& CBinaryFileStorage::operator>>( unsigned int& i )
{
	fread( &i, sizeof( unsigned int ), 1, m_pFile );
	return *this;
}

const IBaseStorage& CBinaryFileStorage::operator>>( float& f ) const
{
	fread( &f, sizeof( float ), 1, m_pFile );
	return *this;
}

const IBaseStorage& CBinaryFileStorage::operator>>( string& s ) const
{
	int nSize;
	fread( &nSize, sizeof( int ), 1, m_pFile );
	s.resize( nSize );
	fread( &s[0], sizeof( char ), nSize, m_pFile );
	return *this;
}

IBaseStorage& CBinaryFileStorage::operator >> (char* sz)
{
	int nSize;
	fread(&nSize, sizeof(int), 1, m_pFile);
	sz = new char[nSize + 1];
	fread(sz, sizeof(char), nSize, m_pFile);
	sz[nSize] = 0;
	return *this;
}

IBaseStorage& CBinaryFileStorage::operator<<( const IPersistantObject& object )
{
	object >> *this;
	return *this;
}

const IBaseStorage& CBinaryFileStorage::operator>>( IPersistantObject& object ) const
{
	object << *this;
	return *this;
}


CAsciiFileStorage::CAsciiFileStorage():
m_nWidth( 10 ),
m_dCap( 0. ),
m_nPrecision( 7 ),
m_bDisplayVectorInLine( false ),
m_bEnableVectorEnumeration( true )
{
}


bool CAsciiFileStorage::OpenFile(string sFileName, TOpenMode mode)
{
	if (m_pFile)
		fclose(m_pFile);
	string sMode;
	if (mode == eRead)
		sMode = "r";
	else if (mode == eWrite)
		sMode = "w";
	errno = 0;
	fopen_s(&m_pFile, sFileName.c_str(), sMode.c_str());
	//m_pFile = fopen( sFileName.c_str(), sMode.c_str() );
	if (!m_pFile) {
		char msg[64];
		sprintf(msg, "Error %d (%s) \n", errno, strerror(errno));
		OutputDebugStringA(msg);
	}
	return m_pFile != NULL;
}

void CAsciiFileStorage::SetCurrentMapNames( string sKey, string sValue )
{
	m_sCurrentMapKeyName = sKey;
	m_sCurrentMapValueName = sValue;
}

void CAsciiFileStorage::SetWidth( int nWidth )
{
	m_nWidth = nWidth;
}

void CAsciiFileStorage::SetPrecision( int nPrecision )
{
	m_nPrecision = nPrecision;
}

void CAsciiFileStorage::SetCap( double dCap )
{
	m_dCap = dCap;
}

void CAsciiFileStorage::DisplayVectorInLine( bool dDisplayInLine )
{
	m_bDisplayVectorInLine = dDisplayInLine;
}

void CAsciiFileStorage::EnableVectorEnumeration( bool bEnable )
{
	m_bEnableVectorEnumeration = bEnable;
}

int CAsciiFileStorage::GetWidth()
{
	return m_nWidth;
}
	
int	CAsciiFileStorage::GetPrecision()
{
	return m_nPrecision;
}

double CAsciiFileStorage::GetCap()
{
	return m_dCap;
}

IBaseStorage& CAsciiFileStorage::operator<<( int nVal )
{
	//ostringstream oss;
	//oss << nVal;
	CStringStorage ss;
	ss.SetWidth( m_nWidth );
	ss << nVal;
	*this << ss.GetValue();
	return *this;
}

IBaseStorage& CAsciiFileStorage::operator<< (bool b)
{
	CStringStorage ss;
	ss.SetWidth(m_nWidth);
	ss << b;
	*this << ss.GetValue();
	return *this;
}

IBaseStorage& CAsciiFileStorage::operator<<( unsigned int nVal )
{
	//ostringstream oss;
	//oss << nVal;
	CStringStorage ss;
	ss.SetWidth( m_nWidth );
	ss << nVal;
	*this << ss.GetValue();
	return *this;
}
	
IBaseStorage& CAsciiFileStorage::operator<<( float f )
{
	//ostringstream oss;
	//oss << f;
	//fwrite( oss.str().c_str(), sizeof( char ), oss.str().size(), m_pFile );
	CStringStorage ss;
	ss.SetWidth( m_nWidth );
	ss << f;
	*this << ss.GetValue();
	return *this;
}
	
IBaseStorage& CAsciiFileStorage::operator<<( string s )
{
	fwrite( s.c_str(), sizeof( char ), s.size(), m_pFile );
	return *this;
}

IBaseStorage& CAsciiFileStorage::operator<<(char* sz)
{
	fwrite(sz, sizeof(char), strlen(sz), m_pFile);
	return *this;
}

const IBaseStorage& CAsciiFileStorage::operator >> (char& c) const
{
	return *this;
}

const IBaseStorage& CAsciiFileStorage::operator>>( int& ) const
{
	return *this;
}

const IBaseStorage& CAsciiFileStorage::operator >> (bool&) const
{
	return *this;
}

IBaseStorage& CAsciiFileStorage::operator>>( unsigned int& )
{
	return *this;
}
	
const IBaseStorage& CAsciiFileStorage::operator>>( float& ) const
{
	return *this;
}
	
const IBaseStorage& CAsciiFileStorage::operator>>( string& ) const
{
	return *this;
}

IBaseStorage& CAsciiFileStorage::operator >> (char*)
{
	return *this;
}


IBaseStorage& CAsciiFileStorage::operator<<( const IPersistantObject& object )
{
	object >> *this;
	return *this;
}

const IBaseStorage& CAsciiFileStorage::operator>>(IPersistantObject& object) const
{
	object << *this;
	return *this;
}

void CAsciiFileStorage::SetVectorElementsName( string sVectorElementName )
{
	m_sVectorElementName = sVectorElementName;
}

CStringStorage::CStringStorage():
m_dCap( 0. ),
m_nWidth( 10 ),
m_nPrecision( 7 )
{
}

double CStringStorage::GetCaped( double dNumber, double dCap )
{
	if( dNumber > dCap || dNumber < -dCap )
		return dNumber;
	return 0.;
}
	
void CStringStorage::SetWidth( int nWidth )
{
	m_nWidth = nWidth;
}
	
void CStringStorage::SetPrecision( int nPrecision )
{
	m_nPrecision = nPrecision;
}

void CStringStorage::SetCap( double dCap )
{
	m_dCap = dCap;
}

string& CStringStorage::GetValue()
{
	return m_sValue;
}

IBaseStorage& CStringStorage::operator<<( int nValue )
{
	ostringstream oss;
	oss << setw( m_nWidth ) << setprecision( m_nPrecision ) << nValue;
	*this << oss.str();
	return *this;
}

IBaseStorage& CStringStorage::operator<<(bool b)
{
	ostringstream oss;
	oss << setw(m_nWidth) << setprecision(m_nPrecision) << b;
	*this << oss.str();
	return *this;
}

IBaseStorage& CStringStorage::operator<<( unsigned int uValue )
{
	ostringstream oss;
	oss << setw( m_nWidth ) << setprecision( m_nPrecision ) << uValue;
	*this << oss.str();
	return *this;
}

IBaseStorage& CStringStorage::operator<<( float fValue )
{
	ostringstream oss;
	oss << setw( m_nWidth ) << setprecision( m_nPrecision ) << GetCaped( fValue, m_dCap );
	*this << oss.str();
	return *this;
}

IBaseStorage& CStringStorage::operator<<( string sValue )
{
	m_sValue += sValue;
	return *this;
}

IBaseStorage& CStringStorage::operator<<(char* sz)
{
	string s = sz;
	m_sValue += s;
	return *this;
}

const IBaseStorage& CStringStorage::operator >> (char& c) const
{
	return *this;
}

const IBaseStorage& CStringStorage::operator>>( int& ) const
{
	return *this;
}

const IBaseStorage& CStringStorage::operator >> (bool&) const
{
	return *this;
}

IBaseStorage& CStringStorage::operator>>( unsigned int& )
{
	return *this;
}

const IBaseStorage& CStringStorage::operator>>( float& ) const
{
	return *this;
}

const IBaseStorage& CStringStorage::operator>>( string& sValue ) const
{
	sValue = m_sValue;
	return *this;
}

IBaseStorage& CStringStorage::operator>>( char* szValue )
{
	strcpy(szValue, m_sValue.c_str());
	return *this;
}

IBaseStorage& CStringStorage::operator<<( const IPersistantObject& object )
{
	object >> *this;
	return *this;
}

const IBaseStorage& CStringStorage::operator>>(IPersistantObject& object) const
{
	object << *this;
	return *this;
}