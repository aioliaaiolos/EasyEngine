#include "box.h"
#include "IRenderer.h"
#include "Cylinder.h"
#include "Exception.h"

CBox::CBox():
m_bInitialized( false )
{
}

CBox::CBox( CVector& oMinPoint, CVector& oDimension ):
m_oMinPoint( oMinPoint ),
m_oDimension( oDimension ),
m_bInitialized( true )
{
	m_fBoundingSphereRadius = ComputeBoundingSphereRadius();
}

CBox::CBox( const CBox& oBox )
{
	m_oMinPoint = oBox.m_oMinPoint;
	m_oDimension = oBox.m_oDimension;
	m_oTM = oBox.m_oTM;
	m_sName = oBox.m_sName;
}

void CBox::Set( const CVector& oMinPoint, const CVector& oDimension )
{
	m_oMinPoint = oMinPoint;
	m_oDimension = oDimension;
	m_fBoundingSphereRadius = ComputeBoundingSphereRadius();
	m_bInitialized = true;
}

void CBox::GetCenter( CVector& oCenter ) const
{
	oCenter = m_oMinPoint +  m_oDimension / 2.f;
}

void CBox::SetX(float x)
{
	m_oDimension.m_x = x;
}

void CBox::SetY(float y)
{
	m_oDimension.m_y = y;
}

void CBox::SetZ(float z)
{
	m_oDimension.m_z = z;
}

void CBox::AddPoint( const CVector& p )
{
	if( !m_bInitialized )
	{
		m_oMinPoint = p;
		m_bInitialized = true;
	}

	CVector vMax = m_oMinPoint + m_oDimension;
	if( vMax.m_x < p.m_x )
		vMax.m_x = p.m_x;
	if( vMax.m_y < p.m_y )
		vMax.m_y = p.m_y;
	if( vMax.m_z < p.m_z )
		vMax.m_z = p.m_z;

	if ( p.m_x < m_oMinPoint.m_x )
		m_oMinPoint.m_x = p.m_x;
	if ( p.m_y < m_oMinPoint.m_y )
		m_oMinPoint.m_y = p.m_y;
	if ( p.m_z < m_oMinPoint.m_z )
		m_oMinPoint.m_z = p.m_z;

	
	m_oDimension = vMax - m_oMinPoint;
	m_fBoundingSphereRadius = ComputeBoundingSphereRadius();
}

float CBox::ComputeBoundingSphereRadius() const
{
	CVector oCenter;
	GetCenter( oCenter );
	float fBoundingSphereRadius = ( m_oMinPoint - oCenter ).Norm();
	return fBoundingSphereRadius;
}

float CBox::ComputeBoundingCylinderRadius( TAxis eGeneratorAxis ) const
{
	CVector oCenter;
	float fRadius;
	switch( eGeneratorAxis )
	{
	case eAxisX:
		oCenter = m_oMinPoint + CVector( 0.f, m_oDimension.m_y / 2.f, m_oDimension.m_z / 2.f );
		break;
	case eAxisY:
		oCenter = m_oMinPoint + CVector( m_oDimension.m_x / 2.f, 0.f, m_oDimension.m_z / 2.f );
		break;
	case eAxisZ:
		oCenter = m_oMinPoint + CVector( m_oDimension.m_x / 2.f, m_oDimension.m_y / 2.f, 0.f );
		break;
	}
	fRadius = ( oCenter - m_oMinPoint ).Norm();
	return fRadius;
}

const CVector& CBox::GetBase() const
{
	return m_oMinPoint;
}

IGeometry* CBox::Duplicate()
{
	return new CBox(*this);
}

float CBox::GetHeight() const
{
	return m_oDimension.m_y;
}

void CBox::Transform(const CMatrix& tm)
{
	CVector oMaxPoint = m_oMinPoint + m_oDimension;
	m_oMinPoint = tm * m_oMinPoint;
	oMaxPoint = tm * oMaxPoint;
	m_oDimension = CVector(abs(oMaxPoint.m_x - m_oMinPoint.m_x), abs(oMaxPoint.m_y - m_oMinPoint.m_y), abs(oMaxPoint.m_z - m_oMinPoint.m_z));
}


