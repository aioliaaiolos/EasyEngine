#include "Segment.h"


CSegment::CSegment( const CVector& first, const CVector& last ) :
m_oFirst( first ),
m_oLast( last )
{
}


CSegment::~CSegment()
{
}

void CSegment::ComputeProjectedPointOnLine( const CVector& oPointToProject, CVector& oProjectedPoint ) const
{
	//float a = m_oFirst.m_z - m_oLast.m_z;
	//float b = m_oLast.m_x - m_oFirst.m_x;
	//float c1 = m_oFirst.m_x * ( m_oLast.m_z - m_oFirst.m_z ) + m_oFirst.m_z * ( m_oFirst.m_x - m_oLast.m_x );
	float a, b, c1;
	Compute2DLineEquation( a, b, c1 );
	float c2 = oPointToProject.m_x * ( m_oFirst.m_x - m_oLast.m_x ) + oPointToProject.m_z * ( m_oFirst.m_z - m_oLast.m_z );

	oProjectedPoint.m_x = -( a * c1 + b * c2 ) / ( a * a + b * b );
	oProjectedPoint.m_y = oPointToProject.m_y;
	oProjectedPoint.m_z = ( a * c2 - b * c1 ) /  ( a * a + b * b );
}

float CSegment::ComputeDistanceToPoint( const CVector& oPoint )
{
	CVector H;
	ComputeProjectedPointOnLine( oPoint, H );
	return ( H - oPoint ).Norm();
}

void CSegment::Compute2DLineEquation( float& a, float& b, float& c ) const
{
	a = m_oFirst.m_z - m_oLast.m_z;
	b = m_oLast.m_x - m_oFirst.m_x;
	c = m_oFirst.m_x * ( m_oLast.m_z - m_oFirst.m_z ) - m_oFirst.m_z * ( m_oLast.m_x - m_oFirst.m_x );
}

void CSegment::GetPoints( CVector& p1, CVector& p2 ) const
{
	p1 = m_oFirst;
	p2 = m_oLast;
}

bool CSegment::IsIntersect(const IGeometry& oGeometry) const
{
	return false;
}

void CSegment::SetTM(const CMatrix& m)
{

}

void CSegment::GetTM(CMatrix& m) const
{

}

const CMatrix& CSegment::GetTM() const
{
	return m_oTM;
}

const CVector& CSegment::GetBase() const
{
	return m_oTM.GetPosition();
}

float CSegment::ComputeBoundingSphereRadius() const
{
	return (m_oFirst - m_oLast).Norm();
}

IGeometry*	CSegment::Duplicate()
{
	return NULL;
}

float CSegment::GetHeight() const
{
	return 0.f;
}

void CSegment::Transform(const CMatrix& tm)
{

}

float CSegment::GetDistance(const IGeometry& oGeometry) const
{
	return 0.f;
}

void CSegment::Draw(IRenderer& oRenderer) const
{

}

IGeometry::TFace CSegment::GetReactionYAlignedPlane(const CVector& firstPoint, const CVector& lastPoint, float planeHeight, CVector& R)
{
	return eNone;
}
IGeometry::TFace CSegment::GetReactionYAlignedBox(IGeometry& firstPositionBox, IGeometry& lastPositionBox, CVector& R)
{
	return eNone;
}

const IPersistantObject& CSegment::operator >> (CBinaryFileStorage& store) const
{
	return *this;
}

IPersistantObject& CSegment::operator << (const CBinaryFileStorage& store)
{
	return *this;
}

const IPersistantObject& CSegment::operator >> (CAsciiFileStorage& store) const
{
	return *this;
}

IPersistantObject& CSegment::operator << (const CAsciiFileStorage& store)
{
	return *this;
}

const IPersistantObject& CSegment::operator >> (CStringStorage& store) const
{
	return *this;
}

IPersistantObject& CSegment::operator << (const CStringStorage& store)
{
	return *this;
}

CSegment2D::CSegment2D( const CVector2D& first, const CVector2D& last ):
m_oFirst( first ),
m_oLast( last )
{
}

void CSegment2D::ComputeProjectedPointOnLine( const CVector2D& oPointToProject, CVector2D& oProjectedPoint ) const
{
	float a, b, c1;
	ComputeLineEquation( a, b, c1 );
	float c2 = oPointToProject.m_x * ( m_oFirst.m_x - m_oLast.m_x ) + oPointToProject.m_y * ( m_oFirst.m_y - m_oLast.m_y );

	oProjectedPoint.m_x = -( a * c1 + b * c2 ) / ( a * a + b * b );
	oProjectedPoint.m_y = ( a * c2 - b * c1 ) /  ( a * a + b * b );
}

float CSegment2D::ComputeDistanceToPoint( const CVector2D& oPoint )
{
	CVector2D H;
	ComputeProjectedPointOnLine( oPoint, H );
	return ( H - oPoint ).Norm();
}

void CSegment2D::ComputeLineEquation( float& a, float& b, float& c ) const
{
	a = m_oFirst.m_y - m_oLast.m_y;
	b = m_oLast.m_x - m_oFirst.m_x;
	c = m_oFirst.m_x * ( m_oLast.m_y - m_oFirst.m_y ) - m_oFirst.m_y * ( m_oLast.m_x - m_oFirst.m_x );
}

void CSegment2D::GetPoints( CVector2D& p1, CVector2D& p2 ) const
{
	p1 = m_oFirst;
	p2 = m_oLast;
}