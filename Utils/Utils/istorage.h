#ifndef ISTORAGE_H
#define ISTORAGE_H

#include <string>
#include <vector>
#include <map>
#include <stdio.h>

using namespace std;

class IPersistantObject;

class IBaseStorage
{
public:
	virtual IBaseStorage& operator<<( int ) = 0;
	virtual IBaseStorage& operator<< (bool b) = 0;
	virtual IBaseStorage& operator<<( unsigned int ) = 0;
	virtual IBaseStorage& operator<<( float ) = 0;
	virtual IBaseStorage& operator<<( string ) = 0;
	virtual IBaseStorage& operator<<(char*) = 0;
	virtual IBaseStorage& operator<<( const IPersistantObject& object ) = 0;
	//template< class T >	IBaseStorage& operator<<(const vector< T >& vData) = 0;
	//template< class T1, class T2 > IBaseStorage& operator<<(map< T1, T2 >& m) = 0;
	//template< class T1, class T2 > IBaseStorage& operator<<(pair< T1, T2 >& p) { return *this; }

	virtual const IBaseStorage& operator >> (char&) const = 0;
	virtual const IBaseStorage& operator>>( int& ) const = 0;
	virtual const IBaseStorage& operator>> (bool&) const = 0;
	virtual IBaseStorage& operator>>( unsigned int& ) = 0;
	virtual const IBaseStorage& operator>>( float& ) const = 0;
	virtual const IBaseStorage& operator>>( string& ) const = 0;
	virtual IBaseStorage& operator>> (char*) = 0;
	virtual const IBaseStorage& operator>>( IPersistantObject& object ) const = 0;
	//template< class T > IBaseStorage& operator >> (vector< T >& vData);
	//template< class T1, class T2 > const IBaseStorage& operator >> (map< T1, T2 >& m);
	template< class T1, class T2 > const IBaseStorage& operator >> (pair< T1, T2 >& p) = 0;
	
};

class IFileStorage : public IBaseStorage
{
protected:
	FILE*	m_pFile;
public:
	enum TOpenMode
	{
		eRead = 0,
		eWrite,
		eReadWrite,
		eAppend
	};

	IFileStorage();
	~IFileStorage();
	virtual bool	OpenFile( string sFileName, TOpenMode mode) = 0;
	virtual void	CloseFile();
};

class CBinaryFileStorage : public IFileStorage
{
public:

	bool	OpenFile( string sFileName, TOpenMode mode);
	void Seek(int position);
	IBaseStorage& operator<<( int ) override;
	IBaseStorage& operator << (bool b) override;
	IBaseStorage& operator<<( unsigned int ) override;
	IBaseStorage& operator<<( float ) override;
	IBaseStorage& operator<<( string ) override;
	IBaseStorage& operator<<(char*) override;
	template< class T >	CBinaryFileStorage& operator<<( const vector< T >& vData );
	template< class T1, class T2 > const CBinaryFileStorage& operator<<( const map< T1, T2 >& m );
	template< class T1, class T2 > CBinaryFileStorage& operator<<(const pair< T1, T2 >& p);

	const IBaseStorage& operator >> (char&) const override;
	const IBaseStorage& operator>>( int& ) const override;
	const IBaseStorage& operator>> (bool&) const override;
	IBaseStorage& operator>>( unsigned int& ) override;
	const IBaseStorage& operator>>( float& ) const override;
	const IBaseStorage& operator>>( string& ) const override;
	IBaseStorage& operator >>(char*) override;
	template< class T > const CBinaryFileStorage& operator>>( vector< T >& vData ) const;
	template< class T1, class T2 > const CBinaryFileStorage& operator>>(  map< T1, T2 >& m  ) const;
	template< class T1, class T2 > const CBinaryFileStorage& operator >> (pair< T1, T2 >& p) const;

	template< class T >	void Load( vector< T >& vData, int nCount ) const;