bool CBox::TestBoxesCollisionIntoFirstBoxBase(const IBox& b1, const IBox& b2) const
{
	CMatrix b1Mat, b2Mat;
	b1.GetTM(b1Mat);
	b2.GetTM(b2Mat);
	CMatrix b1MatInv;
	b1Mat.GetInverse(b1MatInv);
	CMatrix b2MatBaseB1 = b1MatInv * b2Mat;

	CBox b2Temp;
	b2Temp.Set(b2.GetMinPoint(), b2.GetDimension());
	b2Temp.SetTM(b2.GetTM());

	b2Temp.SetTM(b2MatBaseB1);
	vector< CVector > vPoints2;
	b2Temp.GetPoints(vPoints2);
	float fMinx = CVector::GetMinx(vPoints2);
	if (fMinx > b1.GetMinPoint().m_x + b1.GetDimension().m_x)
		return false;
	float fMiny = CVector::GetMiny(vPoints2);
	if (fMiny > b1.GetMinPoint().m_y + b1.GetDimension().m_y)
		return false;
	float fMinz = CVector::GetMinz(vPoints2);
	if (fMinz > b1.GetMinPoint().m_z + b1.GetDimension().m_z)
		return false;
	float fMaxx = CVector::GetMaxx(vPoints2);
	if (fMaxx < b1.GetMinPoint().m_x)
		return false;
	float fMaxy = CVector::GetMaxy(vPoints2);
	if (fMaxy < b1.GetMinPoint().m_y)
		return false;
	float fMaxz = CVector::GetMaxz(vPoints2);
	if (fMaxz < b1.GetMinPoint().m_z)
		return false;
	return true;
}


float CBox::GetDistance(const IGeometry& oGeometry) const
{
	const CBox* pBox = dynamic_cast<const CBox*>(&oGeometry);
	if (pBox)
		return GetDistance(*pBox);
	return -1.f;
}

float CBox::GetDistance(const CBox& oBox) const
{
	float d1 = GetDistanceInBase(oBox);
	float d2 = ((CBox&)oBox).GetDistanceInBase(*this);
	return min(d1, d2);
}

float CBox::GetDistance(const ICylinder& oCylinder) const
{
	return -1.f;
}

float CBox::GetDistanceInBase(const IBox& oBox) const
{
	CMatrix b1Mat, b2Mat;
	GetTM(b1Mat);
	oBox.GetTM(b2Mat);
	CMatrix b1MatInv;
	b1Mat.GetInverse(b1MatInv);
	CMatrix b2MatBaseB1 = b1MatInv * b2Mat;

	CBox b2Temp;
	b2Temp.Set(oBox.GetMinPoint(), oBox.GetDimension());
	b2Temp.SetTM(oBox.GetTM());

	b2Temp.SetTM(b2MatBaseB1);
	vector< CVector > vPoints2;
	b2Temp.GetPoints(vPoints2);
	float fMinx = CVector::GetMinx(vPoints2);

	float distance = -1.f;
	if (fMinx > GetMinPoint().m_x + GetDimension().m_x)
		distance = fMinx - GetMinPoint().m_x + GetDimension().m_x;
	float fMiny = CVector::GetMiny(vPoints2);

	if (fMiny > GetMinPoint().m_y + GetDimension().m_y) {
		float d = fMiny - GetMinPoint().m_y + GetDimension().m_y;
		if (d < distance)
			distance = d;
	}

	float fMinz = CVector::GetMinz(vPoints2);
	if (fMinz > GetMinPoint().m_z + GetDimension().m_z) {
		float d = fMinz - GetMinPoint().m_z + GetDimension().m_z;
		if (d < distance)
			distance = d;
	}
		
	float fMaxx = CVector::GetMaxx(vPoints2);
	if (fMaxx < GetMinPoint().m_x){
		float d = fMaxx - GetMinPoint().m_x;
		if (d < distance)
			distance = d;
	}
	
	float fMaxy = CVector::GetMaxy(vPoints2);
	if (fMaxy < GetMinPoint().m_y) {
		float d = fMaxy - GetMinPoint().m_y;
		if (d < distance)
			distance = d;
	}
		
	float fMaxz = CVector::GetMaxz(vPoints2);
	if (fMaxz < GetMinPoint().m_z) {
		float d = fMaxz - GetMinPoint().m_z;
		if (d < distance)
			distance = d;
	}
		
	return distance;
}

float CBox::GetBoundingSphereRadius() const
{
	return m_fBoundingSphereRadius;
}

const IPersistantObject& CBox::operator >> (CBinaryFileStorage& store) const
{
	store << (int)eBox << m_oTM << m_oMinPoint << m_oDimension << m_fBoundingSphereRadius << m_sName;
	return *this;
}

IPersistantObject& CBox::operator << (CBinaryFileStorage& store)
{
	store >> m_oTM >> m_oMinPoint >> m_oDimension >> m_fBoundingSphereRadius >> m_sName;
	return *this;
}

const IPersistantObject& CBox::operator >> (CAsciiFileStorage& store) const
{
	store << "Min point : " << m_oMinPoint << ", dimension : " << m_oDimension << ", radius : " << m_fBoundingSphereRadius << "\n";
	return *this;
}

