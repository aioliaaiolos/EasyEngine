#ifndef IENTITY_H
#define IENTITY_H

// stl
#include <string>

// Engine
#include "INode.h"
#include "EEPlugin.h"
#include "ILoader.h"
#include "Math/matrix.h"
#include "IRessource.h"
#include "IPathFinder.h"


class IRessourceManager;
class IRenderer;
class ICameraManager;
class IAnimation;
class IShader;
class IRessource;
class ICollisionManager;
class IGeometryManager;
class ISphere;
class IAEntity;
class IFighterEntity;
class IGUIManager;

using namespace std;

#define orgEpsilonError 0.001f


struct CKey
{
	enum TKey { eRotKey = 0, ePosKey };
	CKey() : m_nTimeValue(0) {}
	int				m_nTimeValue;
	CMatrix			m_oWorldTM;
	CMatrix			m_oLocalTM;
	CQuaternion		m_oQuat;
	TKey			m_eType;
};

class CBody
{
	static float	s_fEpsilonError;
	static float	s_fZCollisionError;
public:
	float		m_fWeight;
	CVector		m_oSpeed;
				CBody();
	void		Update();

	//static float		GetEpsilonError(){ return 0.001f; }
	static float		GetEpsilonError(){ return s_fEpsilonError; }
	static void			SetZCollisionError( float e );
	static float		GetZCollisionError();
};

float GetGravity();

class IEntity : virtual public INode
{

public:
	typedef void(*LoadRessourceCallback)(CPlugin*);

	enum TAnimation
	{
		eNone = 0,
		eWalk,
		eStand,
		eRun,
		eHitLeftFoot,
		eHitRightArm,
		eHitReceived,
		eJump,
		eDying,
		eGuard,
		eAnimationCount
	};

	enum TLinkType
	{
		ePreserveChildRelativeTM = 0,
		eSetChildToParentTM
	};

	virtual void				Update() = 0;
	virtual void				DrawBoundingBox( bool bDraw ) = 0;
	virtual void				SetShader( IShader* pShader ) = 0;
	virtual IGeometry*			GetBoundingGeometry() = 0;
	virtual void				SetRessource(string sFileName, bool bDuplicate = false) = 0;
	virtual IRessource*			GetRessource() = 0;
	virtual void				SetDiffuseTexture(string sFileName) = 0;
	virtual void				SetWeight( float fWeight ) = 0;
	virtual float				GetWeight() = 0;
	virtual void				AddAnimation( std::string sAnimationFile ) = 0;
	virtual void				SetCurrentAnimation( std::string sAnimation ) = 0;
	virtual IAnimation*			GetCurrentAnimation() = 0;
	virtual void				PlayCurrentAnimation(bool loop) = 0;
	virtual void				PauseCurrentAnimation(bool loop) = 0;
	virtual bool				HasAnimation( string sAnimationName ) = 0;
	virtual void				DetachCurrentAnimation() = 0;
	virtual IBone*				GetSkeletonRoot() = 0;
	virtual void				Hide( bool bHide ) = 0;
	virtual void				RunAction( string sAction, bool bLoop ) = 0;
	virtual void				LinkEntityToBone( IEntity* pChild, IBone* pParentBone, TLinkType = ePreserveChildRelativeTM ) = 0;
	virtual void				SetAnimationSpeed( TAnimation eAnimationType, float fSpeed ) = 0;
	virtual TAnimation			GetCurrentAnimationType() const = 0;
	virtual void				GetTypeName( string& sName ) = 0;
	virtual void				SetScaleFactor( float x, float y, float z ) = 0;
	virtual void				SetRenderingType( IRenderer::TRenderType t ) = 0;
	virtual void				DrawBoundingSphere( bool bDraw ) = 0;
	virtual void				DrawBoneBoundingSphere( int nID, bool bDraw ) = 0;
	virtual void				DrawAnimationBoundingBox( bool bDraw ) = 0;
	virtual float				GetBoundingSphereRadius() const = 0;
	virtual void				Goto( const CVector& oPosition, float fSpeed ) = 0;
	virtual void				GetEntityName(string& sName) = 0;
	virtual void				SetEntityName( string sName ) = 0;
	virtual void				Colorize(float r, float g, float b, float a) = 0;
	virtual void				LinkDummyParentToDummyEntity(IEntity* pEntity, string sDummyName) = 0;
	virtual void				AbonneToEntityEvent(IEventDispatcher::TEntityCallback callback) = 0;
	virtual void				DeabonneToEntityEvent(IEventDispatcher::TEntityCallback callback) = 0;
	virtual void				SetCustomSpecular(const CVector& customSpecular) = 0;
	virtual void				DrawCollisionBoundingBoxes(bool bDraw) = 0;
	virtual int					GetCellSize() = 0;
	virtual void				CreateCollisionMaps(float fBias) = 0;
};

