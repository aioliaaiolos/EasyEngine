#ifndef IRENDERER_H
#define IRENDERER_H

// stl
#include <vector>
#include <string>
#include "EEPlugin.h"

// Math
#include "Math/matrix.h"

// Interfaces
#include "IEventDispatcher.h"


using namespace std;

class IBuffer;
class CMatrix;
class IFileSystem;
class IWindow;
class IShader;
class CVector;


class IBuffer
{
public:
	virtual ~IBuffer(){}
};

class IRenderer : public CPlugin
{
protected:
	CMatrix			m_mCurrentObjectWorldTM;
	CMatrix			m_mCurrentObjectLocalTM;
	CMatrix			m_oCameraMatrix;
	std::string		m_sDebugString;
	bool			m_bDisplayDebugInfos;
	IRenderer() : CPlugin( nullptr, "" ){}

public:

	typedef void	( *TRenderEventCallback )( IRenderer* );	

	enum TPixelFormat
	{ 
		T_RGB = 0, 
		T_RGBA,
		T_BGR,
		T_BGRA,
		T_FormatNone
	};

	enum TRenderType
	{
		ePoint = 0,
		eLine,
		eFill
	};

	enum TShaderType{ T_PIXEL_SHADER = 0, T_VERTEX_SHADER };
	enum TNumberType{ T_FLOAT = 0, T_INT };
	enum TDrawStyle{ T_POINTS = 0, T_LINES , T_TRIANGLES, T_LINE_LOOP, T_LINE_STRIP, T_QUADS };
	enum TTextureDimension{ T_1D = 0, T_2D, T_3D };
		
	virtual void			SetRenderType( TRenderType t ) = 0;
	virtual void			EnableZTest( bool bEnable ) = 0;
	virtual void			SetLightAttenuation( unsigned int nLightID, float fConstant, float fLinear, float fQuadratic ) = 0;
	virtual void			SetLightSpotProp( unsigned int nLightID, float fCutoff, float fExponent ) = 0;
	virtual void			EnableLight( unsigned int LightID) = 0;
	virtual void			SetLightLocalPos( unsigned int nLightID, float x, float y, float z, float w ) = 0;
	virtual void			SetLightSpotDirection( unsigned int nLightID, float x, float y ,float z, float w ) = 0;
	virtual void			DisableLight( unsigned int LightID ) = 0;
	virtual void			SetLightAmbient( unsigned int nLightID, float r, float g, float b, float a ) = 0;
	virtual void			SetLightDiffuse( unsigned int nLightID, float r, float g, float b, float a ) = 0;
	virtual void			SetLightSpecular( unsigned int nLightID, float r, float g, float b, float a ) = 0;
	virtual void			SetMaterialAmbient( float* fAmbient ) = 0;
	virtual void			SetMaterialDiffuse( float* fDiffuse ) = 0;
	virtual void			SetMaterialSpecular( float* fSpecular ) = 0;
	virtual void			SetMaterialEmissive( float* fEmissive ) = 0;
	virtual void			SetMaterialShininess( float fShininess ) = 0;
	virtual unsigned int	CreateVertexBuffer( const std::vector< float >& vData ) const = 0;
	virtual unsigned int	CreateVertexBuffer( const std::vector< int >& vData ) const = 0;
	virtual int				CreateBuffer( int nElementCount ) = 0;
	virtual void			FillBuffer( const std::vector< float >& vData, int nBufferID, int nOffset = 0 ) = 0;
	virtual void			FillBuffer(const vector< CMatrix >& vMatrix, int nBufferID, int nOffset = 0) = 0;
	virtual void			AppendBuffer( const std::vector< float >& vData ) = 0;
	virtual void			GetNonIndexedVertexBuffer( const std::vector< float >& vVertexBuffer, const std::vector< unsigned int >& vIndexBuffer, std::vector< float >& vOutVertexBuffer ) = 0;
	virtual void			BindVertexBuffer( int nBufferID ) = 0;
	virtual void			DeleteBuffer( unsigned int nBufferID ) = 0;
	virtual void			DeleteBuffer( IBuffer* pBuffer ) = 0;
	//virtual void			CreateNonIndexedVertexArray( const std::vector< UINT >& vIndexArray, const std::vector< float >& vVertexArray, int nComposantCount, std::vector< float >& vOutVertexArray ) = 0;
	virtual IBuffer*		CreateIndexedGeometry( const std::vector< float >& vVertexArray, const std::vector< unsigned int >& vIndexArray,
												const std::vector< float >& vUVVertexArray, const vector< unsigned int >& vUVIndexArray, 
												const std::vector< float >& vNormalVertexArray ) = 0;

	virtual IBuffer*		CreateGeometry( const std::vector< float >&	vVertexArray, const std::vector< unsigned int >& vIndexArray, 
											const std::vector< float >& vUVVertexArray, const std::vector< unsigned int >& vUVIndexArray, 
											const std::vector< float >& vNormalFaceArray, const std::vector< float >& vNormalVertexArray ) = 0;

