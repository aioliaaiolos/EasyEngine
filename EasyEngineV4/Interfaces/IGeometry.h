#ifndef IGEOMETRY_H
#define IGEOMETRY_H

#include "EEPlugin.h"
#include <vector>
#include "Math/Vector.h"
#include "Exception.h"

using namespace std;

class IRenderer;

class IWeightTable
{
public:
	virtual void	Add( int iVertexIndex, int nBoneID, float fBoneWeight ) = 0;
	virtual bool	Get( int iVertexIndex, std::map< int, float >& mWeight ) const = 0;
	virtual int		GetVertexCount() const = 0;
	virtual void	GetArrays( vector<float >& vWeightVertex, vector< float >& vWeigtedVertexID ) = 0;
	virtual void	BuildFromArrays( const vector<float >& vWeightVertex, const vector< float >& vWeightedVertexID ) = 0;
};

class ISphere
{
public:
	virtual float				GetRadius() const = 0;
	virtual void				SetRadius( float fRadius ) = 0;
	virtual void				SetCenter( CVector& oCenter ) = 0;
	virtual const CVector&		GetCenter() const = 0;
	
};

class IBox;

class IGeometry : public IPersistantObject
{
public:
	enum Type
	{
		eBox = 1,
		eCylinder,
		eQuad
	};

	enum TFace
	{
		eNone = 0,
		eXPositive,
		eXNegative,
		eYPositive,
		eYNegative,
		eZPositive,
		eFace,
		eZNegative
	};

	
	virtual bool				IsIntersect(const IGeometry& oGeometry) const = 0;
	virtual void				SetTM(const CMatrix& m) = 0;
	virtual void				GetTM(CMatrix& m) const = 0;
	virtual const CMatrix&		GetTM() const = 0;	
	virtual const CVector&		GetBase() const = 0;
	virtual float				ComputeBoundingSphereRadius() const = 0;
	virtual IGeometry*			Duplicate() = 0;
	virtual float				GetHeight() const = 0;
	virtual void				Transform(const CMatrix& tm) = 0;
	virtual float				GetDistance(const IGeometry& oGeometry) const = 0;
	virtual void				Draw(IRenderer& oRenderer) const = 0;
	virtual TFace				GetReactionYAlignedPlane(const CVector& firstPoint, const CVector& lastPoint, float planeHeight, CVector& R) = 0;
	virtual TFace				GetReactionYAlignedBox(IGeometry& firstPositionBox, IGeometry& lastPositionBox, CVector& R) = 0;
	virtual bool				IsIncludedInto(const IGeometry& oGeometry) = 0;
	virtual void				GetBBoxDimension(CVector& dim) = 0;
	virtual const CVector&		GetBBoxDimension() const = 0;
	virtual void				GetBBoxPoints(vector< CVector >& vPoints) = 0;
	virtual void				GetName(string& sName) { sName = m_sName; }
	virtual void				SetName(string& sName) { m_sName = sName; }

protected:
	string m_sName;
};

class ILine
{
public:
	virtual void			GetPoints(CVector& first, CVector& last) const = 0;
};

class IQuad : public IGeometry
{
public:
	virtual void				GetDimension(float& lenght, float& width) = 0;
	virtual void				GetLineIntersection(const CVector& A, const CVector& B, CVector& I) = 0;
	virtual bool				IsIncludedInto(const IGeometry& oGeometry) override { return false; }
};

class IBox : public IGeometry
{
public:

	enum TAxis
	{
		eAxisX = 0,
		eAxisY,
		eAxisZ
	};

	virtual const CVector&		GetMinPoint() const = 0;
	virtual const CVector&		GetDimension() const = 0;
	virtual float				ComputeBoundingCylinderRadius( TAxis eGeneratorAxis ) const = 0;
	virtual float				GetBoundingSphereRadius() const = 0;
	virtual void				AddPoint( const CVector& p ) = 0;
	virtual IBox&				operator=( const IBox& oBox ) = 0;
	virtual void				Set( const CVector& oMinPoint, const CVector& oDimension ) = 0;
	virtual void				GetDimension(CVector& dim) const = 0;
	virtual void				SetTM(const CMatrix& m) = 0;
	virtual void				GetPoints( vector< CVector >& vPoints ) = 0;
	virtual void				GetCenter( CVector& oCenter ) const = 0;
	virtual void				SetX(float x) = 0;
	virtual void				SetY(float y) = 0;
	virtual void				SetZ(float z) = 0;
	virtual void				SetMinPoint(const CVector& oMinPoint) = 0;
	virtual bool				IsIncludedInto(const IGeometry& oGeometry) override = 0;
};

