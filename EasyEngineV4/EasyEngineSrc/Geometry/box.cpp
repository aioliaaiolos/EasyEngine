#include "box.h"

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
	float fBoundingSphereRadius = 2 * ( m_oMinPoint - oCenter ).Norm();
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

void CBox::GetBase(CVector& oBase)
{
	oBase = m_oMinPoint;
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


bool CBox::TestBoxesCollisionIntoFirstBoxBase(const IBox& b1, const IBox& b2)
{
	CMatrix b1Mat, b2Mat;
	b1.GetTM(b1Mat);
	b2.GetTM(b2Mat);
	CMatrix b1MatInv;
	b1Mat.GetInverse(b1MatInv);
	CMatrix b2MatBaseB1 = b1MatInv * b2Mat;

	CBox b2Temp;
	b2Temp.Set(b2.GetMinPoint(), b2.GetDimension());
	b2Temp.SetWorldMatrix(b2.GetTM());

	b2Temp.SetWorldMatrix(b2MatBaseB1);
	vector< CVector > vPoints2;
	b2Temp.GetPoints(vPoints2);
	float fMinx = GetMinx(vPoints2);
	if (fMinx > b1.GetMinPoint().m_x + b1.GetDimension().m_x)
		return false;
	float fMiny = GetMiny(vPoints2);
	if (fMiny > b1.GetMinPoint().m_y + b1.GetDimension().m_y)
		return false;
	float fMinz = GetMinz(vPoints2);
	if (fMinz > b1.GetMinPoint().m_z + b1.GetDimension().m_z)
		return false;
	float fMaxx = GetMaxx(vPoints2);
	if (fMaxx < b1.GetMinPoint().m_x)
		return false;
	float fMaxy = GetMaxy(vPoints2);
	if (fMaxy < b1.GetMinPoint().m_y)
		return false;
	float fMaxz = GetMaxz(vPoints2);
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
	b2Temp.SetWorldMatrix(oBox.GetTM());

	b2Temp.SetWorldMatrix(b2MatBaseB1);
	vector< CVector > vPoints2;
	b2Temp.GetPoints(vPoints2);
	float fMinx = GetMinx(vPoints2);

	float distance = -1.f;
	if (fMinx > GetMinPoint().m_x + GetDimension().m_x)
		distance = fMinx - GetMinPoint().m_x + GetDimension().m_x;
	float fMiny = GetMiny(vPoints2);

	if (fMiny > GetMinPoint().m_y + GetDimension().m_y) {
		float d = fMiny - GetMinPoint().m_y + GetDimension().m_y;
		if (d < distance)
			distance = d;
	}

	float fMinz = GetMinz(vPoints2);
	if (fMinz > GetMinPoint().m_z + GetDimension().m_z) {
		float d = fMinz - GetMinPoint().m_z + GetDimension().m_z;
		if (d < distance)
			distance = d;
	}
		
	float fMaxx = GetMaxx(vPoints2);
	if (fMaxx < GetMinPoint().m_x){
		float d = fMaxx - GetMinPoint().m_x;
		if (d < distance)
			distance = d;
	}
	
	float fMaxy = GetMaxy(vPoints2);
	if (fMaxy < GetMinPoint().m_y) {
		float d = fMaxy - GetMinPoint().m_y;
		if (d < distance)
			distance = d;
	}
		
	float fMaxz = GetMaxz(vPoints2);
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
	store << (int)eBox << m_oTM << m_oMinPoint << m_oDimension << m_fBoundingSphereRadius;
	return *this;
}

IPersistantObject& CBox::operator << (CBinaryFileStorage& store)
{
	store >> m_oTM >> m_oMinPoint >> m_oDimension >> m_fBoundingSphereRadius;
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
	
void CBox::GetTM( CMatrix& m ) const
{
	m = m_oTM;
}

const CMatrix& CBox::GetTM() const
{
	return m_oTM;
}

const CVector& CBox::GetDimension() const
{
	return m_oDimension;
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

void CBox::SetWorldMatrix( const CMatrix& oMatrix )
{
	m_oTM = oMatrix;
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

float CBox::GetMinx(const vector< CVector >& vPoints) const
{
	float fMin = vPoints[0].m_x;
	for (int i = 1; i < vPoints.size(); i++)
	{
		if (fMin > vPoints[i].m_x)
			fMin = vPoints[i].m_x;
	}
	return fMin;
}

float CBox::GetMiny(const vector< CVector >& vPoints) const
{
	float fMin = vPoints[0].m_y;
	for (int i = 1; i < vPoints.size(); i++)
	{
		if (fMin > vPoints[i].m_y)
			fMin = vPoints[i].m_y;
	}
	return fMin;
}

float CBox::GetMinz(const vector< CVector >& vPoints) const
{
	float fMin = vPoints[0].m_z;
	for (int i = 1; i < vPoints.size(); i++)
	{
		if (fMin > vPoints[i].m_z)
			fMin = vPoints[i].m_z;
	}
	return fMin;
}

float CBox::GetMaxx(const vector< CVector >& vPoints) const
{
	float fMax = vPoints[0].m_x;
	for (int i = 1; i < vPoints.size(); i++)
	{
		if (fMax < vPoints[i].m_x)
			fMax = vPoints[i].m_x;
	}
	return fMax;
}

float CBox::GetMaxy(const vector< CVector >& vPoints) const
{
	float fMax = vPoints[0].m_y;
	for (int i = 1; i < vPoints.size(); i++)
	{
		if (fMax < vPoints[i].m_y)
			fMax = vPoints[i].m_y;
	}
	return fMax;
}

float CBox::GetMaxz(const vector< CVector >& vPoints) const
{
	float fMax = vPoints[0].m_z;
	for (int i = 1; i < vPoints.size(); i++)
	{
		if (fMax < vPoints[i].m_z)
			fMax = vPoints[i].m_z;
	}
	return fMax;
}

bool CBox::IsIntersect(const IBox& box)
{
	if (TestBoxesCollisionIntoFirstBoxBase(*this, box))
		return TestBoxesCollisionIntoFirstBoxBase(box, *this);
	return false;
}