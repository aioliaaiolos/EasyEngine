#ifndef SEMANTICANALYSER_H
#define SEMANTICANALYSER_H

#include <set>
#include "SyntaxAnalyser.h"
#include "IScriptManager.h"

class CScriptState : public IScriptState
{
	vector< IScriptFuncArg* >	m_vArg;
	float						m_fReturnValue;

public:
	IScriptFuncArg* GetArg( int iIndex );
	void			AddArg( IScriptFuncArg* pArg );
	void			SetReturnValue(float ret);
	float			GetReturnValue();
};

struct CVar
{
	int		m_nScopePos;
	int		m_nRelativeStackPosition; // relative var position from current scope ebp
	bool	m_bIsDeclared;
	CVar() : m_bIsDeclared( false ), m_nRelativeStackPosition(0){}
};


class CVarMap
{
	typedef map< int, map< string, CVar > >	VarMap;
public:
	CVar* GetVariable(string sVarName);
	void AddVariable(string sVarName, int nScope, int nIndex);
	int GetVarCountInScope(const CSyntaxNode& node, int nScope);
	CVar* GetVar(int nScope, string sVarName);

	VarMap&		GetVarMap();
private:
	VarMap	m_mVars;

};

class CSemanticAnalyser
{
	typedef map< string, pair< ScriptFunction, vector< TFuncArgType > > > FuncMap;
	
	FuncMap						m_mInterruption;
	set<string>					m_vCommand;
	map< string, int >			m_mStringAddress;
	map< int, string >			m_mAddressString;
	CVarMap						m_oVars;
	int							m_nCurrentScopeNumber;
	int							m_nVariableIndex;

public:
	CSemanticAnalyser();
	void			RegisterFunction( std::string sFunctionName, ScriptFunction Function, const vector< TFuncArgType >& vArgsType );
	void			CompleteSyntaxicTree( CSyntaxNode& oTree, vector<string> vFunctions);
	void			GetFunctionAddress( map< string, int >& mFuncAddr );
	void			SetTypeFromChildType( CSyntaxNode& oTree );
	unsigned int	GetFuncArgsCount( int nFuncIndex );
	float			CallInterruption( int nIndex, const vector< float >& vArgs );
	void			GetRegisteredFunctions( vector< string >& vFuncNames );
	CVarMap&		GetVarMap();
	const CVar*		GetVariable(string varName);

protected:
	void			AddNewVariable(CSyntaxNode& oTree);
};

#endif // SEMANTICANALYSER_H