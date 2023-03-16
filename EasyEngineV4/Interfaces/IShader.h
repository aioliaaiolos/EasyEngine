#ifndef ISHADER_H
#define ISHADER_H

// stl
#include <string>
#include <vector>

#include "Math/Matrix.h"
#include "Math/Vector.h"

using namespace std;

class IShader
{
public:
	virtual void			Enable( bool bEnable ) = 0;
	virtual unsigned int	EnableVertexAttribArray( const std::string& sAttribName ) = 0;
	virtual void			DisableVertexAttribArray( unsigned int nAttribID ) = 0;
	virtual void 			VertexAttributePointerf( unsigned int id, int nSize, int nPos ) = 0;
	virtual void			SendUniformVec3f( std::string sVariableName, float x, float y, float z ) = 0;
	virtual void			SendUniformVec4f( std::string sVariableName, float x, float y, float z, float w ) = 0;
	virtual void			SendUniformVec3f( std::string sVariableName, CVector v ) = 0;
	virtual void			SendUniformVec4f( std::string sVariableName, CVector v ) = 0;
	virtual void 			SendUniformMatrix4(const std::string& sVariableName, const CMatrix& oMatrix, bool bTranspose = false) = 0;
	virtual void 			SendUniformMatrix4Array( const std::string& sVariableName, std::vector< CMatrix >& vMatrix, bool bTranspose = false ) = 0;
	virtual void			SendUniformVectorArray(const string& sVariableName, vector< int >& vVector) const = 0;
	virtual void			SendUniformVectorArray(const string& sVariableName, vector< float >& vVector) const = 0;
	virtual void 			SendUniformVector4Array(const std::string& sVariableName, std::vector< float >& vVector) = 0;
	virtual void 			SendUniformVector4Array(const std::string& sVariableName, std::vector< int >& vVector) = 0;
	virtual void			SendUniformValues( const std::string& sVariableName, float f ) const = 0;
	virtual void			SendUniformValues( const std::string& sVariableName, int nValue ) const = 0;
	virtual void			SendUniformVec2Array( const std::string& sVariableName, std::vector< float >& vArray ) = 0;
	virtual void			GetName( string& sName ) = 0;
	virtual void			AttribDivisor(unsigned int id, unsigned int nSize) = 0;
};

#endif // ISHADER_H