
#include "Animation.h"
#include <windows.h> // GetTickCount()
#include "Utils2/TimeManager.h"

#define _USE_MATH_DEFINES
#include <math.h>
using namespace std;


CAnimation::CAnimation(EEInterface& oInterface):
IAnimation(oInterface),
m_oInterface(oInterface),
m_nCurrentAnimationTime( -1 ),
m_bLoop( true ),
m_fSpeed( 1.f ),
m_nStartAnimationTime( 0 ),
m_nEndAnimationTime( 16000 ),
m_bPause( false ),
m_bStart( false ),
m_nLastTickCount( 0 ),
m_nCurrentTickCount( 0 ),
m_pSkeletonRoot( NULL ),
m_oTimeManager(static_cast<CTimeManager&>(*oInterface.GetPlugin("TimeManager")))
{
}

void CAnimation::SetStartAnimationTime( int nTime )
{
	m_nStartAnimationTime = nTime;
}

void CAnimation::SetEndAnimationTime( int nTime )
{
	m_nEndAnimationTime = nTime;
}

void CAnimation::NextFrame()
{
	m_nCurrentAnimationTime += 160;
}

void CAnimation::AddBone( int nBoneID)
{
	vector< CKey > vKey;
	m_mBoneKeys.insert( map< int, vector< CKey > >::value_type( nBoneID, vKey ) );
}

void CAnimation::AddKey( int nBoneID, int nTimeValue, CKey::TKey eKeyType, const CMatrix& oLocalTM, const CMatrix& oWorldTM, const CQuaternion& q )
{
	CKey oKey;
	oKey.m_oLocalTM = oLocalTM;
	oKey.m_oWorldTM = oWorldTM;
	oKey.m_nTimeValue = nTimeValue;
	oKey.m_oQuat = q;
	oKey.m_eType = eKeyType;
	if(m_mBoneKeys[nBoneID].empty())
		m_mBoneKeys[nBoneID].push_back(oKey);
	else {
		for (vector< CKey >::iterator itKey = m_mBoneKeys[nBoneID].begin(); itKey != m_mBoneKeys[nBoneID].end(); itKey++) {
			CKey& k0 = *itKey;
			if (nTimeValue == k0.m_nTimeValue)
				break;
			vector< CKey  >::iterator itNextKey = itKey + 1;
			if (itNextKey != m_mBoneKeys[nBoneID].end()) {
				CKey& k1 = *itNextKey;
				if (nTimeValue > k0.m_nTimeValue && nTimeValue < k1.m_nTimeValue) {
					m_mBoneKeys[nBoneID].insert(itNextKey, oKey);
					break;
				}
			}
			else {
				m_mBoneKeys[nBoneID].push_back(oKey);
				break;
			}
		}
	}
}

void CAnimation::SetSkeleton( IBone* pBone )
{
	if (pBone) {
		if (!m_pSkeletonRoot)
			m_pSkeletonRoot = pBone;
		map< int, vector< CKey > >::iterator itBone = m_mBoneKeys.find(pBone->GetID());
		if (itBone != m_mBoneKeys.end())
		{
			m_mBones[pBone->GetID()] = pBone;
			vector< CKey >& vKeys = m_mBoneKeys[pBone->GetID()];
			for (unsigned int iKey = 0; iKey < vKeys.size(); iKey++)
				pBone->AddKey(m_sName, vKeys[iKey]);
		}
		for (unsigned int iBone = 0; iBone < pBone->GetChildCount(); iBone++)
			SetSkeleton(dynamic_cast<IBone*>(pBone->GetChild(iBone)));
	}
}

