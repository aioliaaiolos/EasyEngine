#ifndef IRESSOURCE_H
#define IRESSOURCE_H

#include <string>
#include <vector>
#include <string>
#include "EEPlugin.h"
#include "../Utils2/Chunk.h"
#include "ILoader.h"
#include "IRenderer.h"

class IRenderer;
class IShaderManager;
class CMatrix;
class IDrawTool;
class IFileSystem;
class ILoaderManager;
class CQuaternion;
class CVector;
class IShader;
class IBone;
class IBox;
struct CKey;
class IGeometry;

using namespace std;

class IRessource
{	
	IRenderer&						m_oRenderer;

protected:
	string							m_sFileName;
	string							m_sName;
	IRenderer&						GetRenderer(){ return m_oRenderer; }

public:

	enum TLight
	{
		DIRECTIONAL,
		OMNI,
		SPOT
	};

	
	struct Desc
	{
		IRenderer&			m_oRenderer;
		IShader*			m_pShader;
		string				m_sName;
		string				m_sFileName;
		Desc( IRenderer& oRenderer, IShader* pShader ) : m_oRenderer( oRenderer ), m_pShader( pShader ){}
	};

	IRessource( const Desc& oDesc ) : m_oRenderer( oDesc.m_oRenderer ), m_sName( oDesc.m_sName ), m_sFileName( oDesc.m_sFileName ){}
	virtual				~IRessource() = 0{}

	virtual void 		Update() = 0;
	virtual void		SetShader( IShader* pShader ) = 0;
	virtual void		GetFileName( string& sFileName )
	{ 
		sFileName = m_sFileName; 
	}
	virtual void		SetFileName( string sFileName ){ m_sFileName = sFileName; }
	virtual IShader*	GetShader() const = 0;
	virtual void		GetName( string& sName ){ sName = m_sName; }
	virtual void		SetName( string sName ){ m_sName = sName; }
};

class ILight : public IRessource
{
public:

	ILight(const IRessource::Desc& oDesc) : IRessource(oDesc) {}
	virtual void SetIntensity(float fIntensity) = 0;
	virtual void SetAmbient(float fAmbient) = 0;
	virtual void Enable(bool enable) = 0;
};

class ITexture : public IRessource
{
public:
	ITexture( const IRessource::Desc& oDesc ) : IRessource( oDesc ){}
	virtual void			GetDimension( int& nWidth, int& nHeight ) = 0;
	virtual unsigned int	GetFrameBufferObjectId() = 0;
	virtual void			SetUnitTexture(int nUnitTexture) = 0;
	virtual void			SetUnitName(string sUnitName) = 0;
};



class IAnimation : public IRessource
{
public:

	enum TEvent
	{
		ePlay = 0,
		eBeforeUpdate,
		eAfterUpdate,
		eBeginRewind

	};
	typedef void(*TCallback)(TEvent, void*);

	IAnimation( const IRessource::Desc& oDesc ) : IRessource( oDesc ){}
	virtual void		Play( bool bLoop ) = 0;
	virtual void		Pause( bool bPause ) = 0;
	virtual void		Stop() = 0;
	virtual float		GetSpeed() = 0;
	virtual void		SetSpeed( float fSpeed ) = 0;
	virtual bool		GetPause() = 0;
	virtual void		SetSkeleton( IBone* pBone ) = 0;
	virtual void		NextFrame() = 0;
	virtual void		SetAnimationTime( int nTime ) = 0;
	virtual int			GetAnimationTime() = 0;
	virtual void		AddCallback( TCallback pCallback, void* pData ) = 0;
	virtual void		RemoveCallback( TCallback pCallback ) = 0;
	virtual void		RemoveAllCallback() = 0;
	virtual int			GetStartAnimationTime() = 0;
	virtual int			GetEndAnimationTime() = 0;
	virtual void		GetBoneKeysMap( map< int, vector< CKey > >& mBoneKeys ) = 0;
};

class IMaterial : public IRessource
{
public:
	IMaterial(const Desc& oDesc) : IRessource(oDesc) {}
	virtual void SetAmbient(float r, float g, float b, float a) = 0;
	virtual void SetDiffuse(float r, float g, float b, float a) = 0;
	virtual void SetSpecular(float r, float g, float b, float a) = 0;
	virtual void SetSpecular(const CVector& pos) = 0;
	virtual void SetShininess(float shininess) = 0;
	virtual void SetAdditionalColor(float r, float g, float b, float a) = 0;
	virtual CVector GetSpecular() = 0;
};

