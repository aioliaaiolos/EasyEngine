#include "Quaternion.h"

// system
#include <math.h>

// Math
#include "matrix.h"

using namespace std;

CQuaternion::CQuaternion():
m_x(0),
m_y(0),
m_z(0),
m_w(0)
{
}

CQuaternion::CQuaternion( const CQuaternion& q )
{
	m_x = q.m_x;
	m_y = q.m_y;
	m_z = q.m_z;
	m_w = q.m_w;
	m_vPosition = q.m_vPosition;
}

CQuaternion::CQuaternion( float x, float y, float z, float w ):
m_x( x ),
m_y( y ),
m_z( z ),
m_w( w )
{
}

CQuaternion::CQuaternion( const vector< float >& v ):
m_x ( v[ 0 ] ),
m_y ( v[ 1 ] ),
m_z ( v[ 2 ] ),
m_w ( v[ 3 ] )
{
}

CQuaternion::CQuaternion( const float* quat ):
m_x ( quat[ 0 ] ),
m_y ( quat[ 1 ] ),
m_z ( quat[ 2 ] ),
m_w ( quat[ 3 ] )
{
}

CQuaternion::CQuaternion( const CVector& vAxis, float fAngle )
{
	float sin_a = sinf( fAngle / 2 );
	m_x = vAxis.m_x * sin_a;
	m_y = vAxis.m_y * sin_a;
	m_z = vAxis.m_z * sin_a;
	m_w = cosf( fAngle / 2 );
	Normalize();
}

void CQuaternion::Fill( float x, float y, float z, float w )
{
	m_x = x;
	m_y = y;
	m_z = z;
	m_w = w;
}

void CQuaternion::GetConjugate( CQuaternion& oConjugate )
{
	oConjugate.m_x = -m_x;
	oConjugate.m_y = -m_y;
	oConjugate.m_z = -m_z;
	oConjugate.m_w = m_w;
}

void CQuaternion::GetMatrix( CMatrix& mat )
{
	mat.m_00 = 1 - 2 * m_y * m_y - 2 * m_z * m_z;
	mat.m_01 = 2 * m_x * m_y - 2 * m_z * m_w;
	mat.m_02 = 2 * m_x * m_z + 2 * m_y * m_w;
	mat.m_03 = m_vPosition.m_x;

	mat.m_10 = 2 * m_x * m_y + 2 * m_z * m_w;
	mat.m_11 = 1 - 2 * m_x * m_x - 2 * m_z * m_z;
	mat.m_12 = 2 * m_y * m_z - 2 * m_x * m_w;
	mat.m_13 = m_vPosition.m_y;

	mat.m_20 = 2 * m_x * m_z - 2 * m_y * m_w;
	mat.m_21 = 2 * m_y * m_z + 2 * m_x * m_w;
	mat.m_22 = 1 - 2 * m_x * m_x - 2 * m_y * m_y;
	mat.m_23 = m_vPosition.m_z;

	mat.m_30 = 0;
	mat.m_31 = 0;
	mat.m_32 = 0;
	mat.m_33 = 1;
}

float CQuaternion::GetAngle() const
{
	return 2 * acos( m_w );
}

void CQuaternion::GetAngleAxis( CVector& vAxis, float& fAngle ) const
{
	//throw 1;
	CQuaternion q = *this;
	q.Normalize();
	fAngle = 2 * acos( q.m_w );
	vAxis.m_x = q.m_x;
	vAxis.m_y = q.m_y;
	vAxis.m_z = q.m_z;
	float fNorma = sqrt( vAxis.m_x * vAxis.m_x + vAxis.m_y * vAxis.m_y + vAxis.m_z * vAxis.m_z );
	if ( fNorma > 0.f )
	{
		vAxis.m_x /= fNorma;
		vAxis.m_y /= fNorma;
		vAxis.m_z /= fNorma;
	}
	vAxis.m_w = 1;
}

float CQuaternion::GetNorm()
{
	return sqrt( m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w );
}

void CQuaternion::Normalize()
{
	float fNorm = GetNorm();
	m_x /= fNorm;
	m_y /= fNorm;
	m_z /= fNorm;
	m_w /= fNorm;
}


void CQuaternion::Slerp( const CQuaternion& q0, const CQuaternion& q1, float t, CQuaternion& qr )
{
	float fInitAngle = q0.GetAngle();
	float fFinalAngle = q1.GetAngle();
	float Omega = ( fFinalAngle - fInitAngle ) / 2.f;
	qr = ( q0 * sin( ( 1 - t ) * Omega ) + q1 * sin( t * Omega ) ) / sin( Omega );
	CVector::Lerp( q0.m_vPosition, q1.m_vPosition, t, qr.m_vPosition );
}

CQuaternion CQuaternion::operator*( float f ) const
{
	return CQuaternion( f * m_x, f * m_y, f * m_z, f * m_w );
}

CQuaternion	CQuaternion::operator*( CQuaternion& q ) const
{
	CVector v1( m_x, m_y, m_z );
	CVector v2( q.m_x, q.m_y, q.m_z );
	CVector vr;
	CQuaternion qr;
	qr.m_w = m_w * q.m_w - v1 * v2;
	vr = v2 * m_w + v1 * q.m_w + v1 ^ v2;
	qr.m_x = vr.m_x;
	qr.m_y = vr.m_y;
	qr.m_z = vr.m_z;
	return qr;
}

CQuaternion CQuaternion::operator/( float f ) const
{
	return CQuaternion( ( *this ) * ( 1.f/f ) );
}

CQuaternion CQuaternion::operator+( const CQuaternion& q ) const
{
	return CQuaternion( m_x + q.m_x, m_y + q.m_y, m_z + q.m_z, m_w + q.m_w );
}

const IPersistantObject& CQuaternion::operator >> (CBinaryFileStorage& store) const
{
	store << m_x << m_y << m_z << m_w << m_vPosition;
	return *this;
}

IPersistantObject& CQuaternion::operator << (const CBinaryFileStorage& store)
{
	store >> m_x >> m_y >> m_z >> m_w;
	store >> m_vPosition;
	return *this;
}

const IPersistantObject& CQuaternion::operator >> (CAsciiFileStorage& store) const
{
	CStringStorage oString;
	oString << *this;
	store << oString.GetValue();
	return *this;
}

IPersistantObject& CQuaternion::operator << (const CAsciiFileStorage& store)
{
	return *this;
}

const IPersistantObject& CQuaternion::operator >> (CStringStorage& store) const
{
	store << "Rotation = (" << m_x << ",  " << m_y << ",  " << m_z << ",  " << m_w << ")    "
		<< "Position = (" << m_vPosition.m_x << ",  " << m_vPosition.m_y << ",  " << m_vPosition.m_z << ",  " << m_vPosition.m_w << ")";
	return *this;
}

IPersistantObject& CQuaternion::operator << (const CStringStorage& store)
{
	return *this;
}