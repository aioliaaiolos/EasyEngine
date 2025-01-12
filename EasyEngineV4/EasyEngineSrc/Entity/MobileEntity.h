#ifndef MOBILEENTITY_H
#define MOBILEENTITY_H

#include "Entity.h"
#include "FighterEntity.h"

class IFileSystem;
class CItem;
class CWeapon;

class CObject : public CEntity
{
public:
	CObject(EEInterface& oInterface);
	CObject(EEInterface& oInterface, string sFileName);
	void						Update() override;
	virtual void				ManageGravity();
	void						UpdateCollision() override;

protected:
	bool						m_bFirstUpdate;
	bool						m_bCollideOnObstacle;

	static void 				OnCollision(CEntity* pThis, vector<INode*> entities);

private:
	CVector						m_vNextLocalTranslate;
	
};

class CCharacter : public CObject, public virtual IFighterEntity, public virtual ICharacter
{

public:
	CCharacter(EEInterface& oInterface, string sFileName, string sID);
	virtual ~CCharacter();

	float													GetAnimationSpeed(IEntity::TAnimation eAnimationType);
	void													GetEntityInfos(ILoader::CObjectInfos*& pInfos);
	void													BuildFromInfos(const ILoader::CObjectInfos& infos, IEntity* pParent, bool bExcludeChildren = false) override;
	void													Save();
	void													SetMovmentSpeed(TAnimation eAnimationType, float fSpeed) override;
	TAnimation												GetCurrentAnimationType() const;
	void													RunAction(string sAction, bool bLoop);
	void													Die();
	void													WearArmorToDummy(string armorName);
	void													WearShoes(string shoesName) override;
	void													UnWearShoes(string shoesPath) override;
	void													UnWearAllShoes() override;
	void													SetHairs(string sHairsPath) override;
	void													SetBody(string sBodyName) override;
	void													Yaw(float fAngle);
	void													Pitch(float fAngle);
	void													Roll(float fAngle);
	IAnimation*												GetCurrentAnimation();
	void													AddItem(string sItemName);
	void													AddItem(CItem* pItem);
	void													RemoveItem(string sItemName);
	void													WearItem(string sItemID);
	void													WearItem(IItem* pItem);
	void													UnWearItem(string sItemID);
	void													UnWearItem(IItem* pItem);
	int														GetItemCount(string sItemID);
	const map<string, vector<IItem*>>&						GetItems() const override;
	void													SetFightMode(bool fightMode) override;
	bool													GetFightMode() override;
	void													SetCurrentWeapon(CWeapon* pWeapon);
	void													Link(INode* pParent) override;
	IBox*													GetBoundingBox();
	const string&											GetClass();
	void													SetClass(string sClassName);
	static void												InitStatics(IFileSystem& oFileSystem);
	static map<string, IEntity::TAnimation>					s_mStringToAnimation;

protected:
	typedef void (*TAction)( CCharacter*, bool );