IPersistantObject& CBox::operator << (CAsciiFileStorage& store)
{

	return *this;
}

const IPersistantObject& CBox::operator >> (CStringStorage& store) const
{
	return *this;
}

IPersistantObject& CBox::operator << (CStringStorage& store)
{
	return *this;
}

const CVector& CBox::GetMinPoint() const
{
	return m_oMinPoint;
}
	
void CBox::SetMinPoint(const CVector& oMinPoint)
{
	m_oMinPoint = oMinPoint;
}

void CBox::GetTM( CMatrix& m ) const
{
	m = m_oTM;
}

const CMatrix& CBox::GetTM() const
{
	return m_oTM;
}

void CBox::SetTM(const CMatrix& m)
{
	m_oTM = m;
}

const CVector& CBox::GetDimension() const
{
	return m_oDimension;
}

void CBox::GetDimension(CVector& dim) const
{
	dim = m_oDimension;
}

IBox& CBox::operator=( const IBox& oBox )
{
	m_bInitialized = true;
	m_fBoundingSphereRadius = oBox.GetBoundingSphereRadius();
	m_oMinPoint = oBox.GetMinPoint();
	oBox.GetTM(m_oTM);
	m_oDimension = oBox.GetDimension();
	return *this;
}

void CBox::GetPoints( vector< CVector >& vPoints )
{
	vector< CVector > vTemp;
	vTemp.push_back( m_oMinPoint );
	vTemp.push_back( m_oMinPoint + CVector( m_oDimension.m_x, 0, 0 ) );
	vTemp.push_back( m_oMinPoint + CVector( 0, m_oDimension.m_y, 0 ) );
	vTemp.push_back( m_oMinPoint + CVector( 0, 0, m_oMinPoint.m_z ) );
	vTemp.push_back( m_oMinPoint + CVector( m_oDimension.m_x, m_oDimension.m_y, 0 ) );
	vTemp.push_back( m_oMinPoint + CVector( m_oDimension.m_x, 0, m_oDimension.m_z ) );
	vTemp.push_back( m_oMinPoint + CVector( 0, m_oDimension.m_y, m_oDimension.m_z ) );
	vTemp.push_back( m_oMinPoint + CVector( m_oDimension.m_x, m_oDimension.m_y, m_oDimension.m_z ) );
	for( int i = 0; i < vTemp.size(); i++ )
	{
		CVector v = m_oTM * vTemp[ i ];
		vPoints.push_back( v );
	}
}

void CBox::GetBBoxPoints(vector< CVector >& vPoints)
{
	GetPoints(vPoints);
}

void CBox::GetCenterPoints(vector< CVector >& vPoints) const
{
	vector< CVector > vTemp;
	vTemp.push_back(m_oMinPoint + CVector(0, m_oDimension.m_y / 2.f, 0));	
	vTemp.push_back(m_oMinPoint + CVector(m_oDimension.m_x, m_oDimension.m_y / 2.f, 0));
	vTemp.push_back(m_oMinPoint + CVector(0, m_oDimension.m_y / 2.f, m_oDimension.m_z));
	vTemp.push_back(m_oMinPoint + CVector(m_oDimension.m_x, m_oDimension.m_y / 2.f, m_oDimension.m_z));
	for (int i = 0; i < vTemp.size(); i++)
	{
		CVector v = m_oTM * vTemp[i];
		vPoints.push_back(v);
	}
}

bool CBox::IsIntersect(const CBox& box) const
{
	if (TestBoxesCollisionIntoFirstBoxBase(*this, box))
		return TestBoxesCollisionIntoFirstBoxBase(box, *this);
	return false;
}

bool CBox::IsIntersect(const IGeometry& box) const
{
	const CBox* pBox = dynamic_cast<const CBox*>(&box);
	if (pBox)
		return IsIntersect(*pBox);
	else {
		const CCylinder* pCylinder = dynamic_cast<const CCylinder*>(&box);
		if (pCylinder)
			return pCylinder->IsIntersect(*this);
	}
	return false;
}

void CBox::Draw(IRenderer& oRenderer) const
{
	oRenderer.DrawBox(m_oMinPoint, m_oDimension);
}

bool IsComplexCollision(IBox& box1, IBox& box2)
{
	if ( (box1.GetDimension().m_x > 0.5f * box2.GetDimension().m_x) && 
		 (box1.GetDimension().m_x < 2.f  * box2.GetDimension().m_x) &&
		 (box1.GetDimension().m_z > 0.5f * box2.GetDimension().m_z) && 
		 (box1.GetDimension().m_z < 2.f  * box2.GetDimension().m_z)) {
		return false;
	}
	return true;
}

