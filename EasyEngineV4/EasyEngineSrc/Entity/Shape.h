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
	void			GetTypeName( string& sName ){}
	void			SetScaleFactor( float x, float y, float z ){}
	void			SetRenderingType( IRenderer::TRenderType t ){}
	void			DrawBoundingSphere( bool bDraw ){}
	void			DrawBoneBoundingSphere( int nID, bool bDraw ){}
	void			DrawAnimationBoundingBox( bool bDraw ){}
	float			GetBoundingSphereRadius() const{ return 0.f; }
	void			Goto( const CVector& oPosition, float fSpeed ){ throw 1; }
	void			SetEntityName( string sName ){ throw 1; }
	void			ReloadShader();
	void			AbonneToEntityEvent(IEventDispatcher::TEntityCallback callback) override {}
	void			DeabonneToEntityEvent(IEventDispatcher::TEntityCallback callback) override {}
	void			SetDiffuseTexture(string sFileName) override {}
	void			GetEntityInfos(ILoader::CObjectInfos*& pInfos) {}
	void			SetCustomSpecular(const CVector& customSpecular) override {}
	void			DrawCollisionBoundingBoxes(bool bDraw) {}
	void			PlayCurrentAnimation(bool loop) {}
	void			PauseCurrentAnimation(bool loop) {}
	void			CreateCollisionMaps(float fBias) override{}
	void			SetSkinOffset(float x, float y, float z) override {}
	void			GetScaleFactor(CVector& factor) {}
};

#endif // SHAPE_H