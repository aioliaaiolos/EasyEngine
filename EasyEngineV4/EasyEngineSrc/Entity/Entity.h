#ifndef ENTITY_H
#define ENTITY_H
#include "IEntity.h"
#include "Node.h"

class CRessource;
class IRessourceManager;
class IRenderer;
class IRessource;
class IAnimation;
class IGeometryManager;
class CScene;
class CEntityManager;
class CNode;
class CBone;
class ICollisionMap;
class CCollisionEntity;
class IScriptManager;
class IConsole;

typedef std::map< std::string, std::map< int, const CBone* > > AnimationBonesMap;

class CEntity : virtual public IEntity, public CNode
{
protected:

	typedef void (*TCollisionCallback)( CEntity*, vector<INode*>);

public:
	CEntity(EEInterface& oInterface);
	CEntity(EEInterface& oInterface, const std::string& sFileName, bool bDuplicate = false);
	virtual							~CEntity();
	void							Update();
	void							DrawBoundingBox( bool bDraw );
	void							SetShader( IShader* pShader );
	void							CenterToworld();
	IRessource*						GetRessource();
	IMesh*							GetMesh();
	void							SetRessource(string sFileName, bool bDuplicate = false);
	void							SetDiffuseTexture(string sFileName);
	float							GetWeight();
	void							SetWeight( float fWeight ) override;
	void							SetMesh( IMesh* pMesh );
	void							AddAnimation( std::string sAnimationFile );
	void							SetCurrentAnimation( std::string sAnimation );
	IAnimation*						GetCurrentAnimation();
	void							PlayCurrentAnimation(bool loop);
	void							PauseCurrentAnimation(bool loop);
	IBone*							GetSkeletonRoot();
	IBone*							GetOrgSkeletonRoot();
	void							SetSkeletonRoot(CBone* pBone, CBone* pOrgBone);
	void							GetEntityInfos(ILoader::CObjectInfos*& pInfos);
	virtual void					BuildFromInfos(const ILoader::CObjectInfos& infos, IEntity* pParent);
	bool							HasAnimation( string sAnimationName );
	void							DetachCurrentAnimation();
	void							Hide( bool bHide );
	void							RunAction( string sAction, bool bLoop ){}
	void							LocalTranslate(float dx , float dy , float dz);
	void							LocalTranslate( const CVector& vTranslate );
	void							LinkEntityToBone( IEntity* pChild, IBone* pParentBone, TLinkType = ePreserveChildRelativeTM );
	virtual void					LinkDummyParentToDummyEntity(IEntity* pEntity, string sDummyName);
	void							SetAnimationSpeed( TAnimation eAnimationType, float fSpeed ){}
	TAnimation						GetCurrentAnimationType() const{return eNone;}
	void							GetTypeName( string& sName );
	void							SetScaleFactor( float x, float y, float z );
	void							GetScaleFactor(CVector& factor);
	void							SetRenderingType( IRenderer::TRenderType t );
	void							DrawBoundingSphere( bool bDraw );
	void							DrawBoneBoundingSphere( int nID, bool bDraw );
	void							DrawAnimationBoundingBox( bool bDraw );
	float							GetBoundingSphereRadius() const;
	void							Link( INode* pNode ) override;
	void							Goto( const CVector& oPosition, float fSpeed );
	void							SetEntityName( string sName );
	void							GetEntityName( string& sName );
	void							Colorize(float r, float g, float b, float a) override;
	ICollisionMesh*					GetCollisionMesh();
	void							ForceAssignBoundingGeometry(IGeometry* pBoundingGeometry);
	IGeometry*						GetBoundingGeometry();
	IBox*							GetBoundingBox();
	float							GetHeight();
	void							LinkAndUpdateMatrices(CEntity* pEntity);
	bool							IsOnTheGround();
	virtual float					GetGroundHeight(float x, float z);
	virtual void					UpdateRessource();
	void							AbonneToEntityEvent(IEventDispatcher::TEntityCallback callback);
	void							DeabonneToEntityEvent(IEventDispatcher::TEntityCallback callback);
	void							SetCustomSpecular(const CVector& customSpecular) override;
	void							DrawCollisionBoundingBoxes(bool bDraw) override;
	static void						GetSkeletonEntities(CBone* pRoot, vector< CEntity* >& vEntity, string sFileFilter);
	void							GetBonesMatrix(std::vector< CMatrix >& vBoneMatrix);
	int								GetCellSize();
	void							CreateCollisionGrid();
	void							CreateCollisionMaps(float fBias, int nCellSize) override;
	void							LoadCollisionMaps();
	ICollisionMap*					GetCollisionMap();
	IGrid*							GetCollisionGrid();
	void							SetSkinOffset(float x, float y, float z) override;
	void							SetSkinOffset(CVector& oSkinOffset);
	void							AttachScript(string sScript) override;
	void							DetachScript(string sScript);
	const string&					GetAttachedScript() const override;
	

protected:
	IRessource*										m_pRessource;
	IRenderer&										m_oRenderer;
	IRessourceManager&								m_oRessourceManager;
	CEntityManager*									m_pEntityManager;
	IGeometryManager&								m_oGeometryManager;
	ICollisionManager&								m_oCollisionManager;
	ILoaderManager*									m_pLoaderManager;
	IScriptManager&									m_oScriptManager;
	IConsole&										m_oConsole;
	CBody											m_oBody;
	IAnimation*										m_pCurrentAnimation;
	std::map< std::string, IAnimation* >			m_mAnimation;
	std::string										m_sCurrentAnimation;
	bool											m_bDrawBoundingBox;
	CBone*											m_pOrgSkeletonRoot;
	CBone*											m_pSkeletonRoot;
	bool											m_bHidden;
	CEntity*										m_pEntityRoot;
	float											m_fBoundingSphereRadius;
	CMatrix											m_oFirstAnimationFrameSkeletonMatrixInv;
	bool											m_bUsePositionKeys;
	string											m_sTypeName;
	CMatrix											m_oScaleMatrix;
	IRenderer::TRenderType							m_eRenderType;
	IEntity*										m_pBoundingSphere;
	map< int, IEntity* >							m_mBonesBoundingSphere;
	bool											m_bDrawAnimationBoundingBox;
	TCollisionCallback								m_pfnCollisionCallback;
	string											m_sEntityName;
	CScene*											m_pScene;
	ICollisionMesh*									m_pCollisionMesh;
	IGeometry*										m_pBoundingGeometry;
	float											m_fMaxStepHeight;
	EEInterface&									m_oInterface;
	IMesh*											m_pMesh;
	bool											m_bEmptyEntity;
	map< string, map< int, IBox* > >				m_oKeyBoundingBoxes;
	vector<IEventDispatcher::TEntityCallback>		m_vEntityCallback;
	ITexture*										m_pBaseTexture;
	ITexture*										m_pCustomTexture;
	bool											m_bIsOnTheGround;
	CVector											m_vCustomSpecular;
	bool											m_bUseCustomSpecular;
	CEntity*										m_pCloth;
	IGrid*											m_pCollisionGrid;
	//int												m_nCollisionGridCellSize;
	IPathFinder&									m_oPathFinder;
	ICollisionMap*									m_pCollisionMap;
	vector< CMatrix >								m_vBoneMatrix;
	CVector											m_oSkinOffset;
	string											m_sAttachedScript;
	vector<unsigned char>							m_vAttachedScriptByteCode;
	time_t											m_nLastAttachScriptTime;

	
	void				SetNewBonesMatrixArray(std::vector< CMatrix >& vMatBones);
	void				GetBonesMatrix(INode* pInitRoot, INode* pCurrentRoot, std::vector< CMatrix >& vMatrix);
	virtual void		UpdateCollision();
	void				GetEntitiesCollision(vector<INode*>& entities);
	void				CreateAndLinkCollisionChildren(string sFileName);
	float				GetBoundingSphereDistance(INode* pEntity);
	void				UpdateBoundingBox();
	bool				ManageGroundCollision(const CMatrix& olastLocalTM);
	bool				TestCollision(INode* pEntity);
	bool				TestWorldCollision(INode* pEntity);
	bool				IsPassingDoor(INode* pWall, IGeometry* pBBox, IGeometry* pWallBBox);
	bool				ManageBoxCollision(vector<INode*>& vCollideEntities, float dx, float dy, float dz, const CMatrix& oBackupMatrix);
	virtual void		SendBonesToShader();
	void				DispatchEntityEvent();
	void				LinkDoorsToWalls(const vector<CCollisionEntity*>& walls, const vector<CCollisionEntity*>& doors);
	void				GetPassageMatrix(INode* pOrgNode, INode* pCurrentNode, CMatrix& passage);
	virtual CEntity*	CreateEmptyEntity(string sName);
	void				ExecuteScripts();
	static void			OnAnimationCallback(IAnimation::TEvent e, void*);	
};

class CCollisionEntity : public CEntity, public ICollisionEntity
{
public:
	CCollisionEntity(EEInterface& oInterface) : CEntity(oInterface) {}
private:
};

#endif // ENTITY_H