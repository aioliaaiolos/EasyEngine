#include "Cylinder.h"
#include <math.h>
#include "math/matrix.h"
#include "Box.h"
#include "IRenderer.h"

CCylinder::CCylinder() :
	m_fHeight(0),
	m_fRadius(0)
{
}

CCylinder::CCylinder(const CCylinder& cyl)
{
	m_oTM = cyl.GetTM();
	m_fRadius = cyl.GetRadius();
	m_fHeight = cyl.GetHeight();
}

CCylinder::CCylinder( const CMatrix& oTM, float fRadius, float fHeight ):
m_oTM( oTM ),
m_fRadius( fRadius ),
m_fHeight( fHeight )
{
}

float CCylinder::GetRadius() const
{
	return m_fRadius;
}

float CCylinder::GetHeight() const
{
	return m_fHeight;
}

void CCylinder::SetTM(const CMatrix& m)
{
	m_oTM = m;
}

void CCylinder::Set( const CMatrix& oTM, float fRadius, float fHeight )
{
	m_oTM = oTM;
	m_fRadius = fRadius;
	m_fHeight = fHeight;

	m_oDimension.m_x = m_fRadius * 2.f;
	m_oDimension.m_y = m_fHeight;
	m_oDimension.m_z = m_fRadius * 2.f;
}

void CCylinder::ComputeTangent( const CVector& oLinePoint, CVector& oTangentPoint, bool bLeft )
{
	CVector oBase = m_oTM.GetPosition();
	CVector oPointToCenter = oBase - oLinePoint;
	oPointToCenter.m_y = 0;
	float fPointToCenter = oPointToCenter.Norm();
	float alpha = asinf( m_fRadius / fPointToCenter ) * 180.f / 3.1415927f;
	if( !bLeft ) alpha = -alpha;
	
	CVector oPointToCenterNorm = oPointToCenter;
	oPointToCenterNorm.Normalize();
	CVector oPointToTangentNorm = CMatrix::GetyRotation( alpha ) * oPointToCenterNorm;
	float fPointToTangentNorm = sqrt( oPointToCenter.Norm() * oPointToCenter.Norm() - m_fRadius * m_fRadius );
	CVector oPointToTangent = oPointToTangentNorm * fPointToTangentNorm;
	oTangentPoint = oLinePoint + oPointToTangent;
}

bool CCylinder::IsPointIntoCylinder( const CVector& oPoint ) const
{
	CVector oBase = m_oTM.GetPosition();
	CVector oPoint2D( oPoint.m_x, 0.f, oPoint.m_z );
	if( ( oPoint2D - oBase).Norm() > m_fRadius )
		return false;
	if( oPoint.m_y < oBase.m_y || oPoint.m_y > oBase.m_y + m_fHeight )
		return false;
	return true;
}

bool CCylinder::IsIntersect(const IGeometry& oGeometry) const
{
	const CBox* pBox = dynamic_cast<const CBox*>(&oGeometry);
	if (pBox)
		return IsIntersect((const CBox&)*pBox);
	throw 1;
	return false;
}

bool CCylinder::IsIntersect(const CBox& box) const
{
	CMatrix cylMat, boxMat;
	GetTM(cylMat);
	box.GetTM(boxMat);
	CMatrix cylMatInv;
	cylMat.GetInverse(cylMatInv);
	CMatrix boxMatInCylBase = cylMatInv * boxMat;

	CBox boxTemp;
	boxTemp.Set(box.GetMinPoint(), box.GetDimension());
	boxTemp.SetTM(boxMatInCylBase);

	vector<CVector> points;
	boxTemp.GetPoints(points);

	float fMinx = -m_fRadius;
	if(fMinx > CVector::GetMaxx(points))
		return false;
	float fMiny = -m_fHeight / 2.f;
	if (fMiny > CVector::GetMaxy(points))
		return false;
	float fMinz = -m_fRadius;
	if (fMinz > CVector::GetMaxz(points))
		return false;
	float fMaxx = m_fRadius;
	if (fMaxx < CVector::GetMinx(points))
		return false;
	float fMaxy = m_fHeight / 2.f;
	if (fMaxy < CVector::GetMiny(points))
		return false;
	float fMaxz = m_fRadius;
	if (fMaxz < CVector::GetMinz(points))
		return false;

	return true;
	
	//Faire la meme chose en faisant le changement de base dans la base de la boite
	CMatrix boxMatInv;
	boxMat.GetInverse(boxMatInv);
	CMatrix cylMatInBoxBase = boxMatInv * cylMat;
	CVector cylBase = cylMatInBoxBase * CVector(0, 0, 0, 1);
	
	float dx1 = cylBase.m_x - m_fRadius / 2.f;
	float dx2 = box.GetMinPoint().m_x + box.GetDimension().m_x;
	if (dx1 > dx2)
		return false;

	float dy1 = cylBase.m_y - m_fRadius / 2.f;
	float dy2 = box.GetMinPoint().m_y + box.GetDimension().m_y;
	if (dy1 > dy2)
		return false;

	float dz1 = cylBase.m_z - m_fHeight / 2.f;
	float dz2 = box.GetMinPoint().m_z + box.GetDimension().m_z;
	if (dz1 > dz2)
		return false;


	if (cylBase.m_x + m_fRadius < box.GetMinPoint().m_x)
		return false;
	if (cylBase.m_y + m_fRadius < box.GetMinPoint().m_y)
		return false;
	if (cylBase.m_z + m_fHeight / 2.f < box.GetMinPoint().m_z)
		return false;
		
	return true;
}

