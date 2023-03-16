#include "Shader.h"
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "gl/glext.h"
#include "IFileSystem.h"
#include "Renderer.h"
#include "Math/Matrix.h"
#include "Utils2/StringUtils.h"
#include "Exception.h"


#include <map>
using namespace std;



CShader::CShader( CRenderer& oRenderer ):
m_oRenderer( oRenderer )
{
	m_nPixelShader = 0;
	m_nVertexShader = 0;
	m_nProgram = m_oRenderer.CreateProgram();
}



CShader::~CShader()
{
	DeleteShadersAndProgram();
}

void CShader::Attach( string sShaderFileName, IRenderer::TShaderType type, IFileSystem& oFileSystem )
{
	unsigned int nShaderID = m_oRenderer.CreateShader( type );
	
	switch( type )
	{
	case IRenderer::T_PIXEL_SHADER:
		m_nPixelShader = nShaderID;
		break;
	case IRenderer::T_VERTEX_SHADER:
		m_nVertexShader = nShaderID;
		break;
	}
	
	char* pSource = LoadSource( sShaderFileName, oFileSystem );
	if ( pSource == NULL )
	{
		string sMessage = sShaderFileName + " introuvable";
		exception e( sMessage.c_str() );
		throw e;
	}
	m_oRenderer.SetShaderSource( nShaderID, pSource );
	m_oRenderer.CompileShader(nShaderID);
	m_mShaderName[ sShaderFileName ] = nShaderID;
	m_oRenderer.AttachShaderToProgram( m_nProgram, nShaderID );
	delete pSource;
}

void CShader::Link()
{
	m_oRenderer.LinkProgram( m_nProgram );
}

char* CShader::LoadSource( string sFileName, IFileSystem& oFileSystem )
{
	FILE* pFile = oFileSystem.OpenFile( sFileName, "r" ); 
	if ( pFile == NULL )
		return NULL;
	fseek( pFile, 0, SEEK_END );
	long nPos = ftell( pFile );
	nPos -= 5;
	if ( nPos < 1 )
	{
		string sMessage = string( "Erreur lors du chargement du shader " ) + sFileName + " : taille du fichier trop petite";
		exception e( sMessage.c_str() );
		throw e;
	}
	char* pSource = new char[ nPos + 1 ];
	memset( pSource, 0, nPos + 1 );
	rewind( pFile );
	fread( pSource, 1, nPos , pFile );
	pSource[ nPos ] = 0;
	fclose( pFile );
	CStringUtils::GetShortFileName(sFileName, sFileName);
	CStringUtils::GetFileNameWithoutExtension( sFileName, m_sName );
	return pSource;
}

void CShader::Enable( bool bEnable )
{
	if ( bEnable )
		m_oRenderer.UseProgram( m_nProgram );
	else
		m_oRenderer.UseProgram( 0 );
}

void CShader::Reload(IFileSystem& oFileSystem)
{
	for (map<string, GLuint>::iterator it = m_mShaderName.begin(); it != m_mShaderName.end(); it++) {
		string path = it->first;
		char* pSource = LoadSource(path, oFileSystem);
		int id = it->second;
		m_oRenderer.SetShaderSource(id, pSource);
		try {
			m_oRenderer.CompileShader(id);
			m_oRenderer.AttachShaderToProgram(m_nProgram, id);
		}
		catch (exception e) {
			string msg = "Erreur lors de la compilation de \"" + path + "\" : \n" + e.what();
			CEException e2(msg);
			throw e2;
		}
		delete pSource;
	}
}

int CShader::GetUniformID( const std::string& sVariableName ) const
{
	m_oRenderer.UseProgram( m_nProgram );
	try
	{
		return m_oRenderer.GetUniformID( m_nProgram, sVariableName );
	}
	catch( exception& e )
	{
		string sShaderName = m_mShaderName.begin()->first.substr( 0, m_mShaderName.begin()->first.size() - 3 );
		string s = sShaderName + " : " + e.what();
		exception e2( s.c_str() );
		throw e2;
	}
	return -1;
}

int CShader::GetAttributeID( const std::string& sVariableName )
{
	m_oRenderer.UseProgram( m_nProgram );
	return m_oRenderer.GetAttributeID( m_nProgram, sVariableName );
}

void CShader::GetName( string& sName )
{
	sName = m_sName;
}

void CShader::GetFilePath(string& path)
{
	int idx = m_mShaderName.begin()->first.find(".");
	path = m_mShaderName.begin()->first.substr(0, idx);
}

