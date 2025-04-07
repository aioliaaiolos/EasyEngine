#include "Math/vector.h"
#include "FighterEntity.h"

class ICollisionManager;
class ISphere;
class IBone;
class IMesh;
class IAnimation;
class IGeometryManager;

class IAEntity : virtual public IFighterEntity, virtual public IAEntityInterface
{
friend class CFightSystem;


public:
	IAEntity(EEInterface& oInterface);
	virtual void				Goto(const CVector& oPosition, float fSpeed);
	void						TalkTo(IFighterEntityInterface* pEntity, TalkToCallback callback) override;
	void						Update();
	//void						SetWeaponRange(float range);
	//float						GetWeaponRange();

	pair<TalkToCallback, IFighterEntityInterface*>	m_oTalkToCallback;

protected:

	enum TFightState
	{
		eNoFight = 0,
		eFlee,
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

	enum TTalkToState
	{
		eNoTalkTo = 0,
		eBeginGotoInterlocutor,
		eGoingToInterlocutor,
		eArrivedToInterlocutor,
		eTalkingTo
	};


	bool						IsArrivedAtDestination();
	static void 				OnCollision(IAEntity* pEntity);
	virtual void				UpdateGoto();
	virtual void				UpdateFlee();
	virtual void				UpdateTalkToState();
	virtual void				OpenTopicWindow() = 0;
	virtual INode*				GetParent() = 0;
	void						UpdateFaceTo();
	void						UpdateFightState();
	float						GetAngleBetween2Vectors(CVector& v1, CVector& v2);
	float						GetDestinationAngleRemaining();
	float						GetFleeAngleRemaining();
	void						TurnFaceToDestination();
	void						OnReceiveHit(IFighterEntity* pAgressor) override;
	void						OnEndHitAnimation();
	void						FaceTo(const CVector& point);
	void						Attack(IFighterEntity* pEntity);
	void						Attack(IFighterEntityInterface* pEntity);
	void						Flee();
	float						GetDistanceToEnemy();
	virtual void				ComputePathFind2D(const CVector& oOrigin, const CVector& oDestination, vector<CVector>& vPoints) = 0;
	virtual void				SetDestination(const CVector& oDestination);
	virtual void				Turn(float fAngle) = 0;
	virtual IAnimation*			GetCurrentAnimation() = 0;
	virtual float				GetDistanceTo2dPoint(const CVector& oPosition) = 0;
	virtual void				Run() = 0;
	virtual void				MoveToGuard() = 0;
	virtual void				Guard() = 0;
	virtual IBox*				GetFirstCollideBox() = 0;
	virtual IBox*				GetNextCollideBox() = 0;


	float						m_fForceDeltaRotation;
	float						m_fAngleRemaining;
	float						m_fDangerZone = 1000.f;
	float						m_fMinimumFleeDistance = 500.f;
	IFighterEntity*				m_pCurrentEnemy;
	TFightState					m_eFightState;
	TTalkToState				m_eTalkToState;
	int							m_nRecoveryTime;
	int							m_nBeginWaitTimeBeforeNextAttack;
	int							m_nCurrentWaitTimeBeforeNextAttack;
	bool						m_bHitEnemy;
	string						m_sCurrentHitBoneName;
	CVector						m_oDestination;
	bool						m_bArriveAtDestination;
	bool						m_bFaceToTarget;
	IFighterEntityInterface*	m_pCurrentInterlocutor;
	vector<CVector>				m_vCurrentPath;
	int							m_nCurrentPathPointNumber;
	float						m_fWeaponRange = 115.f;
	float						m_fDestinationDeltaRadius;
	CTimeManager&				m_oTimeManager;
	bool						m_bFightingIAEnabled = true;	
};