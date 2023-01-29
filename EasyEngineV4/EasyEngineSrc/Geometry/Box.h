#ifndef BOX_H
#define BOX_H

#include "math/Vector.h"
#include "math/Matrix.h"
#include "IGeometry.h"

class IRenderer;

class CBox : public IBox
{
public:

	CBox();
	CBox(CVector& oMinPoint, CVector& oDimension);
	CBox(const CBox& oBox);

	void				SetTM(const CMatrix& m);
	const CVector&		GetMinPoint() const;
	void				SetMinPoint(const CVector& oMinPoint);
	void				GetTM( CMatrix& m ) const;
	const CMatrix&		GetTM() const;
	const CVector&		GetDimension() const;
	void				GetDimension(CVector& dim) const;
						
	void				Set( const CVector& oMinPoint, const CVector& oDimension );
	void				GetCenter( CVector& oCenter ) const;
	void				SetX(float x);
	void				SetY(float y);
	void				SetZ(float z);
	void				AddPoint( const CVector& p );
	float				GetBoundingSphereRadius() const;
	float				ComputeBoundingSphereRadius()const;
	float				ComputeBoundingCylinderRadius( TAxis eGeneratorAxis ) const;
	const CVector&		CBox::GetBase() const;
	IGeometry*			Duplicate();
	float				GetHeight() const;
	void				Transform(const CMatrix& tm);
	float				GetDistance(const IGeometry& oGeometry) const;
	float				GetDistance(const CBox& oBox) const;
	float				GetDistance(const ICylinder& oBox) const;
	IBox&				operator=(const IBox& oBox);
	void				GetPoints(vector< CVector >& vPoints);
	void				GetBBoxPoints(vector< CVector >& vPoints);
	void				GetCenterPoints(vector< CVector >& vPoints) const;
	bool				IsIntersect(const IGeometry& box) const;
	bool				IsIntersect(const CBox& box) const;
	void				Draw(IRenderer& oRenderer) const;
	TFace				GetReactionYAlignedPlane(const CVector& firstPoint, const CVector& lastPoint, float planeHeight, CVector& R);
	TFace				GetReactionYAlignedBox(IGeometry& firstPositionBox, IGeometry& lastPositionBox, CVector& R);
	bool				IsIncludedInto(const IGeometry& oGeometry) override;
	void				GetBBoxDimension(CVector& dim) override;
	const CVector&		GetBBoxDimension() const override;

	const IPersistantObject& operator >> (CBinaryFileStorage& store) const override;
	IPersistantObject& operator << (const CBinaryFileStorage& store) override;
	const IPersistantObject& operator >> (CAsciiFileStorage& store) const override;
	IPersistantObject& operator << (const CAsciiFileStorage& store) override;
	const IPersistantObject& operator >> (CStringStorage& store) const override;
	IPersistantObject& operator << (const CStringStorage& store) override;

private:
	bool				m_bInitialized;
	float				m_fBoundingSphereRadius;
	CVector				m_oMinPoint;
	CMatrix				m_oTM;
	CMatrix				m_oBackupInvTM;
	CVector				m_oDimension;

	bool				TestBoxesCollisionIntoFirstBoxBase(const IBox& b1, const IBox& b2) const;
	float				GetDistanceInBase(const IBox& oBox) const;
};

#endif // BOX_H