int CShader::GetID()
{
	return m_mShaderName.begin()->second;
}

void CShader::AttribDivisor(unsigned int id, unsigned int nSize)
{
	//glAttribDivisor(id, nSize);
	throw CMethodNotImplementedException("CShader::AttribDivisor()");
}

void CShader::DeleteShadersAndProgram()
{
	if (m_nPixelShader != 0)
		m_oRenderer.DeleteShader(m_nPixelShader);
	if (m_nVertexShader != 0)
		m_oRenderer.DeleteShader(m_nVertexShader);
	if (m_nProgram != 0)
		m_oRenderer.DeleteProgram(m_nProgram);
}

void CShader::SendUniformVec3f( std::string sVariableName, float x, float y, float z )
{
	int id = GetUniformID( sVariableName );
	m_oRenderer.SendUniform3f( id, x, y, z );
}

void CShader::SendUniformVec4f( std::string sVariableName, float x, float y, float z, float w )
{
	int id = GetUniformID( sVariableName );
	m_oRenderer.SendUniform4f( id, x, y, z, w );
}

void CShader::SendUniformVec3f( std::string sVariableName, CVector v )
{
	SendUniformVec3f( sVariableName, v.m_x, v.m_y, v.m_z );
}

void CShader::SendUniformVec4f( std::string sVariableName, CVector v )
{
	SendUniformVec4f( sVariableName, v.m_x, v.m_y, v.m_z, v.m_w );
}

void CShader::SendUniformValues( const std::string& sVariableName, int i ) const
{
	int id = GetUniformID( sVariableName );
	m_oRenderer.SendUniform1i( id, i );
}

void CShader::SendUniformValues( const std::string& sVariableName, float f ) const
{
	int id = GetUniformID( sVariableName );
	m_oRenderer.SendUniform1f( id, f );
}

void CShader::SendUniformVectorArray(const string& sVariableName, vector< int >& vVector) const
{
	int id = GetUniformID(sVariableName);
	m_oRenderer.SendUniform1iv(id, (int)vVector.size(), vVector);
}

void CShader::SendUniformVectorArray(const string& sVariableName, vector< float >& vVector) const
{
	int id = GetUniformID(sVariableName);
	m_oRenderer.SendUniform1fv(id, (int)vVector.size(), vVector);
}

void CShader::SendUniformVector4Array( const string& sVariableName, vector< float >& vVector )
{
	int id = GetUniformID( sVariableName );
	m_oRenderer.SendUniform4fv( id, (int)vVector.size() / 4, vVector ); 
}

void CShader::SendUniformVector4Array( const string& sVariableName, vector< int >& vVector )
{
	int id = GetUniformID( sVariableName );
	m_oRenderer.SendUniform4iv( id, (int)vVector.size() / 4, vVector );
}

void CShader::SendUniformMatrix4(const std::string& sVariableName, const CMatrix& oMatrix, bool bTranspose)
{
	int id = GetUniformID(sVariableName);
	vector< float > vArray;
	oMatrix.Get(vArray);
	m_oRenderer.SendUniformMatrix4fv(id, 1, bTranspose, vArray);
}

void CShader::SendUniformMatrix4Array( const string& sVariableName, vector< CMatrix >& vMatrix, bool bTranspose )
{
	int id = GetUniformID( sVariableName );
	int nArraySize = (int)vMatrix.size() * 16;
	vector< float > vArray;
	vArray.resize( nArraySize );
	for ( unsigned int i = 0; i < vMatrix.size(); i++ )
		vMatrix[ i ].Get( &vArray[ i * 16 ] );
	
	m_oRenderer.SendUniformMatrix4fv( id, (int)vMatrix.size(), bTranspose, vArray );
}

void CShader::SendUniformVec2Array( const std::string& sVariableName, std::vector< float >& vArray )
{
	int id = GetUniformID( sVariableName );
	m_oRenderer.SendUniform2fv( id, (int)vArray.size() / 2, vArray );
}

unsigned int CShader::EnableVertexAttribArray( const std::string& sAttribName )
{
	int id = GetAttributeID( sAttribName );
	m_oRenderer.EnableVertexAttribArray( id );
	return id;
}

void CShader::DisableVertexAttribArray( unsigned int nAttribID )
{
	m_oRenderer.DisableVertexAttribArray( nAttribID );
}

void CShader::VertexAttributePointerf( unsigned int id, int nSize, int nPos )
{
	m_oRenderer.VertexAttribPointer( id, nSize, IRenderer::T_FLOAT, nPos );
}
