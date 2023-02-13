#include "Quad.h"
#include "Segment.h"
#include "IRenderer.h"

CQuad::CQuad(float fLenght, float fWidth) :
	m_fLenght(fLenght),
	m_fWidth(fWidth)
{

}

void CQuad::GetLineIntersection(const CVector& A, const CVector& B, CVector& I)
{	
	float d = m_oTM.GetPosition().Norm();
	if (m_oTM.m_13 < 0)
		d = -d;
	CVector n;
	ComputeNormal(n);
	n.m_w = 0.f;
	CVector D = B - A;
	D.Normalize();
	float num = A*n;
	float den = n*D;
	float fract = num / den;
	float t2 = (d - num) / den;
	float t = (d - A*n) / (n * D);
	I = A + D * t;
}

void CQuad::GetDimension(float& lenght, float& width)
{
	m_fLenght = lenght;
	m_fWidth = width;
}

bool CQuad::IsIntersect(const IGeometry& oGeometry) const
{
	const CSegment* segment = dynamic_cast<const CSegment*>(&oGeometry);
	if (segment) {
		CVector A, B;
		segment->GetPoints(A, B);
		float d = m_fLenght;
		CVector O = A;

		CVector n;
		ComputeNormal(n);
		CVector D = B - A;
		float t = -(d + O*n) / (n * D);

	}
	throw 1;
	return false;
}

void CQuad::ComputeNormal(CVector& n) const
{
	CVector ex, ey, ez;
	m_oTM.GetxBase(ex);
	m_oTM.GetzBase(ez);	
	n = ez ^ ex;
}

void CQuad::SetTM(const CMatrix& oTM)
{
	m_oTM = oTM;
}

void CQuad::GetTM(CMatrix& oTM) const
{
	oTM = m_oTM;
}

const CMatrix& CQuad::GetTM() const
{
	return m_oTM;
}

const CVector& CQuad::GetBase() const
{
	return m_oTM.GetPosition();
}

float CQuad::ComputeBoundingSphereRadius() const
{
	return m_fLenght;
}

IGeometry* CQuad::Duplicate() const
{
	return NULL;
}

float CQuad::GetHeight() const
{
	return 0.f;
}

void CQuad::Transform(const CMatrix& oTM)
{
	m_oTM = oTM;
}

float CQuad::GetDistance(const IGeometry& oGeometry) const
{
	return 0.f;
}

void CQuad::Draw(IRenderer& oRenderer) const
{
	oRenderer.DrawQuad(m_fLenght, m_fWidth);
}

IGeometry::TFace CQuad::GetReactionYAlignedPlane(const CVector& firstPoint, const CVector& lastPoint, float planeHeight, CVector& R)
{
	return eNone;
}

IGeometry::TFace CQuad::GetReactionYAlignedBox(IGeometry& firstPositionBox, IGeometry& lastPositionBox, CVector& R)
{
	return eNone;
}

void CQuad::GetCenter(CVector& oCenter) const
{
	oCenter = GetBase();
}

float CQuad::GetRadius() const
{
	return (m_fLenght / 2.f) * (m_fLenght / 2.f) + (m_fWidth / 2.f) * (m_fLenght / 2.f);
}

const IPersistantObject& CQuad::operator >> (CBinaryFileStorage& store) const
{
	store << eQuad << m_fLenght << m_fWidth;
	return *this;
}

IPersistantObject& CQuad::operator << (const CBinaryFileStorage& store)
{
	store >> m_fLenght >> m_fWidth;
	return *this;
}

const IPersistantObject& CQuad::operator >> (CAsciiFileStorage& store) const
{
	store << "Type = Quad, lenght = " << m_fLenght << ", width = " << m_fWidth;
	return *this;
}

IPersistantObject& CQuad::operator << (const CAsciiFileStorage& store)
{
	//store >> "Type = Quad, lenght = " << m_fLenght << ", width = " << m_fWidth;
	throw 1;
	return *this;
}

const IPersistantObject& CQuad::operator >> (CStringStorage& store) const
{
	store << "Type = Quad, lenght = " << m_fLenght << ", width = " << m_fWidth;
	return *this;
}

IPersistantObject& CQuad::operator << (const CStringStorage& store)
{
	throw 1;
	return *this;
}