#include "Bone.h"
#include "Exception.h"
#include "IGeometry.h"

extern IGeometryManager* m_pGeometryManager;

CBone::CBone( IGeometryManager& oGeometryManager ) :
m_nLastKeyValue( 0 ),
m_nNextKeyValue( 1 ),
m_bEnd( false ),
m_fLastFrameInterpolatedFactor( 0 ),
m_pBoundingBox( NULL ),
m_oGeometryManager( oGeometryManager )
{
	m_pSphere = oGeometryManager.CreateSphere();
}


void CBone::NextKey()
{
	if ( m_nNextKeyValue < (int)m_mKeys[ m_sCurrentAnimation ].size() - 1 )
	{
		m_nLastKeyValue++;
		m_nNextKeyValue++;
	}
	else
	{
		if( !m_bEnd )
		{
			if( m_nNextKeyValue >= (int)m_mKeys[ m_sCurrentAnimation ].size() )
				m_nNextKeyValue = (int)m_mKeys[ m_sCurrentAnimation ].size() - 1;
			else
				m_nLastKeyValue++;
			m_bEnd = true;
		}
	}
}

void CBone::Rewind()
{
	m_bEnd = false;
	m_nLastKeyValue = 0;
	m_nNextKeyValue = 1;
}

void CBone::SetBoundKeys(int nFirstKey, int nLastKey)
{
	m_nLastKeyValue = nFirstKey;
	m_nNextKeyValue = nLastKey;
}

void CBone::AddKey( string sAnimation, int nTimeValue, const CMatrix& m, const CQuaternion& q )
{
	CKey oKey;
	oKey.m_oLocalTM = m;
	oKey.m_nTimeValue = nTimeValue;
	oKey.m_oQuat = q;
	m_mKeys[ sAnimation ].push_back( oKey );
}

void CBone::AddKey( string sAnimation, CKey& oKey )
{
	m_mKeys[ sAnimation ].push_back( oKey );
}

bool CBone::IsTimeToNextKey( float fTime )
{
	if( m_nNextKeyValue >= (int)m_mKeys[ m_sCurrentAnimation ].size() )
		return true;
	return fTime >= m_mKeys[ m_sCurrentAnimation ][ m_nNextKeyValue ].m_nTimeValue;
}

void CBone::UpdateTime( float fTime )
{
	if( m_mKeys[ m_sCurrentAnimation ].size() > 0 )
	{
		while(!m_bEnd && IsTimeToNextKey(fTime))
			NextKey();
		GetInterpolatedMatrix( fTime );
	}
	for( unsigned int iChild = 0; iChild < m_vChild.size(); iChild++ )
		m_vChild[ iChild ]->UpdateTime( fTime );
}

IBone* CBone::DuplicateHierarchy()
{
	CBone* pBone = new CBone( m_oGeometryManager );
	*pBone = *this;
	pBone->ClearChilds();
	for ( unsigned int iBone = 0; iBone < GetChildCount(); iBone++ )
	{
		CBone* pChild = dynamic_cast< CBone* >( GetChild( iBone ) );
		if (pChild) {
			IBone* pChildCopy = pChild->DuplicateHierarchy();
			pChildCopy->Link(pBone);
		}
	}
	return pBone;
}

CBone& CBone::operator=( CBone& oBone )
{
	(*this).CNode::operator = ( oBone );
	m_pBoundingBox = oBone.m_pBoundingBox;
	if( m_pBoundingBox )
		m_pSphere->SetRadius( m_pBoundingBox->GetBoundingSphereRadius() );
	return *this;
}

float CBone::GetInterpolatedFactor( float fTime )
{
	float t1 = (float)m_mKeys[ m_sCurrentAnimation ][ m_nLastKeyValue ].m_nTimeValue;
	float t2 = (float)m_mKeys[ m_sCurrentAnimation ][ m_nNextKeyValue ].m_nTimeValue;
	if( ( t1 - t2 ) == 0 )
		return 1;
	return ( t1 - fTime ) / ( t1 - t2 );
}

