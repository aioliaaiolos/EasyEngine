#ifndef RENDERER_H
#define RENDERER_H

#include <windows.h>
#include "IRenderer.h"
#include "EEPlugin.h"
#include "Math/Matrix.h"

// stl
#include <vector>

#ifndef GLAPIENTRY
#define GLAPIENTRY APIENTRY
#endif

#define BUFFER_OFFSET(i) ((char*)NULL + i)	

class CMatrix;

enum RENDER_MODE
{
	RENDER_MODE_NONE = 0,
	RENDER_MODE_BUFFER
};


typedef struct _AUX_RGBImageRec AUX_RGBImageRec;
typedef class GLUquadric GLUquadricObj;

typedef int GLsizei;
typedef unsigned int GLuint;
typedef unsigned int GLenum;
typedef ptrdiff_t GLsizeiptrARB;
typedef void GLvoid;
typedef int GLint;
typedef ptrdiff_t GLintptrARB;

class CSLight;
class CGUIWidget;
class IFileSystem;
class IWindow;
class CShader;

class CRenderer : public IRenderer
{

protected:

	enum									ENUM_MODE { MODE_2D, MODE_3D };
	std::map< int, int >					m_mDrawStyle;
	std::map< TPixelFormat, int >			m_mPixelFormat;
	map< TRenderType, int >					m_mRenderType;
	std::map< TNumberType, GLenum >			m_mNumberType;
	std::map< TTextureDimension, int >		m_mTextureDimension;

	GLUquadricObj*							m_pQuadricObj;
	IWindow&								m_oWindow;
	HDC										m_hDC;
	HGLRC									m_hRC;
	ENUM_MODE								m_Mode;
	std::vector< TRenderEventCallback >		m_vRenderEventCallback;
	CMatrix									m_oProjectionMatrix;
	int										m_nCurrentBufferID;
	int										m_nCurrentBufferOffset;

	std::map< std::string, CShader* >		m_mShader;
	IFileSystem&							m_oFileSystem;
	std::string								m_sDefaultShader;
	string									m_sShadersDirectory;
	string									m_sShaderDirectory;

	CMatrix									m_oCameraMatrixInv;
	CMatrix									m_oCurrentModelMatrix;
	TRenderType								m_eCurrentRenderType;
	bool									m_bMustChangeFov;
	float									m_fFov;	

	void						LoadMatrix( const CMatrix& m );
	void						MultMatrix( const CMatrix& m );
	virtual void				CreateOGLContext();
	void						InitOpengl();
	void						InitGLExtensions();
	void						LoadShaderDirectory(const std::string& sShaderDirectory);
	void						TurnOffAllLight();
	void						ApplySetFov();
	CVector						m_vBackgroundColor;
	bool						m_bCameraLocked;

public:	

								CRenderer(EEInterface& oInterface);
								~CRenderer();
	void						SetRenderType( TRenderType t );
	void						ResizeScreen( int nWidth, int nHeight );
	void						DestroyContext();
	void						BeginRender();
	void						ClearFrameBuffer();
	void						EndRender();
	void						EnableZTest( bool bEnable );
	
	void						CalculProjectionMatrix( CMatrix& oMatrix, float fLeft, float fRight, float fBottom, float fTop, float fNear, float fFar );
	void						CalculProjectionMatrix( CMatrix& oMatrix, float fov );
	void						GetProjectionMatrix( CMatrix& oMatrix );
	void						GetModelViewMatrix(CMatrix& oMatrix);
	void						GetModelViewProjectionMatrix(CMatrix& oMatrix);
	void						SetFov( float fov );

	
	IBuffer*					CreateGeometry( const std::vector< float >&	vVertexArray, const std::vector< unsigned int >& vIndexArray, 
												const std::vector< float >& vUVVertexArray, const std::vector< unsigned int >& vUVIndexArray, 
												const std::vector< float >& vNormalFaceArray, const std::vector< float >& vNormalVertexArray );

	IBuffer*					CreateIndexedGeometry( const std::vector< float >&	vVertexArray, const std::vector< unsigned int >& vIndexArray, 
												const vector< float >& vUVVertexArray, const vector< unsigned int >& vUVIndexArray, const  const std::vector< float >& vNormalVertexArray );

