#include "IAEntity.h"
#include "MobileEntity.h"

#pragma warning( disable:4250 )

class CNPCEntity : public CMobileEntity, public IAEntity
{
public:

	CNPCEntity(EEInterface& oInterface, string sFileName, string sID);
	int						GetLife();
	void					SetLife( int nLife );
	void					IncreaseLife( int nLife );
	float					GetDistanceTo2dPoint( const CVector& oPosition );
	void					Run();
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
	void					ComputePathFind2D( const CVector2D& oOrigin, const CVector2D& oDestination, vector< CVector2D >& vPoints);	

protected:
	static void				OnCollision(CEntity* pThis, vector<INode*> entities);
	void					ComputePathFind2DAStar(const CVector2D& oOrigin, const CVector2D& oDestination, vector< CVector2D >& vPoints, int nCellSize);

private:
	void					Turn(float fAngle);

	IPathFinder&			m_oPathFinder;
};