	void													InitAnimations();
	void													InitBoundingBox(string sFileName);
	void													InitReverseAnimations();
	void													SetPredefinedAnimation(string s, bool bLoop, int nFrameNumber = 0);
	void 													Walk(bool bLoop);
	void 													Stand(bool bLoop);
	void 													Run(bool bLoop);
	void													RunReverse(bool bLoop);
	void 													Jump(bool bLoop);
	void													ReceiveHit(bool bLoop);
	void													Stand();
	void													HitWeapon();
	void													ReceiveHit();
	void													PlayHitAnimation();
	void													PlaySecondaryHitAnimation();
	void													MoveToGuard();
	void													MoveToGuardWeaponPart1();
	void													MoveToGuardWeaponPart2();
	void													MoveToGuardWeaponPart1Reverse();
	void													MoveToGuardWeaponPart2Reverse();
	void													Guard();
	void													TurnEyesH(float fValue);
	void													TurnNeckH(float f);
	IBone*													GetPreloadedBone(string sName);
	void													GetPosition(CVector& oPosition) const;
	IMesh*													GetMesh();
	IFighterEntity*											GetFirstEnemy();
	IFighterEntity*											GetNextEnemy();
	CMatrix&												GetWorldTM();
	float													GetBoundingSphereRadius();
	ICollisionManager&										GetCollisionManager() { return m_oCollisionManager; }
	ISphere*												GetBoneSphere(string sBoneName);
	void													AddSpeed(float x, float y, float z);
	IGeometry*												GetAttackGeometry();
	const string&											GetSecondaryAttackBoneName();
	IGeometry*												GetBoundingGeometry() override;
	void													Wear(string sClothPath, string sDummyName);
	void													Wear(CEntity* pEntity, string sDummyName);
	void													UnWear(CEntity* pCloth);
	void													SaveToJson();
	IAnimation*												CreateReverseAnimation(string sAnimationType);
	const CMatrix&											GetWeaponTM() const override;
	bool													GetFightMode() const override;
	int														GetHitDamage() override;
	IAnimation*												GetAnimation(TAnimation eAnimationType);
	void													SetAnimationSetSpeed();
	void													SetAnimationSpeed(TAnimation eAnimationType, float fSpeed);

	int														m_nStrength = 5.f;
	string													m_sFileNameWithoutExt;
	bool													m_bInitSkeletonOffset;
	CMatrix													m_oSkeletonOffset;
	TAnimation												m_eCurrentAnimationType;
	map< string, IBone* >									m_mPreloadedBones;
	IEntity*												m_pLeftEye;
	IEntity*												m_pRightEye;
	IBone*													m_pNeck;
	float													m_fEyesRotH;
	float													m_fEyesRotV;
	float													m_fNeckRotH;
	float													m_fNeckRotV;
	const float												m_fMaxEyeRotationH;
	const float												m_fMaxEyeRotationV;
	const float												m_fMaxNeckRotationH;
	const float												m_fMaxNeckRotationV;
	string													m_sAttackBoneName;
	string													m_sSecondaryAttackBoneName;
	CVector													m_vNextLocalTranslate;
	string													m_sStandAnimation;
	IBox*													m_pBBox;
	map<string, vector<IItem*>>								m_mItems;
	map< TAnimation, float >								m_mAnimationSpeedByType;
	string													m_sCurrentBodyName;
	bool													m_bFightMode = false;
	CWeapon*												m_pCurrentWeapon = nullptr;
	CBone*													m_pDummyRHand = nullptr;
	IGeometry*												m_pWeaponGeometry = nullptr;
	string													m_sClass;

	static map< string, TAction >							s_mActions;
	static map< string, TAnimation >						s_mAnimationStringToType;
	static map< TAnimation, string>							s_mAnimationTypeToString;
	//static map< TAnimation, float >							s_mOrgAnimationSpeedByType;
	static vector< CCharacter* >							s_vHumans;
	static map<string, map<string, pair<string,	pair<float, float>>>>	s_mBodiesAnimations;
	map<string, string>										m_mOverridenAnimation;
	static void												OnWalkAnimationCallback( IAnimation::TEvent e, void* pEntity );
	static void 											Walk( CCharacter*, bool bLoop );
	static void 											Stand( CCharacter*, bool bLoop );
	static void 											Run( CCharacter*, bool bLoop );
	static void 											RunReverse(CCharacter*, bool bLoop);
	static void												Jump(CCharacter* pCharacter, bool bLoop);
	static void												Dying(CCharacter* pCharacter, bool bLoop);
	static void												MoveToGuard(CCharacter* pCharacter, bool bLoop);
	static void												MoveToGuardWeapon(CCharacter* pCharacter, bool bLoop);
	static void												StopRunning(CCharacter* pCharacter, bool bLoop);
	static void 											ReceiveHit( CCharacter* pCharacter, bool bLoop );
	static void												LoadAnimationsJsonFile(IFileSystem& oFileSystem);
};

#endif // MOBILEENTITY_H