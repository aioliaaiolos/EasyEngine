#include "Math/vector.h"
#include "FighterEntity.h"

class ICollisionManager;
class ISphere;
class IBone;
class IMesh;
class IAnimation;
class IGeometryManager;

class IAEntity : public virtual IFighterEntity, public IAEntityInterface
{
friend class CFightSystem;


public:
	IAEntity();
	virtual void				Goto(const CVector& oPosition, float fSpeed);

	void						Update();


protected:

	bool						IsArrivedAtDestination();
	static void 				OnCollision(IAEntity* pEntity);
	virtual void				UpdateGoto();

private:
	enum TFightState
	{
		eNoFight = 0,
		eBeginHitReceived,
		eReceivingHit,
		eBeginGoToEnemy,
		eGoingToEnemy,
		eArrivedToEnemy,
		eBeginLaunchAttack,
		eLaunchingAttack,
		eEndLaunchAttack,
		eBeginPrepareForNextAttack,
		ePreparingForNextAttack,
		eEndFight
	};

	TFightState					m_eFightState;
	int							m_nRecoveryTime;
	int							m_nBeginWaitTimeBeforeNextAttack;
	int							m_nCurrentWaitTimeBeforeNextAttack;
	bool						m_bHitEnemy;
	string						m_sCurrentHitBoneName;
	CVector						m_oDestination;
	float						m_fAngleRemaining;
	bool						m_bArriveAtDestination;
	bool						m_bFaceToTarget;
	IFighterEntity*				m_pCurrentEnemy;
	vector< CVector2D >			m_vCurrentPath;
	int							m_nCurrentPathPointNumber;
	float						m_fDestinationDeltaRadius;

	
	void						UpdateFaceTo();
	void						UpdateFightState();
	float						GetAngleBetween2Vectors(CVector& v1, CVector& v2);
	float						GetDestinationAngleRemaining();
	void						TurnFaceToDestination();
	void						OnReceiveHit( IFighterEntity* pAgressor );
	void						OnEndHitAnimation();
	void						FaceTo(const CVector& point);
	void						Attack(IFighterEntity* pEntity);
	void						Attack(IFighterEntityInterface* pEntity);
	virtual void				ComputePathFind2D( const CVector2D& oOrigin, const CVector2D& oDestination, vector< CVector2D >& vPoints) = 0;
	virtual void				SetDestination( const CVector& oDestination );

	virtual void				Turn( float fAngle ) = 0;
	virtual IAnimation*			GetCurrentAnimation() = 0;
	virtual float				GetDistanceTo2dPoint( const CVector& oPosition ) = 0;
	virtual void				Run() = 0;
	virtual void				Guard() = 0;
	virtual IBox*				GetFirstCollideBox() = 0;
	virtual IBox*				GetNextCollideBox() = 0;

	static void					OnHitReceivedCallback( IAnimation::TEvent e, void* pData );
};