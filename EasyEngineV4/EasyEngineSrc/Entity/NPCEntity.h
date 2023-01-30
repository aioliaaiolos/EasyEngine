#include "IAEntity.h"
#include "MobileEntity.h"

#pragma warning( disable:4250 )


class IGUIWindow;

class CNPCEntity : public CCharacter, public IAEntity
{
public:

	CNPCEntity(EEInterface& oInterface, string sFileName, string sID);
	int						GetLife();
	void					SetLife( int nLife );
	void					IncreaseLife( int nLife );
	float					GetDistanceTo2dPoint( const CVector& oPosition );
	void					Run();
	void					MoveToGuard();
	void					Guard();
	void					ReceiveHit( IAEntity* pEnemy );
	ICollisionManager&		GetCollisionManager(){ return m_oCollisionManager; }
	void					LookAt( float alpha );
	void					Update();
	IAnimation*				GetCurrentAnimation();
	CMatrix&				GetWorldTM();
	IFighterEntity*			GetFirstEnemy();
	IFighterEntity*			GetNextEnemy();
	void					GetPosition( CVector& v );
	void					ReceiveHit( IFighterEntity* pEnemy );
	void					Stand();
	void					Die();
	void					Goto( const CVector& oPosition, float fSpeed );
	IBox*					GetFirstCollideBox();
	IBox*					GetNextCollideBox();	
	void					ComputePathFind2D( const CVector& oOrigin, const CVector& oDestination, vector<CVector>& vPoints);
	void					OpenTopicWindow();

protected:
	void					UpdateGoto() override;
	void					ComputePathFind2DAStar(const CVector& oOrigin, const CVector& oDestination, vector<CVector>& vPoints);
	INode*					GetParent() override;

	static void				OnCollision(CEntity* pThis, vector<INode*> entities);
	static void				OnTopicWindowClosed(IGUIWindow*, IBaseObject*);

private:
	void					Turn(float fAngle);

	IPathFinder&			m_oPathFinder;
	IGUIManager&			m_oGUIManager;
	const float				m_fBBoxReduction = 20.f;
	IBox*					m_pGotoBox;
	IGeometry*				m_pBackupBoundingGeometry;
};