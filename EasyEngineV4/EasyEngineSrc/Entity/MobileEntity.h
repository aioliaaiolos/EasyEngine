#ifndef MOBILEENTITY_H
#define MOBILEENTITY_H

#include "Entity.h"
#include "FighterEntity.h"

class IFileSystem;
class CItem;

class CObject : public CEntity
{
public:
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

class CMobileEntity : public CObject, public virtual IFighterEntity, public virtual ICharacter
{

public:
	CMobileEntity(EEInterface& oInterface, string sFileName, string sID);
	virtual ~CMobileEntity();

	float						GetAnimationSpeed(IEntity::TAnimation eAnimationType);
	void						GetEntityInfos(ILoader::CObjectInfos*& pInfos);
	void						BuildFromInfos(const ILoader::CObjectInfos& infos, IEntity* pParent, bool bExcludeChildren = false) override;
	void						Save();
	void						SetAnimationSpeed(TAnimation eAnimationType, float fSpeed);
	TAnimation					GetCurrentAnimationType() const;
	void						RunAction(string sAction, bool bLoop);
	void						Die();
	void						WearArmorToDummy(string armorName);
	void						WearShoes(string shoesName) override;
	void						UnWearShoes(string shoesPath) override;
	void						UnWearAllShoes() override;
	void						SetHairs(string sHairsPath) override;
	void						SetBody(string sBodyName) override;
	void						Yaw(float fAngle);
	void						Pitch(float fAngle);
	void						Roll(float fAngle);
	IAnimation*					GetCurrentAnimation();
	void						AddItem(string sItemName);
	void						RemoveItem(string sItemName);
	void						WearItem(string sItemID);
	void						UnWearItem(string sItemID);
	int							GetItemCount(string sItemID);
	void						GetItems(map<string, vector<IEntity*>>& vItems) const;
	void						Link(INode* pParent) override;
	IBox*						GetBoundingBox();
	static void					InitStatics(IFileSystem& oFileSystem);

	static map<string, IEntity::TAnimation>	s_mStringToAnimation;

protected:
	typedef void (*TAction)( CMobileEntity*, bool );

	void										InitAnimations();
	void										SetPredefinedAnimation(string s, bool bLoop);
	void 										Walk(bool bLoop);
	void 										Stand(bool bLoop);
	void 										Run(bool bLoop);
	void 										Jump(bool bLoop);
	void										PlayReceiveHit(bool bLoop);
	void										Stand();
	void										PlayReceiveHit();
	void										PlayHitAnimation();
	void										PlaySecondaryHitAnimation();
	void										MoveToGuard();
	void										Guard();
	void										TurnEyesH(float fValue);
	void										TurnNeckH(float f);
	IBone*										GetPreloadedBone(string sName);
	void										GetPosition(CVector& oPosition) const;
	IMesh*										GetMesh();
	IFighterEntity*								GetFirstEnemy();
	IFighterEntity*								GetNextEnemy();
	CMatrix&									GetWorldTM();
	float										GetBoundingSphereRadius();
	ICollisionManager&							GetCollisionManager() { return m_oCollisionManager; }
	ISphere*									GetBoneSphere(string sBoneName);
	void										AddSpeed(float x, float y, float z);
	const string&								GetAttackBoneName();
	const string&								GetSecondaryAttackBoneName();
	IGeometry*									GetBoundingGeometry() override;
	void										Wear(string sClothPath, string sDummyName);
	void										Wear(CEntity* pEntity, string sDummyName);
	void										UnWear(CEntity* pCloth);
	void										SaveToJson();

	string										m_sFileNameWithoutExt;
	bool										m_bInitSkeletonOffset;
	CMatrix										m_oSkeletonOffset;
	TAnimation									m_eCurrentAnimationType;
	map< string, IBone* >						m_mPreloadedBones;
	IEntity*									m_pLeftEye;
	IEntity*									m_pRightEye;
	IBone*										m_pNeck;
	float										m_fEyesRotH;
	float										m_fEyesRotV;
	float										m_fNeckRotH;
	float										m_fNeckRotV;
	const float									m_fMaxEyeRotationH;
	const float									m_fMaxEyeRotationV;
	const float									m_fMaxNeckRotationH;
	const float									m_fMaxNeckRotationV;
	string										m_sAttackBoneName;
	string										m_sSecondaryAttackBoneName;
	CVector										m_vNextLocalTranslate;
	string										m_sStandAnimation;
	IBox*										m_pBBox;
	map<string, vector<CItem*>>					m_mItems;
	map< TAnimation, float >					m_mAnimationSpeedByType;
	string										m_sCurrentBodyName;

	static map< string, TAction >				s_mActions;
	static map< string, TAnimation >			s_mAnimationStringToType;
	static map< TAnimation, string>				s_mAnimationTypeToString;
	static map< TAnimation, float >				s_mOrgAnimationSpeedByType;
	static vector< CMobileEntity* >				s_vHumans;
	static map<string, map<string, string>>		s_mBodiesAnimations;

	static void				OnWalkAnimationCallback( IAnimation::TEvent e, void* pEntity );
	static void 			Walk( CMobileEntity*, bool bLoop );
	static void 			Stand( CMobileEntity*, bool bLoop );
	static void 			Run( CMobileEntity*, bool bLoop );
	static void				Jump(CMobileEntity* pHuman, bool bLoop);
	static void				Dying(CMobileEntity* pHuman, bool bLoop);
	static void				MoveToGuard(CMobileEntity* pHuman, bool bLoop);
	static void 			PlayReceiveHit( CMobileEntity* pHuman, bool bLoop );
	static void				OnDyingCallback(IAnimation::TEvent e, void* data);
	static void				LoadAnimationsJsonFile(IFileSystem& oFileSystem);
};

#endif // MOBILEENTITY_H