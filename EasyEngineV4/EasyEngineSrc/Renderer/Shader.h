#ifndef SHADER_H
#define SHADER_H

// stl
#include <map>
#include <string>
#include <vector>

#include "IShader.h"
#include "IRenderer.h"

class IFileSystem;
class CMatrix;
class CRenderer;

typedef unsigned int GLuint;

class CShader : public IShader
{
	CRenderer&						m_oRenderer;
	GLuint							m_nPixelShader;
	GLuint							m_nVertexShader;
	GLuint							m_nProgram;
	std::map< std::string, GLuint >	m_mShaderName;
	//int								m_nCurrentAttributeID;

	char*							LoadSource( std::string sFileName, IFileSystem& oFileSystem );
	int								GetUniformID( const std::string& sVariableName ) const;
	string							m_sName;
	

public:
	CShader( CRenderer& oRenderer );
	virtual ~CShader();
	
	void 			Attach( std::string sShaderFileName, IRenderer::TShaderType type, IFileSystem& oFileSystem );
	void 			DetachShader( const char* pShaderFileName );
	void 			Link();
	void 			Enable( bool bEnable );
	void			Reload(IFileSystem& oFileSystem);
	void 			SendUniformValues( const std::string& sVariableName, int i ) const;
	void			SendUniformValues( const std::string& sVariableName, float f ) const;
	void			SendUniformVectorArray(const string& sVariableName, vector< float >& vVector) const override;
	void			SendUniformVectorArray(const string& sVariableName, vector< int >& vVector) const override;
	void			SendUniformVec3f( std::string sVariableName, float x, float y, float z );
	void			SendUniformVec4f( std::string sVariableName, float x, float y, float z, float w );
	void			SendUniformVec3f( std::string sVariableName, CVector v );
	void			SendUniformVec4f( std::string sVariableName, CVector v );
	void 			SendUniformVector4Array( const std::string& sVariableName, std::vector< float >& vVector ) override;
	void 			SendUniformVector4Array( const std::string& sVariableName, std::vector< int >& vVector ) override;
	void			SendUniformVec2Array( const std::string& sVariableName, std::vector< float >& vArray );
	void 			SendUniformMatrix4(const std::string& sVariableName, const CMatrix& oMatrix, bool bTranspose = false);
	void 			SendUniformMatrix4Array( const string& sVariableName, vector< CMatrix >& vMatrix, bool bTranspose = false ) override;
	unsigned int	EnableVertexAttribArray( const std::string& sAttribName );
	void			DisableVertexAttribArray( unsigned int nAttribID );
	void 			VertexAttributePointerf( unsigned int id, int nSize, int nPos );
	int				GetAttributeID( const std::string& sVariableName );
	void			GetName(string& path) override;
	const			string&	GetName() override;
	void			GetFilePath(string& path);
	int				GetID();
	void			AttribDivisor(unsigned int id, unsigned int nSize);
	void			DeleteShadersAndProgram();	
};

#endif // SHADER_H