	void						GenUVVertexArray( vector< unsigned int >& vIndexArray, vector< float >& vVertexArray, 
									const vector< unsigned int >& vUVIndexArray, const vector< float >& vUVVertexArray, vector< float >& vNewUVVertexArray  );

	
	
	void						BindTexture( int nTextureID, int nUnitTextureID, TTextureDimension texDim );
	void						DrawGeometry( const IBuffer* pBuffer ) override;
	void						DrawGeometryInstanced(const IBuffer* pBuffer, int instanceCount) override;
	void						DrawIndexedGeometry( const IBuffer* pBuffer, TDrawStyle style = T_TRIANGLES ) override;
	void						DrawIndexedGeometryInstanced(const IBuffer* pBuffer, TDrawStyle style = T_TRIANGLES, int instanceCount = 1) override;
	void						DrawBase( const CMatrix& mBase, float fSize );
	void						DrawLine(const CVector& p1, const CVector& p2, const CVector& color);
	void						DrawLineInternal( const CVector& p1, const CVector& p2, const CVector& color  );
	void						DrawBox( const CVector& oMinPoint, const CVector& oDimension );
	int							CreateTexture1D( float* pTexelsArray, int nSize, TPixelFormat format );
	int							CreateTexture2D( vector< unsigned char >& pTexelsArray, int nWidth, int nWeight, TPixelFormat format );
	int							CreateMipmaps2D(vector< unsigned char>& vTexel, int nWidth, int nHeight, IRenderer::TPixelFormat format);
	void						EnableLighting( bool bEnable );
	void						SetMaterialAmbient(float* fAmbient);
	void						SetMaterialDiffuse(float* fDiffuse);
	void						SetMaterialSpecular(float* fSpecular);
	void						SetMaterialEmissive(float* fEmissive);
	void						SetMaterialShininess(float fShininess);
	void						EnableFog();
	void						Set2DMode();
	void						Set3DMode();
	void						GetResolution( unsigned int& nWidth, unsigned int& nHeight );
	void						Translate2D(float fx, float fy);
	void						EnableTextureBlend(bool bEnable);
	void						SetModelViewToIdentity();
	void						EnableLight(unsigned int LightID);
	void						DisableLight(unsigned int LightID);
	void						BeginDirectGeometry( IRenderer::TDrawStyle style );
	void						SetVertex3f( float x, float y, float z );
	void						SetVertex4f( float x, float y, float z, float w );
	void						SetColor3f( float r, float g, float b );
	void						EndDirectGeometry();
	void						DrawSphere(double dRadius, unsigned int nSliceCount, unsigned int nStackCount);
	void						DrawCylinder(double dBaseRadius, double dTopRadius, double dHeight, 
											unsigned int nSlicesCount, unsigned int nStacksCount);
	void						DrawQuad(float fLenght, float fWidth);
	void						SetLightAmbient(unsigned int nLightID, float r, float g, float b, float a);
	void						SetLightDiffuse(unsigned int nLightID, float r, float g, float b, float a);
	void						SetLightSpecular(unsigned int nLightID, float r, float g, float b, float a);
	void						SetLightAttenuation( unsigned int nLightID, float fConstant, float fLinear, float fQuadratic );
	void						SetLightSpotProp(unsigned int nLightID, float fCutoff, float fExponent);
	void						SetLightSpotDirection(unsigned int nLightID, float x, float y ,float z, float w);
	void						SetLightLocalPos(unsigned int nLightID,float x, float y, float z, float w);

	unsigned int				CreateVertexBuffer( const std::vector< float >& vData ) const;
	unsigned int				CreateVertexBuffer( const std::vector< int >& vData ) const;

	int							CreateBuffer( int nElementCount );
	void						DeleteBuffer( IBuffer* pBuffer );
	void						DeleteBuffer( unsigned int nBufferID );
	void						GetNonIndexedVertexBuffer( const std::vector< float >& vVertexBuffer, const std::vector< unsigned int >& vIndexBuffer, std::vector< float >& vOutVertexBuffer );
	void						SetCurrentBuffer( int nBufferID );
	void						FillBuffer(const std::vector< float >& vData, int nBufferID, int nOffset = 0 ) override;
	void						FillBuffer(const vector< CMatrix >& vMatrix, int nBufferID, int nOffset = 0) override;
	void						AppendBuffer( const std::vector< float >& vData );