	IBaseStorage&	operator<<( const IPersistantObject& object ) override;
	const IBaseStorage& operator>>( IPersistantObject& object ) const override;
};

class CAsciiFileStorage : public IFileStorage
{
	string	m_sCurrentMapKeyName;
	string	m_sCurrentMapValueName;

	int		m_nWidth;
	double	m_dCap;
	int		m_nPrecision;
	bool	m_bDisplayVectorInLine;
	string	m_sVectorElementName;
	bool	m_bEnableVectorEnumeration;

public:

	CAsciiFileStorage();
	bool	OpenFile( string sFileName, TOpenMode mode );
	void	SetCurrentMapNames( string sKey, string sValue );

	void	SetWidth( int nWidth );
	void	SetPrecision( int nPrecision );
	void	SetCap( double dCap );
	void	SetVectorElementsName( string sVectorElementName );
	void	DisplayVectorInLine( bool dDisplayInLine );
	void	EnableVectorEnumeration( bool bEnable );

	int		GetWidth();
	int		GetPrecision();
	double	GetCap();
	
	IBaseStorage& operator<<(int) override;
	IBaseStorage& operator<<(bool b) override;
	IBaseStorage& operator<<(unsigned int) override;
	IBaseStorage& operator<<(float) override;
	IBaseStorage& operator<<( string ) override;
	IBaseStorage& operator<<(char* sz) override;
	template< class T >	CAsciiFileStorage& operator<<(const vector< T >& vData);
	template< class T1, class T2 > CAsciiFileStorage& operator<<(const map< T1, T2 >& m);
	template< class T1, class T2 > CAsciiFileStorage& operator<<(const pair< T1, T2 >& p);

	const IBaseStorage& operator >> (char&) const override;
	const IBaseStorage& operator>>(int& ) const override;
	const IBaseStorage& operator>>(bool&) const override;
	IBaseStorage& operator>>(unsigned int&) override;
	const IBaseStorage& operator>>(float&) const override;
	const IBaseStorage& operator>>(string&) const override;
	IBaseStorage& operator>>(char* sz) override;
	template< class T >	IBaseStorage& operator>>( vector< T >& vData );

	IBaseStorage&	operator<<( const IPersistantObject& object ) override;
	const IBaseStorage& operator>>( IPersistantObject& object ) const override;
};


class CStringStorage : public IBaseStorage 
{

	string	m_sValue;
	int		m_nWidth;
	int		m_nPrecision;
	double	m_dCap;

	double	GetCaped( double fNumber, double cap );

public:

	void	SetWidth( int nWidth );
	void	SetPrecision( int nPrecision );
	void	SetCap( double dCap );
	string& GetValue();	

	CStringStorage();
	IBaseStorage& operator<<( int );
	IBaseStorage& operator<<(bool);
	IBaseStorage& operator<<( unsigned int );
	IBaseStorage& operator<<( float );
	IBaseStorage& operator<<( string );
	IBaseStorage& operator<<(char*);

	const IBaseStorage& operator >> (char&) const override;
	const IBaseStorage& operator>>( int& ) const;
	const IBaseStorage& operator>>(bool&) const;
	IBaseStorage& operator>>( unsigned int& );
	const IBaseStorage& operator>>( float& ) const;
	const IBaseStorage& operator>>( string& ) const;
	IBaseStorage& operator>>(char*);

	IBaseStorage&	operator<<( const IPersistantObject& object );
	const IBaseStorage& 	operator>>( IPersistantObject& object ) const;

	template< class T >	CStringStorage& operator>>( vector< T >& vData );
	template< class T >	CStringStorage& operator<<( const vector< T >& vData );
	template< class T1, class T2 > const CStringStorage& operator<<( const map< T1, T2 >& m );	
}; 


template< class T >
CBinaryFileStorage& CBinaryFileStorage::operator<<( const vector< T >& vData )
{
	int nCount = (int)vData.size();
	*this << nCount;
	if (nCount > 0) {
		const T& pData = vData.operator[](0);
		fwrite(&pData, sizeof(T), nCount, m_pFile);
	}
	return *this;
}