class ILightEntity : public virtual IEntity
{
public:

};

class ICharacter : public virtual IEntity
{
public:
	virtual void				WearShoes(string shoesName)  = 0;
	virtual void				WearCloth(string sClothName, string sDummyName) = 0;
	virtual void				AddHairs(string sHairsPath) = 0;
	virtual void				WearSkinnedClothFull(string sClothName) = 0;
	virtual void				SetBody(string sBodyName) = 0;
};

class IPlayer : public virtual ICharacter
{
public:
	virtual void				Action() = 0;
	virtual void				ToggleDisplayPlayerWindow() = 0;
};

class IScene : public virtual IEntity
{
public:
	typedef void(*LevelCompleteProc)(void*);
	virtual void				RenderMinimap() = 0;
	virtual ITexture*			GetMinimapTexture() = 0;
	virtual void				DisplayMinimap(bool display) = 0;
	virtual void				UpdateMapEntities() = 0;
	virtual void				CollectMinimapEntities(vector<IEntity*>& entities) = 0;
	virtual void				SetGroundMargin(float margin) = 0;
	virtual float				GetGroundMargin() = 0;
	virtual void				GetInfos(ILoader::CSceneInfos& si) = 0;
	virtual void				GetCharactersInfos(vector<IEntity*>& si, INode* pRoot = nullptr) = 0;
	virtual void				GetOriginalSceneFileName(string& sFileName) = 0;
	virtual void				SetOriginalSceneFileName(string sFileName) = 0;
	virtual void				Clear() = 0;
	virtual void				ClearCharacters() = 0;
	virtual void				Load(const ILoader::CSceneInfos& si) = 0;
	virtual IEntity*			Merge(string sRessourceName, float x, float y, float z) = 0;
	virtual void				SetDiffuseFileName(string diffuseFileName) = 0;
	virtual int					GetCurrentHeightMapIndex() = 0;
	virtual void				SetLength(int length) = 0;
	virtual void				SetHeight(float height) = 0;
	virtual void				SetHMFile(string sHMFile) = 0;
	virtual void				DeleteTempDirectories() = 0;
	virtual void				HandleLoadingComplete(LevelCompleteProc callback, void* pData) = 0;
	virtual void				UnhandleLoadingComplete() = 0;
	virtual void				OnChangeSector() = 0;
};

class IFighterEntityInterface
{
public:
	virtual int					GetLife() = 0;
	virtual void				SetLife(int nLife) = 0;
	virtual void				IncreaseLife(int nLife) = 0;
	virtual void				MainHit() = 0;
	virtual void				SecondaryHit() = 0;
};


class IAEntityInterface
{
public:
	virtual void Attack(IFighterEntityInterface* pEntity) = 0;
};

