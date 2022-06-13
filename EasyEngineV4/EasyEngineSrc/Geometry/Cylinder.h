#include "Math/Matrix.h"
#include "IGeometry.h"

class CBox;

class CCylinder : public ICylinder
{
	CMatrix		m_oTM;
	float		m_fRadius;
	float		m_fHeight;

public:
	CCylinder();
	CCylinder(const CCylinder& box);
	CCylinder( const CMatrix& oTM, float fRadius, float fHeight );
	float				GetRadius() const;
	float				GetHeight() const;
	void				SetTM(const CMatrix& m);
	void				Set( const CMatrix& oBase, float fRadius, float fHeight );
	void				ComputeTangent( const CVector& oLinePoint, CVector& oTangentPoint, bool bLeft );	
	bool				IsPointIntoCylinder( const CVector& oPoint ) const;
	bool				IsIntersect(const IGeometry& box) const;
	bool				IsIntersect(const CBox& pBox) const;
	void				GetTM(CMatrix& m) const;
	const CMatrix&		GetTM() const;
	float				ComputeBoundingSphereRadius() const;
	const CVector&		GetBase() const;
	IGeometry*			Duplicate();
	void				Transform(const CMatrix& tm);
	float				GetHeight();
	float				GetDistance(const IGeometry& oGeometry) const;
	float				GetDistance(const IBox& oBox) const;
	float				GetDistance(const ICylinder& oBox) const;
	void				Draw(IRenderer& oRenderer) const;
	TFace				GetReactionYAlignedPlane(const CVector& firstPoint, const CVector& lastPoint, float planeHeight, CVector& R);
	TFace				GetReactionYAlignedBox(IGeometry& firstPositionBox, IGeometry& lastPositionBox, CVector& R);
	void				GetBBoxDimension(CVector& dim) override;
	

	const IPersistantObject& operator >> (CBinaryFileStorage& store) const;
	IPersistantObject& operator << (CBinaryFileStorage& store);
	const IPersistantObject& operator >> (CAsciiFileStorage& store) const;
	IPersistantObject& operator << (CAsciiFileStorage& store);
	const IPersistantObject& operator >> (CStringStorage& store) const;
	IPersistantObject& operator << (CStringStorage& store);
};