const IPersistantObject& CCylinder::operator >> (CBinaryFileStorage& store) const
{
	store << (int)eCylinder << m_oTM << m_fRadius << m_fHeight;
	return *this;
}

IPersistantObject& CCylinder::operator << (const CBinaryFileStorage& store)
{
	store >> m_oTM >> m_fRadius >> m_fHeight;
	return *this;
}

const IPersistantObject& CCylinder::operator >> (CAsciiFileStorage& store) const
{
	return *this;
}

IPersistantObject& CCylinder::operator << (const CAsciiFileStorage& store)
{
	return *this;
}

const IPersistantObject& CCylinder::operator >> (CStringStorage& store) const
{
	return *this;
}

IPersistantObject& CCylinder::operator << (const CStringStorage& store)
{
	return *this;
}

void CCylinder::GetTM(CMatrix& m) const
{
	m = m_oTM;
}

const CMatrix& CCylinder::GetTM() const
{
	return m_oTM;
}

float CCylinder::ComputeBoundingSphereRadius() const
{
	return sqrtf(pow(m_fHeight / 2.f, 2) + pow(m_fRadius, 2));
}

const CVector& CCylinder::GetBase() const
{
	return m_oTM.GetPosition();
}

IGeometry* CCylinder::Duplicate()
{
	return new CCylinder(*this);
}

void CCylinder::Transform(const CMatrix& tm)
{
}

float CCylinder::GetHeight()
{
	return m_fHeight;
}

float CCylinder::GetDistance(const IGeometry& oGeometry) const
{
	return -1.f;
}

float CCylinder::GetDistance(const IBox& oBox) const
{
	return -1.f;
}

float CCylinder::GetDistance(const ICylinder& oBox) const
{
	return -1.f;
}

void CCylinder::Draw(IRenderer& oRenderer) const
{
	throw 1;
}


IGeometry::TFace CCylinder::GetReactionYAlignedPlane(const CVector& firstPoint, const CVector& lastPoint, float planeHeight, CVector& R)
{
	static bool oldVersion = false;
	if (oldVersion) {
		R = firstPoint;
		return eFace;
	}
	
	CVector first = firstPoint;
	CVector last = lastPoint;
	last.m_y = first.m_y;


	CVector center = m_oTM.GetPosition();
	center.m_y = first.m_y;
	float distanceToCenter = (center - last).Norm();
	if (distanceToCenter > m_fRadius)
		return eNone;

	// First, build a new basis into which work.
	// This basis is componed by newX, unitary vector colinear to (firstPoint, lastPoint) vector
	// newY, which is the world Y axis and newZ, an unitary vector orthogonal to the 2 first.
	CVector newX = last - first;
	newX.Normalize();
	CVector newY = CVector(0, 1, 0, 0);
	CVector newZ = newX ^ newY;
	newX.m_w = 0.f;
	newY.m_w = 0.f;
	newZ.m_w = 0.f;
	CMatrix OPBase(newX, newY, newZ, CVector(0, 0, 0, 1), false);
	OPBase.SetAffinePart(first);

	CMatrix OPBaseInv;
	OPBase.GetInverse(OPBaseInv);

	CVector C = OPBaseInv * center; // cylinder center into the new basis
	CVector P = OPBaseInv * last; // lastPoint into the new basis

	float CH = C.m_z; // distance between C and H, H is the orthogonal projection of C on OP (with O is the local firstPoint)
	float IH2 = m_fRadius * m_fRadius - CH * CH;
	float IH = sqrtf(IH2);

	CVector I; // intersect point between OP and cylinder
	I.m_x = C.m_x - IH;

	float IP = P.m_x - I.m_x;
	
	float cosa = IH / m_fRadius; // a is angle between CI and CH, also between IP and IR
	float sina = sqrtf(1 - cosa * cosa);

	float IR = CH * IP / m_fRadius;
	CVector RLocal;
	RLocal.m_x = IR * cosa;
	RLocal.m_z = -IR * sina;
	R = OPBase * RLocal;
	return eFace;
}


