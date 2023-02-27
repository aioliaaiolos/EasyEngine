#ifndef ISCRIPTMANAGER_H
#define ISCRIPTMANAGER_H

// stl
#include <string>
#include <vector>

// Engine
#include "EEPlugin.h"
#include "IFileSystem.h"

using namespace std;

struct IValue
{
	enum Type
	{
		eString = 0,
		eInt,
		eFloat
	};

	virtual ~IValue() = 0 {}
	virtual Type GetType() = 0;
};

struct CValueInt : public IValue
{
	int m_nValue;
	CValueInt(){}
	CValueInt( int nValue ) : m_nValue( nValue ){}
	Type GetType() override { return eInt; }
};

struct CValueFloat : public IValue
{
	float	m_fValue;
	CValueFloat(){}
	CValueFloat( float fValue ) : m_fValue( fValue ){}
	Type GetType() override { return eFloat; }
};

struct CValueString : public IValue
{
	string m_sValue;
	CValueString(){}
	CValueString( string sValue ) : m_sValue( sValue ){}
	Type GetType() override { return eString; }
};

class IScriptState
{
public:
	virtual IValue* GetArg( int iIndex ) = 0;
	virtual void			SetReturnValue(float ret) = 0;
};

typedef void ( *ScriptFunction )( IScriptState* );

enum TFuncArgType
{
	eInt = 0,
	eFloat,
	eString,
	eVoid
};

class IScriptManager : public CPlugin
{
protected:
	IScriptManager() : CPlugin( nullptr, ""){}

public:

	
	virtual void	RegisterFunction( std::string sFunctionName, ScriptFunction Function, const vector< TFuncArgType >& vArgsType, TFuncArgType returnType) = 0;
	virtual void	ExecuteCommand( std::string sCommand ) = 0;
	virtual void	ExecuteByteCode(const vector<unsigned char>& vByteCode) = 0;
	virtual void	GetRegisteredFunctions( vector< string >& vFuncNames ) = 0;
	virtual float	GetVariableValue(string variableName) = 0;
	virtual float	GetRegisterValue(string sRegisterName) = 0;
	virtual void	GenerateAssemblerListing(bool generate) = 0;
	virtual void	Compile(string script, vector<unsigned char>& vByteCode) = 0;
	virtual void	SetVariableValue(string sVariableName, float value) = 0;
};

#endif // ISCRIPTMANAGER_H