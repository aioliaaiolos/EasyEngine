#include "IGeometry.h"

class IBox;

class CGeometryManager : public IGeometryManager
{
	map< int, IBox* >		m_mBox;
	map< int, ISphere* >	m_mSphere;
public:
	CGeometryManager( CPlugin::Desc& oDesc );
	IWeightTable*	CreateWeightTable() const;
	IBox*			CreateBox();
	IBox*			CreateBox( const IBox& oBox );
	int				GetLastCreateBoxID();
	IBox*			GetBox( int nID ) const;
	ISphere*		CreateSphere();
	ISphere*		CreateSphere( CVector& oCenter, float fRadius );
	ISegment*		CreateSegment( const CVector& first, const CVector& last );
	ICylinder*		CreateCylinder( const CMatrix& oTM, float fRadius, float fHeight );
	ICylinder*		CreateCylinder();
	ICircle*		CreateCircle( const CVector2D& oCenter, float fRadius );
	ISegment2D*		CreateSegment2D( const CVector2D& first, const CVector2D& last );
	IQuad*			CreateQuad(float lenght, float width);
	string			GetName() override;
	void			RayCast(int x, int y, const CMatrix& oWorldMatrix, const CMatrix& projectionMatrix, int nScreenWidth, int nScreenHeight, CVector& origin, CVector& ray);
	bool			IsIntersect(const CVector& linePt1, const CVector& linePt2, const CVector& M, float radius);
};

extern "C" _declspec(dllexport) IGeometryManager* CreateGeometryManager( IGeometryManager::Desc& oDesc );