#include "Vector.h"
#include "matrix.h"

// stl
#include <exception>

using namespace std;

CVector::CVector(void):
m_x(0),
m_y(0),
m_z(0),
m_w(1)
{
}


CVector::CVector(float x, float y, float z, float w)
{
	m_x = x;
	m_y = y;
	m_z = z;
	m_w = w;
}

CVector::CVector(float x, float y, float z )
{
	m_x = x;
	m_y = y;
	m_z = z;
	m_w = 1.f;
}

CVector::CVector(const CVector& v)
{
	m_x = v.m_x;
	m_y = v.m_y;
	m_z = v.m_z;
	m_w = v.m_w;
}


CVector::CVector(float TabVal[])
{
	m_x = TabVal[0];
	m_y = TabVal[1];
	m_z = TabVal[2];
	m_w = TabVal[3];
}


CVector::~CVector()
{

}




//Produit scalaire
float CVector::operator *(const CVector& v) const
{
	return (m_x*v.m_x + m_y*v.m_y + m_z*v.m_z + m_w*v.m_w);
}



CVector CVector::operator *(const float d) const
{
	return CVector(m_x*d,m_y*d,m_z*d,m_w*d);
}

CVector CVector::operator *(const CMatrix& mat) const
{		
	return CVector(		m_x*mat.m_00 + m_y*mat.m_10+ m_z*mat.m_20 + m_w*mat.m_30	,
						m_x*mat.m_01 + m_y*mat.m_11+ m_z*mat.m_21 + m_w*mat.m_31	,
						m_x*mat.m_02 + m_y*mat.m_12+ m_z*mat.m_22 + m_w*mat.m_32	,
						m_x*mat.m_03 + m_y*mat.m_13+ m_z*mat.m_23 + m_w*mat.m_33	);
}

CVector CVector::operator +(const CVector& v) const
{
	return CVector(m_x+v.m_x,m_y+v.m_y,m_z+v.m_z,1);
}

CVector CVector::operator-(const CVector& v) const
{
	return CVector(m_x-v.m_x,m_y-v.m_y,m_z-v.m_z,1);
}

CVector CVector::operator-() const
{
	return CVector( -m_x, -m_y, -m_z, -m_w );
}

CVector CVector::operator /(const float d) const
{
	return CVector(m_x/d,m_y/d,m_z/d,1);
}

float& CVector::operator[]( int i )
{
	switch( i )
	{
	case 0:
		return m_x;
		break;
	case 1:
		return m_y;
		break;
	case 2:
		return m_z;
		break;
	case 3:
		return m_w;
		break;
	}

	exception e ( "Index out of array range" );
	throw e;
}

bool CVector::operator==( const CVector& v ) const
{
	return( m_x == v.m_x && m_y == v.m_y && m_z == v.m_z );
}

bool CVector::operator!=( const CVector& v ) const
{
	return !( *this == v );
}

float CVector::Norm()
{
	return sqrt(m_x * m_x + m_y * m_y + m_z * m_z);
}

void CVector::Normalize()
{
	float fNorma = Norm();
	if ( fNorma > 0 )
	{
		m_x /= fNorma;
		m_y /= fNorma;
		m_z /= fNorma;
	}
}

void CVector::Fill( float x, float y, float z, float w )
{
	m_x = x;
	m_y = y;
	m_z = z;
	m_w = w;
}

CVector	CVector::operator^( const CVector& v )
{
	return CVector( m_y * v.m_z - m_z * v.m_y, m_z * v.m_x - m_x * v.m_z, m_x * v.m_y - m_y * v.m_x );
}

CVector& CVector::operator +=( const CVector& v )
{
	m_x += v.m_x;
	m_y += v.m_y;
	m_z += v.m_z;
	return *this;
}

CVector& CVector::operator -=( const CVector& v )
{
	m_x -= v.m_x;
	m_y -= v.m_y;
	m_z -= v.m_z;
	return *this;
}

void CVector::Lerp( const CVector& v1, const CVector& v2, float t, CVector& vOut )
{
	vOut.m_x = v1.m_x + ( v2.m_x - v1.m_x ) * t;
	vOut.m_y = v1.m_y + ( v2.m_y - v1.m_y ) * t;
	vOut.m_z = v1.m_z + ( v2.m_z - v1.m_z ) * t;
}

const IPersistantObject& CVector::operator >> (CBinaryFileStorage& store) const
{
	store << m_x << m_y << m_z << m_w;
	return *this;
}

IPersistantObject& CVector::operator << (CBinaryFileStorage& store)
{
	store >> m_x >> m_y >> m_z >> m_w;
	return *this;
}

const IPersistantObject& CVector::operator >> (CAsciiFileStorage& store) const
{
	CStringStorage oString;
	oString.SetWidth(10);
	oString << *this;
	store << oString.GetValue();
	return *this;
}

IPersistantObject& CVector::operator << (CAsciiFileStorage& store)
{
	return *this;
}

const IPersistantObject& CVector::operator >> (CStringStorage& store) const
{
	store << "( " << m_x << ", " << m_y << ", " << m_z << ", " << m_w << " )";
	return *this;
}

IPersistantObject& CVector::operator << (CStringStorage& store)
{
	return *this;
}

float CVector::GetMinx(const vector< CVector >& vPoints)
{
	float fMin = vPoints[0].m_x;
	for (int i = 1; i < vPoints.size(); i++)
	{
		if (fMin > vPoints[i].m_x)
			fMin = vPoints[i].m_x;
	}
	return fMin;
}

float CVector::GetMiny(const vector< CVector >& vPoints)
{
	float fMin = vPoints[0].m_y;
	for (int i = 1; i < vPoints.size(); i++)
	{
		if (fMin > vPoints[i].m_y)
			fMin = vPoints[i].m_y;
	}
	return fMin;
}