template< class T1, class T2 > 
const CBinaryFileStorage& CBinaryFileStorage::operator<<(  const map< T1, T2 >& m  )
{
	*this << (int)m.size();
	if( m.size() > 0 )
	{
		for (map<T1, T2>::const_iterator itMap = m.begin(); itMap != m.end(); ++itMap) {
			*this << itMap->first;
			*this << itMap->second;
		}
	}
	return *this;
}


template< class T1, class T2 > 
CBinaryFileStorage& CBinaryFileStorage::operator<<(const pair< T1, T2 >& p)
{
	*this << p.first;
	*this << p.second;
	return *this;
}

template< class T >
const CBinaryFileStorage& CBinaryFileStorage::operator>>( vector< T >& vData ) const
{
	int nCount;
	*this >> nCount;
	if( nCount > 0 )
		Load( vData, nCount );
	return *this;
}

template< class T1, class T2 > 
const CBinaryFileStorage& CBinaryFileStorage::operator>>(  map< T1, T2 >& m  ) const
{
	int nSize = 0;
	*this >> nSize;
	for( int i = 0; i < nSize; i++ )
	{
		T1 key;
		T2 value;
		*this >> key;
		*this >> value;
		m[ key ] = value;
	}
	return *this;
}


template< class T1, class T2 > 
const CBinaryFileStorage& CBinaryFileStorage::operator>>(pair< T1, T2 >& p) const
{
	*this >> p.first;
	*this >> p.second;
	return *this;
}


template< class T >
void CBinaryFileStorage::Load( vector< T >& vData, int nCount ) const
{
	vData.resize( vData.size() + nCount );
	fread( &vData[ 0 ], sizeof( T ), nCount, m_pFile );
}

	
template< class T >
IBaseStorage& CAsciiFileStorage::operator>>( vector< T >& vData )
{
	return *this;
}

template< class T >
CAsciiFileStorage& CAsciiFileStorage::operator<<( const vector< T >& vData )
{
	CStringStorage s;
	if( m_bDisplayVectorInLine )
	{
		s << vData;
		*this << s.GetValue();
	}
	else
	{
		for( unsigned int i = 0; i < vData.size(); i++ )
		{
			*this << m_sVectorElementName;
			if( m_bEnableVectorEnumeration )
				*this << " " << i;
			*this << " = " << vData.at( i ) << "\n";
		}
	}
	return *this;
}

template< class T1, class T2 > 
CAsciiFileStorage& CAsciiFileStorage::operator<<(  const map< T1, T2 >& m  )
{
	for (map<T1, T2>::const_iterator itMap = m.begin(); itMap != m.end(); ++itMap) {
		*this << m_sCurrentMapKeyName << " : " << itMap->first << "    " << m_sCurrentMapValueName << " : ";
		*this << itMap->second << "\n";
	}

	return *this;
}

template< class T1, class T2 > 
CAsciiFileStorage& CAsciiFileStorage::operator<<(const pair< T1, T2 >& p)
{
	return *this;
}

template< class T >	
CStringStorage& CStringStorage::operator<<( const vector< T >& vData )
{
	for( unsigned int i = 0; i < vData.size(); i++ )
		*this << vData.at( i );
	return *this;
}

class IPersistantObject
{
public:
	virtual const IPersistantObject& operator >> (CBinaryFileStorage& store) const = 0;
	virtual IPersistantObject& operator << (const CBinaryFileStorage& store) = 0;
	virtual const IPersistantObject& operator >> (CAsciiFileStorage& store) const = 0;
	virtual IPersistantObject& operator << (const CAsciiFileStorage& store) = 0;
	virtual const IPersistantObject& operator >> (CStringStorage& store) const = 0;
	virtual IPersistantObject& operator << (const CStringStorage& store) = 0;
};

#endif // ISTORAGE_H