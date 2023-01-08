#ifndef ISCRIPTMANAGER_H
#define ISCRIPTMANAGER_H

// stl
#include <string>
#include <vector>

// Engine
#include "EEPlugin.h"
#include "IFileSystem.h"

using namespace std;

struct IScriptFuncArg
{
	virtual ~IScriptFuncArg() = 0 {}
};

struct CScriptFuncArgInt : public IScriptFuncArg
{
	int m_nValue;
	CScriptFuncArgInt(){}
	CScriptFuncArgInt( int nValue ) : m_nValue( nValue ){}
};

struct CScriptFuncArgFloat : public IScriptFuncArg
{
	float	m_fValue;
	CScriptFuncArgFloat(){}
	CScriptFuncArgFloat( float fValue ) : m_fValue( fValue ){}
};

struct CScriptFuncArgString : public IScriptFuncArg
{
	string m_sValue;
	CScriptFuncArgString(){}
	CScriptFuncArgString( string sValue ) : m_sValue( sValue ){}
};

class IScriptState
{
public:
	virtual IScriptFuncArg* GetArg( int iIndex ) = 0;
	virtual void			SetReturnValue(float ret) = 0;
};

typedef void ( *ScriptFunction )( IScriptState* );

enum TFuncArgType
{
	eInt = 0,
	eFloat,
	eString
};

class IScriptManager : public CPlugin
{
protected:
	IScriptManager() : CPlugin( nullptr, ""){}

public:

	
	virtual void	RegisterFunction( std::string sFunctionName, ScriptFunction Function, const vector< TFuncArgType >& vArgsType ) = 0;
	virtual void	ExecuteCommand( std::string sCommand ) = 0;
	virtual void	ExecuteByteCode(const vector<unsigned char>& vByteCode) = 0;
	virtual void	GetRegisteredFunctions( vector< string >& vFuncNames ) = 0;
	virtual float	GetVariableValue(string variableName) = 0;
	virtual float	GetRegisterValue(string sRegisterName) = 0;
	virtual void	GenerateAssemblerListing(bool generate) = 0;
	virtual void	Compile(string script, vector<unsigned char>& vByteCode) = 0;
};

#endif // ISCRIPTMANAGER_H