IGeometry::TFace CCylinder::GetReactionYAlignedBox(IGeometry& firstPositionBox, IGeometry& lastPositionBox, CVector& R)
{
	CVector first = firstPositionBox.GetTM().GetPosition();
	CVector last = lastPositionBox.GetTM().GetPosition();
	last.m_y = first.m_y;

	static bool test = false;
	if (!test) {
		CVector C = GetTM().GetPosition();
		float d1 = (first - C).Norm();
		float d2 = (last - C).Norm();
		R = d2 > d1 ? last : first;
		return eFace;
	}
	
	if (first == last) {
		R = last;
		return eFace;
	}
	// Build a new basis into which work.
	// This basis is componed by newX, unitary vector colinear to (firstPoint, lastPoint) vector
	// newY, which is the world Y axis and newZ, an unitary vector orthogonal to the 2 first.
	CVector newX = last - first;
	newX.Normalize();
	CVector newY = CVector(0, 1, 0, 0);
	CVector newZ = newX ^ newY;
	newX.m_w = 0.f;
	newY.m_w = 0.f;
	newZ.m_w = 0.f;
	CMatrix OPBase(newX, newY, newZ, CVector(0, 0, 0, 1), false);
	OPBase.SetAffinePart(first);

	CMatrix OPBaseInv;
	OPBase.GetInverse(OPBaseInv);

	CVector O = OPBaseInv * first;
	CVector C = OPBaseInv * GetTM().GetPosition();
	C.m_y = 0.f;
	CVector P = OPBaseInv * last;
	CBox& box2 = static_cast<CBox&>(lastPositionBox);

	// search closest cylinder center point into box
	vector<CVector> qpoints;
	qpoints.push_back(P + CVector( box2.GetDimension().m_x / 2.f, 0,  box2.GetDimension().m_z / 2.f));
	qpoints.push_back(P + CVector( box2.GetDimension().m_x / 2.f, 0, -box2.GetDimension().m_z / 2.f));
	qpoints.push_back(P + CVector(-box2.GetDimension().m_x / 2.f, 0,  box2.GetDimension().m_z / 2.f));
	qpoints.push_back(P + CVector(-box2.GetDimension().m_x / 2.f, 0, -box2.GetDimension().m_z / 2.f));
	CVector Q = qpoints[0];
	float minDistance = (qpoints[0] - C).Norm();
	for(int i = 1; i < qpoints.size(); i++){
		//CVector Qi = box2.GetTM() * qpoints[i];
		CVector Qi = qpoints[i];
		float distance = (Qi - C).Norm();
		if (i == 0)
			minDistance = distance;
		else if (distance < minDistance) {
			distance = minDistance;
			Q = Qi;
		}
	}

	float OC = (C - O).Norm();
	float sin_beta = m_fRadius / OC;
	if (sin_beta > 1)
		sin_beta = 1.f;
	float beta = asin(sin_beta);
	float OP_OC = (P - O) * (C - O);
	float OP = (P - O).Norm();
	float d = OP * OC;
	float cosbeta2 = OP_OC / d;
	if (cosbeta2 > 1.f)
		cosbeta2 = 1.f;
	float beta2 = acos(cosbeta2);
	float beta1 = beta - beta2;
	float OH2 = OC * OC - m_fRadius * m_fRadius;
	if (OH2 < 0)
		OH2 = 0;
 	float OH = sqrt(OH2);
	float Hx = OH * cos(beta1);
	float Hz = OH * sin(beta1);
	CVector H(Hx, 0.f, Hz);
	float OQ_OH = (Q - O) * (H - O);
	float Rx, Rz;
	CVector Rlocal;
	if (OH == 0) {		
		float cos_gamma = OP_OC / (OC * OP);
		if (cos_gamma > 1.f)
			cos_gamma = 1.f;
		float sin_gamma = sqrt(1 - cos_gamma * cos_gamma);
		float OR = OP * sin_gamma;
		Rx = OR * (1 - cos_gamma * cos_gamma);
		Rz = OR * cos_gamma * (1 - cos_gamma * cos_gamma);
	}
	else {
		float OR = OQ_OH / OH;
		Rx = Hx * OR / OH;
		Rz = Hz * OR / OH;
	}
	if (isnan(Rx) || isnan(Rz))
		Rx = Rx;
	Rlocal.Fill(Rx, 0, Rz, 1.f);
	R = OPBase * Rlocal;	

	return eFace;
}

void CCylinder::GetBBoxDimension(CVector& dim)
{ 
	dim.m_x = m_fRadius * 2.f;
	dim.m_y = m_fHeight;
	dim.m_z = m_fRadius * 2.f;
}

const CVector& CCylinder::GetBBoxDimension() const
{
	return m_oDimension;
}