class IEntityManager : public CPlugin
{
public:
	IEntityManager(EEInterface& oInterface) : CPlugin( NULL, "" ){}
	virtual void				AddEntity(IEntity* pEntity, string sName = "noname", int id = -1) = 0;
	virtual IEntity*			CreateEntity(std::string sFileName, bool bDuplicate = false ) = 0;
	virtual IEntity*			CreateEmptyEntity( string sFileName = "noname" ) = 0;
	virtual IEntity*			CreateRepere( IRenderer& oRenderer ) = 0;
	virtual IEntity*			CreateMobileEntity( string sFileName, IFileSystem* pFileSystem, string sID ) = 0;
	virtual IPlayer*			CreatePlayer(string sFileName) = 0;
	virtual ICharacter*			CreateNPC( string sFileName, string sID ) = 0;
	virtual IEntity*			CreateObject(string sFileName) = 0;
	virtual IEntity*			CreateMinimapEntity(string sFileName, IFileSystem* pFileSystem) = 0;
	virtual IEntity*			CreateTestEntity(string sFileName, IFileSystem* pFileSystem) = 0;
	virtual void				GetCharactersName(vector<string>& vCharactersName) = 0;
	virtual IEntity*			GetEntity( int nEntityID ) = 0;
	virtual IEntity*			GetEntity( string sEntityName ) = 0;
	virtual int					GetEntityID( IEntity* pEntity ) = 0;
	virtual int					GetEntityCount() = 0;
	virtual IEntity*			CreateLightEntity( CVector Color, IRessource::TLight type, float fIntensity ) = 0;
	virtual float				GetLightIntensity(int nID) = 0;
	virtual void				SetLightIntensity( int nID, float fIntensity ) = 0;
	virtual void				DestroyEntity( IEntity* pEntity ) = 0;
	virtual void				DestroyAll() = 0;
	virtual void				Clear() = 0;
	virtual IEntity*			CreateSphere( float fSize ) = 0;
	virtual IEntity*			CreateBox(const CVector& oDimension ) = 0;
	virtual IEntity*			CreateQuad(float lenght, float width) = 0;
	virtual ISphere&			GetSphere( IEntity* pSphereEntity ) = 0;
	virtual IBox&				GetBox( IEntity* pBoxEntity ) = 0;
	virtual void				SetZCollisionError( float e ) = 0;
	virtual IAEntity*			GetFirstIAEntity() = 0;
	virtual IAEntity*			GetNextIAEntity() = 0;
	virtual IFighterEntity*		GetFirstFighterEntity() = 0;
	virtual IFighterEntity*		GetNextFighterEntity() = 0;
	virtual IEntity*			GetFirstMobileEntity() = 0;
	virtual IEntity*			GetNextMobileEntity() = 0;
	virtual IEntity*			CreateLineEntity( const CVector& first, const CVector& last ) = 0;
	virtual IEntity*			CreateCylinder( float fRadius, float fHeight ) = 0;
	virtual void				Kill(int entityId) = 0;
	virtual void				WearArmorToDummy(int entityId, string armorName) = 0;
	virtual void				SerializeMobileEntities(INode* pRoot, string& sText) = 0;
	virtual IGUIManager* 		GetGUIManager() = 0;
	virtual void				SetGUIManager(IGUIManager* pGUIManager) = 0;
	virtual void				SetPlayer(IPlayer* player) = 0;
	virtual IPlayer*			GetPlayer() = 0;
	virtual IEntity*			CreatePlaneEntity(int slices, int size, string heightTexture, string diffuseTexture) = 0;
	virtual IBone*				CreateBone() const = 0;
	virtual void				AddNewCharacter(IEntity* pEntity) = 0;
	virtual ICharacter*			BuildCharacterFromDatabase(string sCharacterId, IEntity* pParent) = 0;
	virtual void				SaveCharacter(string sNPCID) = 0;
	virtual void				RemoveCharacterFromDB(string sID) = 0;
	virtual void				EnableInstancing(bool enable) = 0;
};

class ISceneManager : public CPlugin
{
protected:
	ISceneManager() : CPlugin(nullptr, "" ){}
	

public:
	virtual ~ISceneManager() {}
	virtual IScene*		CreateScene(string sSceneName, string sRessourceFileName, string diffuseFileName) = 0;
	virtual IScene*		GetScene( std::string sSceneName ) = 0;
};




class IBone : virtual public INode
{
public:
	virtual void			AddKey(string sAnimation, int nTimeValue, const CMatrix& m, const CQuaternion& q) = 0;
	virtual void			AddKey(string sAnimation, CKey& oKey) = 0;
	virtual void			NextKey() = 0;
	virtual void			Rewind() = 0;
	virtual void			UpdateTime(float fTime) = 0;
	virtual void			GetKeyByIndex(int nIndex, CKey& oKey) const = 0;
	virtual void			GetKeyByTime(int nTime, CKey& oKey) const = 0;
	virtual int				GetKeyCount() const = 0;
	virtual void			SetCurrentAnimation(string sAnimation) = 0;
	virtual void			SetBoundingBox(IBox* oBox) = 0;
	virtual const IBox*		GetBoundingBox() = 0;
	virtual const ISphere*	GetBoundingSphere() = 0;
	virtual IBone*			GetChildBoneByID( int nID ) = 0;
	virtual IBone*			GetChildBoneByName( string sName ) = 0;
};

#endif // IENTITY_H