class IMesh : public IRessource
{
public:
	IMesh( const IRessource::Desc& oDesc ) : IRessource( oDesc ){}
	virtual bool			operator==( const IMesh& w ) = 0;
	virtual					~IMesh() = 0{}
	virtual void			DrawBoundingBox( bool bDraw ) = 0;
	virtual IBox*			GetBBox() = 0;
	virtual int				GetParentBoneID()const = 0;
	virtual void			GetOrgWorldPosition( CVector& v ) = 0;
	virtual void			SetRenderingType( IRenderer::TRenderType t ) = 0;
	virtual IBox*			GetAnimationBBox( string sAnimation ) = 0;
	virtual void			DrawAnimationBoundingBox( bool bDraw ) = 0;
	virtual void			SetCurrentAnimationBoundingBox( string AnimationName ) = 0;
	virtual CVector&		GetOrgMaxPosition() = 0;
	virtual void			Colorize(float r, float g, float b, float a) = 0;
	virtual ITexture*		GetTexture(int nMaterialIndex) = 0;
	virtual void			SetTexture(ITexture* pTexture) = 0;
	virtual void			SetMaterial(IRessource* pMaterial) = 0;
	virtual int				GetMaterialCount() = 0;
	virtual IMaterial*		GetMaterial(int index) = 0;
	virtual void			SetDrawStyle(IRenderer::TDrawStyle style) = 0;
	virtual void			UpdateInstances(int instanceCount) = 0;
	virtual bool			IsSkinned() = 0;
};

class IAnimatableMesh : public IRessource
{
public:
	IAnimatableMesh( const IRessource::Desc& oDesc ) : IRessource( oDesc ){}
	virtual ~IAnimatableMesh() = 0{}
	virtual IMesh*			GetMesh( int nIndex ) = 0;
	virtual IBone*			GetSkeleton() = 0;
	virtual unsigned int	GetMeshCount() = 0;
};


class ICollisionMesh : public IRessource
{
public:
	ICollisionMesh(const IRessource::Desc& oDesc) : IRessource(oDesc) {}
	virtual bool		IsCollide(IBox* pBox) = 0;
	virtual IGeometry*	GetGeometry(int index) = 0;
	virtual int			GetGeometryCount() const = 0;
};


class IRessourceManager : public CPlugin
{
protected:
	IRessourceManager() : CPlugin( nullptr, ""){}

public:
	struct Desc : public CPlugin::Desc
	{
		IFileSystem&		m_oFileSystem;
		ILoaderManager&		m_oLoaderManager;
		IRenderer&			m_oRenderer;
		EEInterface&		m_oEngineInterface;
		Desc( IRenderer& oRenderer, IFileSystem& oFileSystem, ILoaderManager& oLoaderManager, EEInterface& oEngineInterface ):
			CPlugin::Desc( NULL, "" ),
			m_oFileSystem( oFileSystem ),
			m_oLoaderManager( oLoaderManager ),
			m_oRenderer(oRenderer),
			m_oEngineInterface(oEngineInterface) {}
	};

	virtual IRessource*			GetRessource( const std::string& sRessourceFileName, bool bDuplicate = false ) = 0;
	virtual IRessource*			CreateMaterial( ILoader::CMaterialInfos& mi, ITexture* pAlternative = NULL ) = 0;
	virtual IAnimatableMesh*	CreateMesh( ILoader::CAnimatableMeshData& mi, IRessource* pMaterial ) = 0;
	virtual IMesh*				CreatePlane(int slices, int size, string diffuseTexture = "NONE") = 0;
	virtual IMesh*				CreatePlane2(int slices, int size, float height, string heightTexture, string diffuseTexture) = 0;
	virtual int					GetLightCount() = 0;
	virtual void				SetDrawTool( IDrawTool* pDrawTool ) = 0;
	virtual IRessource*			CreateLight( CVector Color, IRessource::TLight type, float fIntensity) = 0;
	virtual void				SetLightIntensity( IRessource* pLight, float fIntensity ) = 0;
	virtual float				GetLightIntensity( IRessource* pRessource ) = 0;
	virtual CVector				GetLightColor( IRessource* pRessource ) = 0;
	virtual IRessource::TLight	GetLightType( IRessource* pRessource ) = 0;
	virtual void				EnableCatchingException( bool bEnable ) = 0;
	virtual bool				IsCatchingExceptionEnabled() = 0;
	virtual void				PopErrorMessage( string& sMessage ) = 0;
	virtual void				DestroyAllRessources() = 0;
	virtual ITexture*			CreateRenderTexture(int width, int height, string sShaderName) = 0;
	virtual ITexture*			CreateTexture2D(IShader* pShader, int nUnitTexture, vector< unsigned char >& vData, int nWidth, int nHeight, IRenderer::TPixelFormat eFormat) = 0;
	virtual ITexture*			CreateTexture2D(string sFileName, bool bGenerateMipmaps) = 0;
	virtual void				RemoveAllLights() = 0;
	virtual void				Reset() = 0;
};

#endif // IRESSOURCE_H