float CVector::GetMinz(const vector< CVector >& vPoints)
{
	float fMin = vPoints[0].m_z;
	for (int i = 1; i < vPoints.size(); i++)
	{
		if (fMin > vPoints[i].m_z)
			fMin = vPoints[i].m_z;
	}
	return fMin;
}

float CVector::GetMaxx(const vector< CVector >& vPoints)
{
	float fMax = vPoints[0].m_x;
	for (int i = 1; i < vPoints.size(); i++)
	{
		if (fMax < vPoints[i].m_x)
			fMax = vPoints[i].m_x;
	}
	return fMax;
}

float CVector::GetMaxy(const vector< CVector >& vPoints)
{
	float fMax = vPoints[0].m_y;
	for (int i = 1; i < vPoints.size(); i++)
	{
		if (fMax < vPoints[i].m_y)
			fMax = vPoints[i].m_y;
	}
	return fMax;
}

float CVector::GetMaxz(const vector< CVector >& vPoints)
{
	float fMax = vPoints[0].m_z;
	for (int i = 1; i < vPoints.size(); i++)
	{
		if (fMax < vPoints[i].m_z)
			fMax = vPoints[i].m_z;
	}
	return fMax;
}


float CVector::GetDistance(const CVector& p1, const CVector& p2)
{
	return sqrtf(p1*p2);
}

int CVector::GetMinxIndex(const vector< CVector >& vPoints)
{
	int index = 0;
	float fMin = vPoints[0].m_x;
	for (int i = 1; i < vPoints.size(); i++)
	{
		if (fMin > vPoints[i].m_x) {
			fMin = vPoints[i].m_x;
			index = i;
		}
	}
	return index;
}

int CVector::GetMinyIndex(const vector< CVector >& vPoints)
{
	int index = 0;
	float fMin = vPoints[0].m_y;
	for (int i = 1; i < vPoints.size(); i++)
	{
		if (fMin > vPoints[i].m_y) {
			fMin = vPoints[i].m_y;
			index = i;
		}
	}
	return index;
}

int CVector::GetMinzIndex(const vector< CVector >& vPoints)
{
	int index = 0;
	float fMin = vPoints[0].m_z;
	for (int i = 1; i < vPoints.size(); i++)
	{
		if (fMin > vPoints[i].m_z) {
			fMin = vPoints[i].m_z;
			index = i;
		}
	}
	return index;
}

int CVector::GetMaxxIndex(const vector< CVector >& vPoints)
{
	int index = 0;
	float fMax = vPoints[0].m_x;
	for (int i = 1; i < vPoints.size(); i++)
	{
		if (fMax < vPoints[i].m_x) {
			fMax = vPoints[i].m_x;
			index = i;
		}
	}
	return index;
}

int CVector::GetMaxyIndex(const vector< CVector >& vPoints)
{
	int index = 0;
	float fMax = vPoints[0].m_y;
	for (int i = 1; i < vPoints.size(); i++)
	{
		if (fMax < vPoints[i].m_y) {
			fMax = vPoints[i].m_y;
			index = i;
		}
	}
	return index;
}

int CVector::GetMaxzIndex(const vector< CVector >& vPoints)
{
	int index = 0;
	float fMax = vPoints[0].m_z;
	for (int i = 1; i < vPoints.size(); i++)
	{
		if (fMax < vPoints[i].m_z) {
			fMax = vPoints[i].m_z;
			index = i;
		}
	}
	return index;
}

CVector2D::CVector2D():
m_x( 0 ),
m_y( 0 ),
m_w( 1 )
{
}

CVector2D::CVector2D( float x, float y ):
m_x( x ),
m_y( y ),
m_w( 1 )
{
}

CVector2D CVector2D::operator-( const CVector2D& v ) const
{
	return CVector2D( m_x - v.m_x, m_y - v.m_y );
}

void CVector2D::Normalize()
{
	float fNorma = Norm();
	if ( fNorma > 0 )
	{
		m_x /= fNorma;
		m_y /= fNorma;
	}
}

float CVector2D::Norm() const
{
	return sqrt( m_x * m_x + m_y * m_y  );
}

CVector CVector2D::operator^( const CVector2D& v ) const
{
	return CVector( 0, 0, m_x * v.m_y - m_y * v.m_x );
}

float CVector2D::operator *( const CVector2D& v ) const
{
	return ( m_x * v.m_x + m_y * v.m_y );
}

CVector2D CVector2D::operator *( float d ) const
{
	return CVector2D( m_x * d, m_y * d );
}

CVector2D CVector2D::operator+( const CVector2D& v ) const
{
	return CVector2D( m_x + v.m_x, m_y + v.m_y );
}


const IPersistantObject& CVector2D::operator >> (CBinaryFileStorage& store) const
{
	store << m_x << m_y << m_w;
	return *this;
}

IPersistantObject& CVector2D::operator << (CBinaryFileStorage& store)
{
	store >> m_x >> m_y >> m_w;
	return *this;
}

const IPersistantObject& CVector2D::operator >> (CAsciiFileStorage& store) const
{
	CStringStorage oString;
	oString.SetWidth(10);
	oString << *this;
	store << oString.GetValue();
	return *this;
}

IPersistantObject& CVector2D::operator << (CAsciiFileStorage& store)
{
	return *this;
}

const IPersistantObject& CVector2D::operator >> (CStringStorage& store) const
{
	store << "( " << m_x << ", " << m_y << ", " << ", " << m_w << " )";
	return *this;
}

IPersistantObject& CVector2D::operator << (CStringStorage& store)
{
	return *this;
}