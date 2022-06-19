#ifndef IFIGHTERENTITY_H
#define IFIGHTERENTITY_H

#include <vector>

#include "IRessource.h"
#include "IEntity.h"

class ISphere;
class CVector;
class IMesh;
class IAnimation;
class IBbox;
class ICollisionManager;


using namespace std;

class IFighterEntity : public IFighterEntityInterface
{

public:
	int							GetLife();
	void						SetLife(int nLife);
	void						IncreaseLife(int nLife);
	virtual void				ReceiveHit(IFighterEntity* pEnemy);
	virtual void				MainHit() override;
	virtual void				SecondaryHit() override;

	virtual void				GetPosition(CVector& v) const = 0;
	virtual void				Die() = 0;
	virtual void				PlayReceiveHit() = 0;
	virtual void				PlayHitAnimation() = 0;
	virtual void				PlaySecondaryHitAnimation() = 0;

protected:
	IFighterEntity();
	virtual void				OnHit(IFighterEntity* pAgressor, string sHitBoneName);
	virtual void				Stand() = 0;
	virtual ICollisionManager&	GetCollisionManager() = 0;

	string						m_sCurrentHitBoneName;
	bool						m_bHitEnemy;
	virtual CMatrix&			GetWorldTM() = 0;
	virtual ISphere*			GetBoneSphere(string sBoneName) = 0;
	virtual float				GetBoundingSphereRadius() = 0;
	virtual IMesh*				GetMesh() = 0;
	virtual const string&		GetAttackBoneName() = 0;

private:
	int							m_nLife;

	virtual IAnimation*			GetCurrentAnimation() = 0;
	virtual void				OnReceiveHit( IFighterEntity* pEnemy );
	virtual IFighterEntity*		GetFirstEnemy() = 0;
	virtual IFighterEntity*		GetNextEnemy() = 0;
	virtual bool				IsHitIntersectEnemySphere( IFighterEntity* pEnemy );
	virtual bool				IsHitIntersectEnemyBox( IFighterEntity* pEnemy );
	virtual void				OnEndHitAnimation();	
	virtual IBox*				GetBoundingBox() = 0;	

	static void					OnHitAnimationCallback( IAnimation::TEvent e, void* pData );
	static void					OnHitReceivedAnimationCallback( IAnimation::TEvent e, void* pData );
};

#endif // IFIGHTERENTITY_H