IGeometry::TFace CBox::GetReactionYAlignedBox(IGeometry& firstPositionBox, IGeometry& lastPositionBox, CVector& R)
{
	CBox& box1 = static_cast<CBox&>(firstPositionBox);
	CBox& box2 = static_cast<CBox&>(lastPositionBox);
	CMatrix oTMInv;
	m_oTM.GetInverse(oTMInv);
	CVector box2Position = oTMInv * box2.GetTM().GetPosition();

	if (!IsComplexCollision(box1, *this)) {
		float dFirst = (box1.GetTM().GetPosition() - m_oTM.GetPosition()).Norm();
		float dLast  = (box2.GetTM().GetPosition() - m_oTM.GetPosition()).Norm();
		if (dLast < dFirst) {
			R = firstPositionBox.GetTM().GetPosition();
			return eFace;
		}
		else {
			return eNone;
		}
	}
	
	float margin = (box2.GetTM().GetPosition() - box1.GetTM().GetPosition()).Norm();
	
	bool collision = false;
	TFace face = eNone;

	float dimx = m_oDimension.m_x / 2.f;
	float dimy = m_oDimension.m_y / 2.f;
	float dimz = m_oDimension.m_z / 2.f;
	CVector RLocal;
	
	vector<CVector> box1PointsWorld;
	box1.GetCenterPoints(box1PointsWorld);

	vector<CVector> box2PointsWorld;
	box2.GetCenterPoints(box2PointsWorld);

	vector<CVector> box1PointsLocal, box2PointsLocal;
	for (int i = 0; i < box1PointsWorld.size(); i++) {
		CVector O = oTMInv * box1PointsWorld[i];
		CVector P = oTMInv * box2PointsWorld[i];
		box1PointsLocal.push_back(O);
		box2PointsLocal.push_back(P);
	}

	for (int i = 0; i < box1PointsLocal.size(); i++) {

		CVector O = box1PointsLocal[i];
		CVector P = box2PointsLocal[i];

		if ( box1.GetDimension().m_y  < dimy && O.m_y < dimy && (O.m_y + box1.GetDimension().m_y) > -dimy) {
			CVector A, B;
			// collision sur le plan x positif
			if (P.m_x > (dimx - margin - box1.GetDimension().m_z / 2.f) && P.m_x < dimx && P.m_x < O.m_x && P.m_z < dimz && P.m_z > -dimz) {
				int closedPointIndex = CVector::GetMinxIndex(box2PointsLocal);
				O = box1PointsLocal[closedPointIndex];
				P = box2PointsLocal[closedPointIndex];
				A = CVector(dimx, O.m_y, -dimz);
				B = CVector(dimx, O.m_y, dimz);
				RLocal = CVector(A.m_x + box1.GetDimension().m_z / 2.f, (O.m_y + P.m_y) / 2.f, box2Position.m_z);
				face = eXPositive;
				collision = true;
				break;
			}
			// collision sur le plan x négatif
			else if (P.m_x < (-dimx + margin + box1.GetDimension().m_z / 2.f) && P.m_x > -dimx && P.m_x > O.m_x && P.m_z < dimz && P.m_z > -dimz) {
				int closedPointIndex = CVector::GetMaxxIndex(box2PointsLocal);
				O = box1PointsLocal[closedPointIndex];
				P = box2PointsLocal[closedPointIndex];
				A = CVector(-dimx, O.m_y, -dimz);
				B = CVector(-dimx, O.m_y, dimz);
				RLocal = CVector(A.m_x - box1.GetDimension().m_z / 2.f, (O.m_y + P.m_y) / 2.f, box2Position.m_z);
				face = eXNegative;
				collision = true;
				break;
			}
			// collision sur le plan z positif
			else if (P.m_z > (dimz - margin - box1.GetDimension().m_z / 2.f) && P.m_z < dimz && P.m_z < O.m_z && P.m_x < dimx && P.m_x > -dimx) {
				int closedPointIndex = CVector::GetMinzIndex(box2PointsLocal);
				O = box1PointsLocal[closedPointIndex];
				P = box2PointsLocal[closedPointIndex];
				A = CVector(-dimx, O.m_y, dimz);
				B = CVector(dimx, O.m_y, dimz);
				RLocal = CVector(box2Position.m_x, (O.m_y + P.m_y) / 2.f, A.m_z + box1.GetDimension().m_z / 2.f);
				face = eZPositive;
				collision = true;
				break;
			}
			// collision sur le plan z négatif
			else if (P.m_z < (-dimz + margin + box1.GetDimension().m_z) && P.m_z > -dimz && P.m_z > O.m_z && P.m_x < dimx && P.m_x > -dimx) {
				int closedPointIndex = CVector::GetMinzIndex(box2PointsLocal);
				O = box1PointsLocal[closedPointIndex];
				P = box2PointsLocal[closedPointIndex];
				A = CVector(-dimx, O.m_y, -dimz);
				B = CVector(dimx, O.m_y, -dimz);
				RLocal = CVector(box2Position.m_x, (O.m_y + P.m_y) / 2.f, A.m_z - box1.GetDimension().m_z / 2.f);
				face = eZNegative;
				collision = true;
				break;
			}
		}
		else {
			O = oTMInv * firstPositionBox.GetTM().GetPosition();
			P = oTMInv * lastPositionBox.GetTM().GetPosition();
			if ( (O.m_y > dimy) ||
				((O.m_y < dimy) && (O.m_y + box1.GetDimension().m_y) > dimy) ) {
				RLocal = P;
				RLocal.m_y = dimy;
				face = eYPositive;
				collision = true;
				break;
			}
			else if ((O.m_y + box1.GetDimension().m_y) < -dimy) {
				face = eYNegative;
				collision = true;
				break;
			}
		}
	}
	if (collision)
		R = m_oTM * RLocal;	
	return face;
}