void CAnimation::UpdateAnimationTime()
{
	int nCurrentTickCount = m_oTimeManager.GetCurrentTimeInMillisecond();
	if (m_nCurrentAnimationTime == -1)
	{
		m_nCurrentAnimationTime = m_nStartAnimationTime;
		m_nLastTickCount = nCurrentTickCount;
	}
	if (m_bPause || !m_bStart)
		m_nLastTickCount = m_oTimeManager.GetCurrentTimeInMillisecond();

	int nDeltaTickCount = nCurrentTickCount - m_nLastTickCount;
	m_nLastTickCount = nCurrentTickCount;
	m_nCurrentAnimationTime += (int)((float)nDeltaTickCount * 4.f * m_fSpeed);
	if (m_nCurrentAnimationTime > m_nEndAnimationTime) {
		m_nCurrentAnimationTime = m_nEndAnimationTime;
		CallCallbacks(eBeginRewind);
		if (!m_bLoop)
			Stop();
		else {
			for (map< int, IBone* >::iterator itBoneKeys = m_mBones.begin(); itBoneKeys != m_mBones.end(); ++itBoneKeys)
				itBoneKeys->second->Rewind();
			m_nCurrentAnimationTime = m_nStartAnimationTime;
		}
	}
}

void CAnimation::Play( bool bLoop )
{
	m_nCurrentAnimationTime = -1;
	m_nLastTickCount = m_oTimeManager.GetCurrentTimeInMillisecond();
	m_bStart = true;
	m_bPause = false;
	m_bLoop = bLoop;
	m_pSkeletonRoot->SetCurrentAnimation( m_sName );
	CallCallbacks( ePlay );
}

bool CAnimation::GetPause()
{
	return m_bPause;
}

float CAnimation::GetSpeed()
{
	return m_fSpeed;
}

void CAnimation::SetSpeed( float fSpeed )
{
	m_fSpeed = fSpeed;
}

void CAnimation::Pause( bool bPause )
{
	m_bPause = bPause;
}

void CAnimation::Update()
{
	UpdateAnimationTime();
	if (!m_bPause) {
		m_pSkeletonRoot->UpdateTime((float)m_nCurrentAnimationTime);
		CallCallbacks(eAfterUpdate);
	}
}

void CAnimation::SetAnimationTime( int nTime )
{
	m_nCurrentAnimationTime = nTime;
}

int CAnimation::GetAnimationTime()
{
	return m_nCurrentAnimationTime;
}

void CAnimation::Stop()
{
	m_bPause = true;
}

int CAnimation::AddCallback(TCallback pCallback)
{
	m_vCallback.push_back(pCallback);
	return m_vCallback.size() - 1;
}

void CAnimation::CallCallbacks( TEvent e )
{
	for( unsigned int i = 0; i < m_vCallback.size(); i++ )
		if( m_vCallback[ i ])
			m_vCallback[ i ](e);
}


void CAnimation::RemoveCallback(int nCallbackIndex)
{
	m_vCallback.erase(m_vCallback.begin() + nCallbackIndex);
}

void CAnimation::RemoveAllCallback()
{
	m_vCallback.clear();
}

int CAnimation::GetStartAnimationTime()
{
	return m_nStartAnimationTime;
}

int CAnimation::GetEndAnimationTime()
{
	return m_nEndAnimationTime;
}

void CAnimation::GetBoneKeysMap( map< int, vector< CKey > >& mBoneKeys )
{
	mBoneKeys = m_mBoneKeys;
}

void CAnimation::SetBoneKeysMap(map< int, vector< CKey > >& mBoneKeys)
{
	m_mBoneKeys = mBoneKeys;
}

IAnimation*	CAnimation::CreateReversedAnimation()
{
	CAnimation* pReversedAnimation = new CAnimation(m_oInterface);
	pReversedAnimation->m_nEndAnimationTime = m_nEndAnimationTime - m_nStartAnimationTime;
	for (const pair<int, vector<CKey>>& bone : m_mBoneKeys) {
		pReversedAnimation->AddBone(bone.first);
		for (int i = bone.second.size() - 1; i >= 0; i--) {
			CKey key = bone.second[i];
			if( (key.m_nTimeValue >= m_nStartAnimationTime) && (key.m_nTimeValue <= m_nEndAnimationTime))
				pReversedAnimation->AddKey(bone.first, m_nEndAnimationTime - key.m_nTimeValue, key.m_eType, key.m_oLocalTM, key.m_oWorldTM, key.m_oQuat);
		}
	}
	return pReversedAnimation;
}