void CBone::GetInterpolatedMatrix( float fTime )
{
	float t = GetInterpolatedFactor( fTime );
	if( t >= 0 )
	{
		CKey& oLastKey = m_vCurrentAnimationKeys[m_nLastKeyValue];
		CKey& oNextKey = m_vCurrentAnimationKeys[m_nNextKeyValue];
		CMatrix& Ms = oLastKey.m_oLocalTM;
		CMatrix& Mf = oNextKey.m_oLocalTM;

		CMatrix MsInv;
		Ms.GetInverse( MsInv );
		CMatrix T = MsInv * Mf;
		CMatrix MI;

		CQuaternion qr;
		T.GetQuaternion(qr);
		CVector vAxis;
		float fAngle;
		qr.GetAngleAxis(vAxis, fAngle);
		CQuaternion qi(vAxis, fAngle * t);
		qi.GetMatrix(MI);

		CVector vs, vt, vi;
		T.GetAffinePart(vt);
		CVector::Lerp(vs, vt, t, vi);
		MI.SetAffinePart(vi);

		m_oLocalMatrix = Ms * MI;
	}
}

void CBone::GetRelativeInterpolatedMatrix( float fTime )
{
	float t = GetInterpolatedFactor( fTime );
	CMatrix& Ms = m_mKeys[ m_sCurrentAnimation ][ m_nLastKeyValue ].m_oLocalTM;
	CMatrix& Mf = m_mKeys[ m_sCurrentAnimation ][ m_nNextKeyValue ].m_oLocalTM;
	CMatrix MsInv;
	Ms.GetInverse( MsInv );
	CMatrix T = MsInv * Mf;
	CQuaternion qr;
	T.GetQuaternion(qr);
	CVector vAxis;
	float fAngle;
	qr.GetAngleAxis(vAxis, fAngle);
	CQuaternion qi( vAxis, fAngle * t );
	qi.GetMatrix( m_oLocalMatrix );
}

void CBone::GetRelativeInterpolatedMatrixSinceLastFrame( float fTime )
{
	float t = GetInterpolatedFactor( fTime );
	float tLast = m_fLastFrameInterpolatedFactor;
	if( t == 0.f )
		tLast = 0.f;
	CMatrix& Ms = m_mKeys[ m_sCurrentAnimation ][ m_nLastKeyValue ].m_oLocalTM;
	CMatrix& Mf = m_mKeys[ m_sCurrentAnimation ][ m_nNextKeyValue ].m_oLocalTM;
	CMatrix MsInv;
	Ms.GetInverse( MsInv );
	CMatrix T = MsInv * Mf;
	CQuaternion qr;
	T.GetQuaternion(qr);
	CVector vAxis;
	float fAngle;
	qr.GetAngleAxis(vAxis, fAngle);
	CQuaternion qi( vAxis, fAngle * ( t - tLast ) );
	qi.GetMatrix( m_oLocalMatrix );
	m_fLastFrameInterpolatedFactor = t;
}

void CBone::GetInterpolatedQuaternion( float fTime, CQuaternion& q )
{
	float t = GetInterpolatedFactor( fTime );
	CQuaternion::Slerp( m_mKeys[ m_sCurrentAnimation ][ m_nLastKeyValue ].m_oQuat, m_mKeys[ m_sCurrentAnimation ][ m_nNextKeyValue ].m_oQuat, t, q );
	q.Normalize();
}

void CBone::Yaw( float fAngle )
{
	CNode::Yaw( fAngle );
	for( unsigned int iKey = 0; iKey < m_mKeys[ m_sCurrentAnimation ].size(); iKey++ )
	{
		CMatrix& oTM = m_mKeys[ m_sCurrentAnimation ][ iKey ].m_oLocalTM ;
		oTM = oTM * CMatrix::GetyRotation( fAngle );
	}
}

void CBone::Pitch( float fAngle )
{
	CNode::Pitch( fAngle );
	for( unsigned int iKey = 0; iKey < m_mKeys[ m_sCurrentAnimation ].size(); iKey++ )
	{
		CMatrix& oTM = m_mKeys[ m_sCurrentAnimation ][ iKey ].m_oLocalTM ;
		oTM = oTM * CMatrix::GetxRotation( fAngle );
	}
}


