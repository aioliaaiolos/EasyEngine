#include "IGeometry.h"
#include "Math/Matrix.h"

class CSegment : public ISegment
{
public:
	
	CSegment( const CVector& first, const CVector& last );
	~CSegment();

	void	ComputeProjectedPointOnLine( const CVector& oPointToProject, CVector& oProjectedPoint ) const;
	float	ComputeDistanceToPoint( const CVector& oPoint );
	void	Compute2DLineEquation( float& a, float& b, float& c ) const;
	void	GetPoints( CVector& p1, CVector& p2 ) const;

	bool				IsIntersect(const IGeometry& oGeometry) const;
	void				SetTM(const CMatrix& m);
	void				GetTM(CMatrix& m) const;
	const CMatrix&		GetTM() const;
	const CVector&		GetBase() const;
	float				ComputeBoundingSphereRadius() const;
	IGeometry*			Duplicate();
	float				GetHeight() const;
	void				Transform(const CMatrix& tm);
	float				GetDistance(const IGeometry& oGeometry) const;
	void				Draw(IRenderer& oRenderer) const;
	TFace				GetReactionYAlignedPlane(const CVector& firstPoint, const CVector& lastPoint, float planeHeight, CVector& R);
	TFace				GetReactionYAlignedBox(IGeometry& firstPositionBox, IGeometry& lastPositionBox, CVector& R);
	void				GetBBoxDimension(CVector& dim) override { throw CMethodNotImplementedException("CSegment::GetBBoxDimension()"); }
	const CVector& 		GetBBoxDimension() const override { throw CMethodNotImplementedException("CSegment::GetBBoxDimension()"); }
	void				GetBBoxPoints(vector< CVector >& vPoints) override { throw CMethodNotImplementedException("CSegment::GetBBoxDimension()"); }

	const IPersistantObject& operator >> (CBinaryFileStorage& store) const;
	IPersistantObject& operator << (CBinaryFileStorage& store);
	const IPersistantObject& operator >> (CAsciiFileStorage& store) const;
	IPersistantObject& operator << (CAsciiFileStorage& store);
	const IPersistantObject& operator >> (CStringStorage& store) const;
	IPersistantObject& operator << (CStringStorage& store);

private:
	CVector		m_oFirst;
	CVector		m_oLast;
	CMatrix		m_oTM;
};

class CSegment2D : public ISegment2D
{
	CVector2D	m_oFirst;
	CVector2D	m_oLast;
public:

	CSegment2D( const CVector2D& first, const CVector2D& last );
	void	ComputeProjectedPointOnLine( const CVector2D& oPointToProject, CVector2D& oProjectedPoint ) const;
	float	ComputeDistanceToPoint( const CVector2D& oPoint );
	void	ComputeLineEquation( float& a, float& b, float& c ) const;
	void	GetPoints( CVector2D& p1, CVector2D& p2 ) const;
};