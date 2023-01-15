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
	int				m_nScopePos;
	int				m_nRelativeStackPosition; // relative var position from current scope ebp
	bool			m_bIsDeclared;
	int				m_nType;
	CVar() : m_bIsDeclared( false ), m_nRelativeStackPosition(0){}
};


class CVarMap
{
	typedef map< int, map< string, CVar > >	VarMap;
public:
	CVar* GetVariable(string sVarName);
	void AddVariable(string sVarName, int nScope, int nIndex, int type);
	int GetVarCountInScope(int nScope);
	int GetVarCountInScope(const CSyntaxNode& node, int nScope);
	CVar* GetVar(int nScope, string sVarName);

	VarMap&		GetVarMap();
private:
	VarMap	m_mVars;

};

class CSemanticAnalyser
{
	typedef map< string, pair< ScriptFunction, pair<vector< TFuncArgType >, TFuncArgType> > > FuncMap;
	
	FuncMap						m_mInterruption;
	set<string>					m_vCommand;
	map< string, int >			m_mStringAddress;
	map< int, string >			m_mAddressString;
	CVarMap						m_oVars;
	int							m_nCurrentScopeNumber;
	int							m_nVariableIndex;

	map<TFuncArgType, CSyntaxNode::Type>	m_mTypes;
	map<CSyntaxNode::Type, string>			m_mTypeToString;

public:
	CSemanticAnalyser();
	void			RegisterFunction( std::string sFunctionName, ScriptFunction Function, const vector< TFuncArgType >& vArgsType, TFuncArgType returnType);
	void			CompleteSyntaxicTree(CSyntaxNode& oTree, vector<string> vFunctions);
	void			GetFunctionAddress( map< string, int >& mFuncAddr );
	unsigned int	GetFuncArgsCount( int nFuncIndex );
	float			CallInterruption( int nIndex, const vector< float >& vArgs );
	void			GetRegisteredFunctions( vector< string >& vFuncNames );
	CVarMap&		GetVarMap();
	const CVar*		GetVariable(string varName);

protected:
	void			SetType(CSyntaxNode& oTree, vector<string> vFunctions, bool checkAPIArgs = false);
	void			SetTypeFromChildType(CSyntaxNode& oTree);
	void			CheckApiArgs(CSyntaxNode& oTree, const vector< TFuncArgType >& vArgs);
	void			AddNewVariable(CSyntaxNode& oTree);
	int				GetUnresolvedNodeCount(CSyntaxNode& node);
	CSyntaxNode::Type	GetFunctionReturnType(CSyntaxNode& node);
};

#endif // SEMANTICANALYSER_H