	virtual void 			DrawGeometry( const IBuffer* pBuffer ) = 0;
	virtual void			DrawGeometryInstanced(const IBuffer* pBuffer, int instanceCount) = 0;
	virtual void			DrawIndexedGeometry( const IBuffer* pBuffer, TDrawStyle style = T_TRIANGLES ) = 0;
	virtual void			DrawIndexedGeometryInstanced(const IBuffer* pBuffer, TDrawStyle style = T_TRIANGLES, int instanceCount = 1) = 0;
	virtual void			DrawLine( const CVector& p1, const CVector& p2, const CVector& color ) = 0;
	virtual void			DrawBase( const CMatrix& mBase, float fSize ) = 0;
	virtual void			DrawBox( const CVector& oMinPoint, const CVector& oDimension ) = 0;
	virtual void			DrawSphere(double dRadius, unsigned int nSliceCount, unsigned int nStackCount) = 0;
	virtual void			DrawCylinder(double dBaseRadius, double dTopRadius, double dHeight,	unsigned int nSlicesCount, unsigned int nStacksCount) = 0;
	virtual void			DrawQuad(float fLenght, float fWidth) = 0;
	virtual int				CreateTexture1D( float* pTexelsArray, int nSize, TPixelFormat format ) = 0;
	virtual int				CreateTexture2D( vector< unsigned char>& pTexelsArray, int nWidth, int nHeight, TPixelFormat format ) = 0;
	virtual int				CreateMipmaps2D(vector< unsigned char>& vTexel, int nWidth, int nHeight, IRenderer::TPixelFormat format) = 0;
	virtual void			BindTexture( int nTextureID, int nUnitTextureID, TTextureDimension texDim ) = 0;
	virtual void			GetResolution( unsigned int& nWidth, unsigned int& nHeight ) = 0;
	virtual void			EnableTextureBlend( bool bEnable ) = 0;
	virtual void			CalculProjectionMatrix( CMatrix& oMatrix, float fov ) = 0;
	virtual void			GetProjectionMatrix( CMatrix& oMatrix ) = 0;
	virtual void			SetFov( float fov ) = 0;

	virtual void			BeginDirectGeometry( IRenderer::TDrawStyle style ) = 0;
	virtual void			SetVertex3f( float x, float y, float z ) = 0;
	virtual void			SetVertex4f( float x, float y, float z, float w ) = 0;
	virtual void			SetColor3f( float r, float g, float b ) = 0;
	virtual void			EndDirectGeometry() = 0;

	virtual void			SetModelViewMatrix( const CMatrix& oMatrix ) = 0;
	virtual void			SetProjectionMatrix( const CMatrix& oMatrix ) = 0;
	virtual void			GetInvCameraMatrix(CMatrix& oMatrix) const = 0;
	virtual void			SetInvCameraMatrix(const CMatrix& oMatrix) = 0;
	virtual void			SetCameraMatrix( const CMatrix& oMatrix ) = 0;
	virtual void			GetModelMatrix(CMatrix& oMatrix) = 0;
	virtual void			SetModelMatrix( const CMatrix& oMatrix  ) = 0;
	virtual void			BeginRender() = 0;
	virtual void			EndRender() = 0;
	virtual void			DestroyContext() = 0;

	virtual IShader*		GetShader( std::string sShaderName ) = 0;
	virtual void			GetDefaultShader( std::string& sDefaultShader ) = 0;
	virtual void			LoadShader(string sShaderName) = 0;

	virtual void			SetDebugString( std::string ) = 0;
	virtual void			GetDebugString( std::string& ) const = 0;
	virtual void			DisplayDebugInfos( bool ) = 0;
	virtual bool			IsDisplayDebugInfos() const = 0;
	virtual void			SetCurrentObjectLocalTM( CMatrix& m ) = 0;
	virtual void			SetCurrentCameraMatrix( CMatrix& m ) = 0;
	virtual void			SetBackgroundColor(const CVector& vColor) = 0;
	virtual void			SetBackgroundColor( float r, float g, float b, float a = 1.f ) = 0;
	virtual void			GetBackgroundColor( CVector& vColor ) = 0;
	virtual void			ReadPixels( int x, int y, int width, int height, vector< unsigned char >& vPixels, TPixelFormat format ) = 0;

	virtual void			AbonneToRenderEvent( TRenderEventCallback callback ) = 0;
	virtual void			DesabonneToRenderEvent( TRenderEventCallback callback ) = 0;

	virtual void			LockCamera(bool lock) = 0;
	virtual void			GetModelViewProjectionMatrix(CMatrix& oMatrix) = 0;
	virtual void			ReloadShaders(IEventDispatcher& oEventDispatcher) = 0;
	virtual void			ReloadShader(string shaderName) = 0;
	virtual void			CullFace(bool enable) = 0;
	virtual void			EnableDepthTest(bool enable) = 0;
	virtual void			SetLineWidth(int width) = 0;
	virtual bool			IsCullingEnabled() = 0;

	virtual void			SetCurrentFBO(int fbo) = 0;
	virtual void			CreateFrameBufferObject(int width, int height, unsigned int& nFBOId, unsigned int& nTextureId) = 0;
	virtual void			ResizeScreen(int nWidth, int nHeight) = 0;
	virtual float			GetScreenRatio() = 0;
	virtual void			SetModelViewToIdentity() = 0;
	virtual void			ClearFrameBuffer() = 0;
	virtual void			GetGlslVersion(string& sVersion) = 0;
	virtual void			GetOpenglVersion(string& sVersion) = 0;
	virtual int				GetLastError() = 0;
};





#endif