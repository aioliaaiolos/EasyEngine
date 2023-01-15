#include "IGeometry.h"
#include "Math/Matrix.h"

class IRenderer;

class CQuad : public IQuad
{
public :
	CQuad(float fLenght, float fWidth);
	void				GetLineIntersection(const CVector& A, const CVector& B, CVector& I);
	void				GetDimension(float& lenght, float& width);
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
	void				GetBBoxDimension(CVector& dim) override { throw CMethodNotImplementedException("CQuad::GetBBoxDimension()"); }
	const CVector&		GetBBoxDimension() const override { throw CMethodNotImplementedException("CQuad::GetBBoxDimension()"); }
	void				GetBBoxPoints(vector< CVector >& vPoints) override { throw CMethodNotImplementedException("CQuad::GetBBoxDimension()"); }

	const IPersistantObject& operator >> (CBinaryFileStorage& store) const;
	IPersistantObject& operator << (CBinaryFileStorage& store);
	const IPersistantObject& operator >> (CAsciiFileStorage& store) const;
	IPersistantObject& operator << (CAsciiFileStorage& store);
	const IPersistantObject& operator >> (CStringStorage& store) const;
	IPersistantObject& operator << (CStringStorage& store);


private:
	void			ComputeNormal(CVector& n) const;
	float			m_fLenght;
	float			m_fWidth;
	CMatrix			m_oTM;
};