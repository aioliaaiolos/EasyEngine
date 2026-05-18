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
class CBoxEntity;

using namespace std;

class IFighterEntity : virtual public IFighterEntityInterface
{

public:
	using THitEnemyCallback = function<void(IFighterEntity*)>;

	int							GetLife();
	void						SetLife(int nLife);
	void						IncreaseLife(int nLife);
	virtual void				ReceiveHit(IFighterEntity* pEnemy);
	virtual int					GetHitDamage() = 0;
	virtual void				MainHit() override;
	virtual void				SecondaryHit() override;	
	virtual void				Die() = 0;
	virtual void				ReceiveHit() = 0;
	virtual void				PlayHitAnimation() = 0;
	virtual void				PlaySecondaryHitAnimation() = 0;
	virtual bool				GetFightMode() const = 0;
	virtual void				SetFightMode(bool bFightMode) = 0;

protected:
	IFighterEntity();
	virtual void				OnHit();
	virtual void				Stand() = 0;
	virtual ICollisionManager&	GetCollisionManager() = 0;

	CBoxEntity*					m_pRHandBoxEntity = nullptr;
	bool						m_bHitEnemy;
	virtual CMatrix&			GetWorldTM() = 0;
	virtual ISphere*			GetBoneSphere(string sBoneName) = 0;
	virtual float				GetBoundingSphereRadius() = 0;
	virtual IMesh*				GetMesh() = 0;
	virtual IGeometry*			GetBoundingGeometry() = 0;
	virtual IGeometry*			GetAttackGeometry() = 0;
	virtual const CMatrix&		GetWeaponTM() const = 0;
	virtual void				SetHitEnemySphereCallback(THitEnemyCallback callback);
	virtual void				SetHitEnemyBoxCallback(THitEnemyCallback callback);

private:

	virtual IAnimation*			GetCurrentAnimation() = 0;
	virtual void				OnReceiveHit( IFighterEntity* pEnemy );
	virtual IFighterEntity*		GetFirstEnemy() = 0;
	virtual IFighterEntity*		GetNextEnemy() = 0;
	virtual bool				IsHitIntersectEnemySphere( IFighterEntity* pEnemy );
	virtual bool				IsHitIntersectEnemyBox( IFighterEntity* pEnemy );
	virtual void				OnEndHitAnimation();
	THitEnemyCallback			m_oHitEnemySphereCallback;
	THitEnemyCallback			m_oHitEnemyBoxCallback;
	int							m_nLife;
};

#endif // IFIGHTERENTITY_H