void CBone::Roll( float fAngle )
{
	CNode::Roll( fAngle );
	if( m_mKeys[ m_sCurrentAnimation ].size() > 0 )
	{
		vector< CKey > vBakKeys;
		for( unsigned int iKey = 0; iKey < m_mKeys[ m_sCurrentAnimation ].size(); iKey++ )
			vBakKeys.push_back( m_mKeys[ m_sCurrentAnimation ][ iKey ] );

		CKey oFirstBakKey = vBakKeys[ 0 ];
		CMatrix oFirstBakKeyInvTM;
		oFirstBakKey.m_oLocalTM.GetInverse( oFirstBakKeyInvTM );
		for( unsigned int iKey = 0; iKey < m_mKeys[ m_sCurrentAnimation ].size(); iKey++ )
		{
			CMatrix& oTM = m_mKeys[ m_sCurrentAnimation ][ iKey ].m_oLocalTM;
			CMatrix oLastTM;
			CMatrix P = oFirstBakKeyInvTM * vBakKeys[ iKey ].m_oLocalTM;
			oTM = oFirstBakKey.m_oLocalTM * CMatrix::GetzRotation( fAngle ) * P;
		}
	}
}

void CBone::GetKeyByIndex( int nIndex, CKey& oKey ) const
{
	map< string, vector< CKey > >::const_iterator itAnim = m_mKeys.find( m_sCurrentAnimation );
	oKey = itAnim->second.at( nIndex );
}

void CBone::GetKeys(map< string, vector< CKey > >& mKeys) const
{
	mKeys = m_mKeys;
}

void CBone::SetKeys(const map< string, vector< CKey > >& mKeys)
{
	m_mKeys = mKeys;
}


void CBone::GetKeyByTime( int nTime, CKey& oKey ) const
{
	map< string, vector< CKey > >::const_iterator itAnim = m_mKeys.find( m_sCurrentAnimation );
	for( unsigned int iIndex = 0; iIndex < itAnim->second.size(); iIndex++ )
	{
		if( nTime == itAnim->second.at( iIndex ).m_nTimeValue )
		{
			oKey = itAnim->second.at( iIndex );
			return;
		}
	}
	ostringstream oss;
	oss << "Erreur lors de l'appel de CBone::GetKeyByTime( " << nTime << " ) : clé inexistante au temps demandé.";
	CEException e( oss.str() );
	throw e;
}

int CBone::GetKeyCount() const
{
	map< string, vector< CKey > >::const_iterator itAnim = m_mKeys.find( m_sCurrentAnimation );
	return itAnim->second.size();
}

void CBone::SetCurrentAnimation( string sAnimation )
{
	m_sCurrentAnimation = sAnimation;
	m_vCurrentAnimationKeys = m_mKeys[m_sCurrentAnimation];
	Rewind();
	for( unsigned int i = 0; i < m_vChild.size(); i++ )
	{
		IBone* pChild = dynamic_cast< IBone* >( m_vChild[ i ] );
		if( pChild )
			pChild->SetCurrentAnimation( sAnimation );
	}
}

void CBone::SetBoundingBox( IBox* pBox )
{
	m_pBoundingBox = pBox;
	m_pSphere->SetRadius( m_pBoundingBox->GetBoundingSphereRadius() );
}

const IBox* CBone::GetBoundingBox()
{
	return m_pBoundingBox;
}

const ISphere* CBone::GetBoundingSphere()
{
	CVector oLocalPosition, oWorldPosition;
	GetWorldPosition( oLocalPosition );
	m_pSphere->SetCenter( m_oWorldMatrix * oLocalPosition );
	return m_pSphere;
}



IBone* CBone::GetChildBoneByID(int nID)
{
	if (m_nID == nID)
		return this;
	else
	{
		IBone* pNodeRet = NULL;
		int i = 0;
		while (!pNodeRet && i < GetChildCount())
		{
			IBone* pChildBone = dynamic_cast<IBone*>(GetChild(i));
			if(pChildBone)
				pNodeRet = pChildBone->GetChildBoneByID(nID);
			i++;
		}
		return pNodeRet;
	}
}

IBone* CBone::GetChildBoneByName(string sName)
{
	if (m_sName == sName)
		return this;
	else
	{
		IBone* pNodeRet = NULL;
		int i = 0;
		while (!pNodeRet && i < GetChildCount())
		{
			IBone* pChildBone = dynamic_cast<IBone*>(GetChild(i));
			if(pChildBone)
				pNodeRet = pChildBone->GetChildBoneByName(sName);
			i++;
		}
		return pNodeRet;
	}
}

//const CBox& CBone::GetBoundingBox()
//{ 
//	return m_pBoundingBox;
//}

