#include "GeometryManager.h"
#include "WeightTable.h"
#include "Box.h"
#include "Sphere.h"
#include "Segment.h"
#include "Cylinder.h"
#include "Circle.h"
#include "Quad.h"

#define M_PI_2     1.57079632679489661923   // pi/2

CGeometryManager::CGeometryManager( CPlugin::Desc& oDesc ) : IGeometryManager()
{
}

IWeightTable* CGeometryManager::CreateWeightTable() const
{
	return new CWeightTable;
}

IBox* CGeometryManager::CreateBox()
{
	IBox* pBox = new CBox;
	m_mBox[ (int)m_mBox.size() ] = pBox;
	return pBox;
}

IBox* CGeometryManager::CreateBox( const IBox& oBox )
{
	const CBox& oCBox = static_cast< const CBox& >( oBox );
	IBox* pBox = new CBox( oCBox );
	m_mBox[ (int)m_mBox.size() ] = pBox;
	return pBox;
}

int CGeometryManager::GetLastCreateBoxID()
{
	return (int)(m_mBox.size() - 1);
}

IBox* CGeometryManager::GetBox( int nID ) const
{
	map< int, IBox* >::const_iterator itBox = m_mBox.find( nID );
	if( itBox != m_mBox.end() )
		return itBox->second;
	return NULL;
}

ISphere* CGeometryManager::CreateSphere()
{
	ISphere* pSphere = new CSphere( CVector(), 0.f );
	m_mSphere[ (int)m_mSphere.size() ] = pSphere;
	return pSphere;
}

ISphere* CGeometryManager::CreateSphere( CVector& oCenter, float fRadius )
{
	ISphere* pSphere = new CSphere( oCenter, fRadius );
	m_mSphere[ (int)m_mSphere.size() ] = pSphere;
	return pSphere;
}

ISegment* CGeometryManager::CreateSegment( const CVector& first, const CVector& last )
{
	return new CSegment( first, last );
}

ICylinder* CGeometryManager::CreateCylinder()
{
	return new CCylinder;
}

ICylinder* CGeometryManager::CreateCylinder( const CMatrix& oTM, float fRadius, float fHeight )
{
	return new CCylinder(oTM, fRadius, fHeight );
}

ICircle* CGeometryManager::CreateCircle( const CVector2D& oCenter, float fRadius )
{
	return new CCircle( oCenter, fRadius );
}

ISegment2D*	CGeometryManager::CreateSegment2D( const CVector2D& first, const CVector2D& last )
{
	return new CSegment2D( first, last );
}

IQuad* CGeometryManager::CreateQuad(float lenght, float width)
{
	return new CQuad(lenght, width);
}

string CGeometryManager::GetName()
{
	return "GeometryManager";
}

void CGeometryManager::RayCast(int x, int y, const CMatrix& oWorldMatrix, const CMatrix& projectionMatrix, int nScreenWidth, int nScreenHeight, CVector& origin, CVector& ray)
{
	CMatrix Pinv;
	projectionMatrix.GetInverse(Pinv);

	float logicalx = (2.f * (float)x / (float)nScreenWidth) - 1.f;
	float logicaly = 1.f - (2.f * (float)y / (float)nScreenHeight);

	CVector ray_nds(logicalx, logicaly, 1.f, 1.f);
	CVector ray_clip = CVector(ray_nds.m_x, ray_nds.m_y, -1.0, 1.0);

	CVector ray_eye = Pinv * ray_clip;
	ray_eye = CVector(ray_eye.m_x, ray_eye.m_y, -1.f, 0.f);

	ray = oWorldMatrix * ray_eye;
	ray.Normalize();

	oWorldMatrix.GetPosition(origin);
}

bool CGeometryManager::IsIntersect(const CVector& linePt1, const CVector& linePt2, const CVector& M, float radius)
{
	CVector P12 = linePt2 - linePt1;
	CVector P1M = M - linePt1;
	float alpha = acosf((P12 * P1M) / (P12.Norm() * P1M.Norm()));
	float d = P1M.Norm() * sinf(alpha);
	if ((alpha < M_PI_2) && (d < radius)) {
		CVector P2M = M - linePt2;
		float beta = acosf((-P12 * P2M) / (P12.Norm() * P2M.Norm()));
		return beta < M_PI_2;
	}

	return false;
}

extern "C" _declspec(dllexport) IGeometryManager* CreateGeometryManager( IGeometryManager::Desc& oDesc )
{
	return new CGeometryManager( oDesc );
}