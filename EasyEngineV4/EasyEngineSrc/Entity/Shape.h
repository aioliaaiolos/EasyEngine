#ifndef SHAPE_H
#define SHAPE_H


#include "IEntity.h"
#include "Node.h"

class IRenderer;
class IShader;

class CShape : virtual public IEntity, public CNode
{
protected:
	IRenderer&	m_oRenderer;
	IShader*	m_pShader;
	bool		m_bHidden;

public:
	CShape( IRenderer& oRenderer );
	virtual void	Update() = 0;
	void			SetAxesLength( float r, float g, float b );
	void			SetAxesColor( int r, int g, int b );
	void			GetRessourceFileName( string& sFileName );
	void			DrawBoundingBox( bool bDraw );
	void			SetShader( IShader* pShader );
	IGeometry*		GetBoundingGeometry() { return NULL; }
	IRessource*		GetRessource(){ return NULL; }
	void			SetWeight( float fWeight ){}
	float			GetWeight(){ return 0.f; }
	void			SetRessource( string sFileName, bool bDuplicate = false ){throw 1;}
	void			AddAnimation( std::string sAnimationFile ){}
	void			SetCurrentAnimation( std::string sAnimation ){}
	IAnimation*		GetCurrentAnimation(){ return NULL; }
	bool			HasAnimation( string sAnimationName ){ return false; }
	void			DetachCurrentAnimation(){}
	IBone*			GetSkeletonRoot(){ return NULL; }
	void			Hide( bool bHide );
	void			RunAction( string sAction, bool bLoop ){}
	void			SetAnimationSpeed( TAnimation eAnimationType, float fSpeed ){}
	TAnimation		GetCurrentAnimationType() const{return eNone;}
	void			LinkEntityToBone( IEntity*, IBone*, TLinkType = ePreserveChildRelativeTM  ){}
	void			SetScaleFactor( float x, float y, float z ){}
	void			SetRenderingType( IRenderer::TRenderType t ){}
	void			DrawBoundingSphere( bool bDraw ) override{}
	void			DrawBoneBoundingSphere( int nID, bool bDraw ) override{}
	void			DrawAnimationBoundingBox( bool bDraw ) override{}
	float			GetBoundingSphereRadius() const override{ return 0.f; }
	void			Goto( const CVector& oPosition, float fSpeed ) override{ throw 1; }
	void			SetEntityID( string sName ) override{ throw 1; }
	void			ReloadShader();
	void			AbonneToEntityEvent(IEventDispatcher::TEntityCallback callback) override {}
	void			DeabonneToEntityEvent(IEventDispatcher::TEntityCallback callback) override {}
	void			SetDiffuseTexture(string sFileName) override {}
	void			SetCustomSpecular(const CVector& customSpecular) override {}
	void			DrawCollisionBoundingBoxes(bool bDraw) override{}
	void			PlayCurrentAnimation(bool loop) override{}
	void			PauseCurrentAnimation(bool loop) override{}
	void			CreateCollisionMaps(float fBias, int nCellSize = -1) override{}
	void			SetSkinOffset(float x, float y, float z) override {}
	void			GetScaleFactor(CVector& factor) override{}
	void			AttachScript(string sScriptName) override {}
	bool			TestCollision(INode* pEntity) override { return false; }
	const			string& GetAttachedScript() const override { return ""; }
};

#endif // SHAPE_H