class ICylinder : public IGeometry
{
public:
	virtual float		GetRadius() const = 0;
	virtual void		SetTM(const CMatrix& m) = 0;
	virtual void		Set(const CMatrix& oBase, float fRadius, float fHeight) = 0;
	virtual void		ComputeTangent(const CVector& oLinePoint, CVector& oTangentPoint, bool bLeft) = 0;
	virtual bool		IsPointIntoCylinder(const CVector& oPoint) const = 0;
	virtual bool		IsIncludedInto(const IGeometry& oGeometry) override { throw CMethodNotImplementedException("ICylinder::IsIncludedInto"); }
};


class ISegment : public IGeometry
{
public:
	virtual ~ISegment() = 0{}
	virtual void	ComputeProjectedPointOnLine( const CVector& oPointToProject, CVector& oProjectedPoint ) const = 0;
	virtual float	ComputeDistanceToPoint( const CVector& oPoint ) = 0;
	virtual void	Compute2DLineEquation( float& a, float& b, float& c ) const = 0;
	virtual void	GetPoints( CVector& p1, CVector& p2 ) const = 0;
	bool			IsIncludedInto(const IGeometry& oGeometry) override { return false; }
	//virtual void	SetPoints( const CVector& first, const CVector last ) = 0;
};

class ISegment2D
{
public:
	virtual ~ISegment2D() = 0{}
	virtual void	ComputeProjectedPointOnLine( const CVector2D& oPointToProject, CVector2D& oProjectedPoint ) const = 0;
	virtual float	ComputeDistanceToPoint( const CVector2D& oPoint ) = 0;
	virtual void	ComputeLineEquation( float& a, float& b, float& c ) const = 0;
	virtual void	GetPoints( CVector2D& p1, CVector2D& p2 ) const = 0;
};

class ICircle
{
public:
	virtual void			Set( const CVector2D& oCenter, float fRadius ) = 0;
	virtual bool			IsSegmentAtLeftSide( const CVector2D& oStartPoint, const CVector2D& oEndPoint ) const = 0;
	virtual bool			IsPointIntoCircle( const CVector2D& oPoint ) const = 0;
	virtual void			ComputeTangent( const CVector2D& oLinePoint, CVector2D& oTangentPoint, bool bLeft ) const = 0;
	virtual CVector2D&		GetCenter() = 0;
};

class IGeometryManager : public CPlugin
{
public:
	IGeometryManager() : CPlugin( NULL, "GeometryManager" ){}
	virtual IWeightTable*	CreateWeightTable() const = 0;
	virtual IBox*			CreateBox() = 0;
	virtual IBox*			CreateBox( const IBox& oBox ) = 0;
	virtual ISphere*		CreateSphere() = 0;
	virtual ISphere*		CreateSphere( CVector& oCenter, float fRadius ) = 0;
	virtual ISegment*		CreateSegment( const CVector& first, const CVector& last ) = 0;
	virtual ISegment2D*		CreateSegment2D( const CVector2D& first, const CVector2D& last ) = 0;
	virtual ICylinder*		CreateCylinder() = 0;
	virtual ICylinder*		CreateCylinder(const CMatrix& oTM, float fRadius, float fHeight) = 0;
	virtual ICircle*		CreateCircle( const CVector2D& oCenter, float fRadius ) = 0;
	virtual IQuad*			CreateQuad(float lenght, float width) = 0;
	virtual int				GetLastCreateBoxID() = 0;
	virtual IBox*			GetBox( int nID ) const = 0;
	virtual void			RayCast(int x, int y, const CMatrix& oWorldMatrix, const CMatrix& projectionMatrix, int nScreenWidth, int nScreenHeight, CVector& origin, CVector& ray) = 0;
	virtual bool			IsIntersect(const CVector& linePt1, const CVector& linePt2, const CVector& M, float radius) = 0;
};

#endif // IGEOMETRY_H