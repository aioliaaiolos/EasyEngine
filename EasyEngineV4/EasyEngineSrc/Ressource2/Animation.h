#ifndef ANIMATION_H
#define ANIMATION_H

// stl
#include <map>
#include <vector>


#include "IRessource.h"
#include "IEntity.h"
#include "Math/Matrix.h"
#include "Math/Quaternion.h"


using namespace std;

//#define OLD_METHOD

class CAnimation : public IAnimation
{

public:
	CAnimation(EEInterface& oInterface);
	void			AddBone(int nBoneID);
	void			AddKey(int nBoneID, int nTimeValue, CKey::TKey eKeyType, const CMatrix& oLocalTM, const CMatrix& oWorldTM, const CQuaternion& q);
	void			SetSkeleton(IBone* pBone);

	void			Play(bool bLoop);
	void			Pause(bool bPause);
	void			Stop();
	float			GetSpeed();
	void			SetSpeed(float fSpeed);
	void			Update();
	bool			GetPause();
	void			SetShader(IShader* pShader) {}
	IShader*		GetShader() const { return NULL; }
	void			SetStartAnimationTime(int nTime);
	void			SetEndAnimationTime(int nTime);
	void			NextFrame();
	void			SetAnimationTime(int nTime);
	int				GetAnimationTime();
	int				AddCallback(TCallback pCallback);
	void			RemoveCallback(int nCallbackIndex);
	void			RemoveAllCallback();
	int				GetStartAnimationTime();
	int				GetEndAnimationTime();
	void			GetBoneKeysMap(map< int, vector< CKey > >& mBoneKeys);
	IAnimation*		CreateReversedAnimation();

private:
	map<int, IBone*>							m_mBones;
	map<int, vector<CKey>>						m_mBoneKeys;
	IBone*										m_pSkeletonRoot;
	int											m_nLastTickCount;
	int											m_nCurrentTickCount;
	int											m_nCurrentAnimationTime;
	bool										m_bLoop;
	float										m_fSpeed;
	bool										m_bPause;
	bool										m_bStart;
	int											m_nStartAnimationTime;
	int											m_nEndAnimationTime;
	vector<TCallback>							m_vCallback;
	EEInterface&								m_oInterface;

	void										UpdateAnimationTime();
	void										CallCallbacks( TEvent e );

};

#endif // ANIMATION_H