	IShader*					GetShader( std::string sShaderName );
	void						GetDefaultShader( std::string& sDefaultShader );

	unsigned int				CreateProgram() const;
	unsigned int				CreateShader( IRenderer::TShaderType ) const;
	void						DeleteShader( unsigned int nShaderID ) const;
	void						DeleteProgram( unsigned int nProgramID ) const;
	void						SetShaderSource( unsigned int nShaderID, const char* pSource ) const;
	void						LoadShader( string sShaderName);
	void						CompileShader( unsigned int nShaderID ) const;
	void						AttachShaderToProgram( unsigned int nProgramID, unsigned int nShaderID ) const;
	void						LinkProgram( unsigned int nProgramID ) const;
	virtual void				UseProgram( unsigned int nProgramID ) const;
	virtual unsigned int		GetUniformID( unsigned int nProgramID, const std::string& sVariableName ) const;
	virtual unsigned int		GetAttributeID( unsigned int nProgramID, const std::string& sVariableName ) const;
	void						SendUniform3f( unsigned int nUniformID, float f1, float f2, float f3 );
	void						SendUniform4f( unsigned int nUniformID, float f1, float f2, float f3, float f4 );
	void						SendUniform2fv( unsigned int nUniformID, int nCount, std::vector< float >& vValues ) const;
	void						SendUniform4fv( unsigned nUniformID, int nValueCount, std::vector< float >& vValues ) const;
	void						SendUniform4iv( unsigned nUniformID, int nValueCount, std::vector< int >& vValues ) const;
	void						SendUniform1i( unsigned int nUniformID, int nValue ) const;
	void						SendUniform1f( unsigned int nUniformID, float fValue ) const;
	virtual void				SendUniformMatrix4fv( unsigned int nUniformID, int nCount, bool bTranspose, std::vector< float >& vValues );
	virtual void				EnableVertexAttribArray( unsigned int nAttributeID );
	virtual void				DisableVertexAttribArray( unsigned int nAttributeID );
	virtual void				VertexAttribPointer( unsigned int nAttributeID, int nSize, TNumberType Type, int nPos );

	void						BindVertexBuffer( int nBufferID );
	void						AbonneToRenderEvent( TRenderEventCallback callback );
	void						DesabonneToRenderEvent( TRenderEventCallback callback );
	void						SetModelViewMatrix( const CMatrix& mModelView );
	void						SetProjectionMatrix( const CMatrix& oMatrix );
	void						GetInvCameraMatrix(CMatrix& oMatrix) const;
	void						SetCameraMatrix( const CMatrix& oMatrix );
	void						SetInvCameraMatrix(const CMatrix& oMatrix);
	void						GetModelMatrix(CMatrix& oMatrix);
	void						SetModelMatrix( const CMatrix& oMatrix );

	void						SetDebugString( std::string );
	void						GetDebugString( std::string& s ) const;
	void						DisplayDebugInfos( bool );
	bool						IsDisplayDebugInfos() const;
	void						SetCurrentObjectLocalTM( CMatrix& m );
	void						SetCurrentCameraMatrix( CMatrix& m );
	void						SetBackgroundColor(const CVector& vColor);
	void						SetBackgroundColor( float r, float g, float b, float a = 1.f );
	void						GetBackgroundColor( CVector& vColor );
	void						ReadPixels( int x, int y, int width, int height, vector< unsigned char >& vPixels, TPixelFormat format );
	void						LockCamera(bool lock);
	void						SetLineWidth(int width);
	void						ReloadShaders(IEventDispatcher& oEventDispatcher);
	void						ReloadShader(string shaderName);
	void						CullFace(bool enable);
	void						EnableDepthTest(bool enable);
	bool						IsCullingEnabled();
	void						CreateFrameBufferObject(int width, int height, unsigned int& nFBOId, unsigned int& nTextureId);
	void						SetCurrentFBO(int fbo);
	float						GetScreenRatio();
	void						GetGlslVersion(string& sVersion);
	void						GetOpenglVersion(string& sVersion);
	string						GetName() override;
	int							GetLastError();
};

extern "C" _declspec(dllexport) IRenderer* CreateRenderer(EEInterface& oInterface );



#endif // RENDERER_H