bool CBox::IsIncludedInto(const IGeometry& oGeometry)
{
	throw CMethodNotImplementedException("CBox::IsIncludedInto()");
	return true;
}

void CBox::GetBBoxDimension(CVector& dim)
{
	dim = m_oDimension;
}

IGeometry::TFace CBox::GetReactionYAlignedPlane(const CVector& firstPoint, const CVector& lastPoint, float planeHeight, CVector& R)
{
	CBox temp(*this);
	CMatrix m;
	temp.SetTM(m);
	CMatrix oTMInv;
	m_oTM.GetInverse(oTMInv);
	//oTMInv = m_oBackupInvTM;
	CVector O = oTMInv * firstPoint;
	CVector P = oTMInv * lastPoint;
	bool collision = true;
	TFace face = eNone;

	float dimx = m_oDimension.m_x / 2.f;
	float dimy = m_oDimension.m_y / 2.f;
	float dimz = m_oDimension.m_z / 2.f;
	float margin = 20.f;

	CVector RLocal;
	if (O.m_y < dimy && (O.m_y + planeHeight) > -dimy) {
		CVector A, B;
		// collision sur le plan x positif
		if (P.m_x > (dimx - margin) && P.m_x < dimx && P.m_x < O.m_x && P.m_z < dimz && P.m_z > -dimz) {
			A = CVector(dimx, O.m_y, -dimz);
			B = CVector(dimx, O.m_y, dimz);
			RLocal = CVector(A.m_x, (O.m_y + P.m_y) / 2.f, P.m_z);
			face = eXPositive;
		}
		// collision sur le plan x négatif
		else if (P.m_x < (-dimx + margin) && P.m_x > -dimx && P.m_x > O.m_x && P.m_z < dimz && P.m_z > -dimz) {
			A = CVector(-dimx, O.m_y, -dimz);
			B = CVector(-dimx, O.m_y, dimz);
			RLocal = CVector(A.m_x, (O.m_y + P.m_y) / 2.f, P.m_z);
			face = eXNegative;
		}
		// collision sur le plan z positif
		else if (P.m_z > (dimz - margin) && P.m_z < dimz && P.m_z < O.m_z && P.m_x < dimx && P.m_x > -dimx) {
			A = CVector(-dimx, O.m_y, dimz);
			B = CVector(dimx, O.m_y, dimz);
			RLocal = CVector(P.m_x, (O.m_y + P.m_y) / 2.f, A.m_z);
			face = eZPositive;
		}
		// collision sur le plan z négatif
		else if (P.m_z < (-dimz + margin) && P.m_z > -dimz && P.m_z > O.m_z && P.m_x < dimx && P.m_x > -dimx) {
			A = CVector(-dimx, O.m_y, -dimz);
			B = CVector(dimx, O.m_y, -dimz);
			RLocal = CVector(P.m_x, (O.m_y + P.m_y) / 2.f, A.m_z);
			face = eZNegative;
		}
		else
			collision = false;
	}
	else if (O.m_y > dimy) {
		RLocal = P;
		RLocal.m_y = dimy;
		face = eYPositive;
	}
	else if ((O.m_y + planeHeight) < -dimy) {
		face = eYNegative;
	}

	if (collision)
		R = m_oTM * RLocal;
	return face;
}
