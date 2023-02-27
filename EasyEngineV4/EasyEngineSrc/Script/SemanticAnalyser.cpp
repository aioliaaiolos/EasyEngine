#include "SemanticAnalyser.h"
#include "Exception.h"
#include <sstream>
#include <algorithm>

void CScriptState::AddArg( IValue* pArg )
{
	m_vArg.push_back( pArg );
}

void CScriptState::SetReturnValue(float ret)
{
	m_fReturnValue = ret;
}

float CScriptState::GetReturnValue()
{
	return m_fReturnValue;
}

IValue* CScriptState::GetArg( int iIndex )
{
	if( iIndex >= m_vArg.size() )
	{
		CBadArgCountException e( iIndex );
		throw e;
	}
	return m_vArg[ iIndex ];
}

CVar* CVarMap::GetVariable(string sVarName)
{
	int nScope = -1;
	for (const pair<int, map<string, CVar>>& scopeVar : m_mVars) {
		for (const pair<string, CVar>& var : scopeVar.second) {
			if (var.first == sVarName) {
				nScope = scopeVar.first;
				return &m_mVars[nScope][sVarName];
			}
		}
	}
	return nullptr;
}

void CVarMap::AddVariable(string sVarName, int nScope, int nIndex, int type)
{
	CVar v;
	v.m_nScopePos = nScope;
	v.m_nRelativeStackPosition = nIndex;
	v.m_nType = type;
	m_mVars[nScope][sVarName] = v;
}

int CVarMap::GetVarCountInScope(const CSyntaxNode& node, int nScope)
{
	int nVarCountInThisScope = 0;
	VarMap::iterator itScope = m_mVars.find(nScope);
	if (itScope != m_mVars.end()) {
		for (const pair<string, CVar>& pairNameVar : itScope->second) {
			if (node.FindVar(pairNameVar.first)) {
				nVarCountInThisScope++;
			}
		}
	}
	return nVarCountInThisScope;
}

int CVarMap::GetVarCountInScope(int nScope)
{
	VarMap::iterator itScope = m_mVars.find(nScope);
	if (itScope != m_mVars.end())
		return itScope->second.size();
	return 0;
}

CVar* CVarMap::GetVar(int nScope, string sVarName)
{
	VarMap::iterator itScope = m_mVars.find(nScope);
	if (itScope != m_mVars.end()) {
		map<string, CVar>::iterator itVar = itScope->second.find(sVarName);
		if (itVar != itScope->second.end()) {
			return &itVar->second;
		}
	}
	return nullptr;
}

CVarMap::VarMap& CVarMap::GetVarMap()
{
	return m_mVars;
}

CSemanticAnalyser::CSemanticAnalyser():
	m_nCurrentScopeNumber( 0 ),
	m_nVariableIndex(0)
{
	m_vCommand.insert("return");
	m_mTypes[TFuncArgType::eFloat] = CSyntaxNode::eFloat;
	m_mTypes[TFuncArgType::eInt] = CSyntaxNode::eInt;
	m_mTypes[TFuncArgType::eString] = CSyntaxNode::eString;
	m_mTypes[TFuncArgType::eVoid] = CSyntaxNode::eVoid;

	m_mTypeToString[CSyntaxNode::eFloat] = "float";
	m_mTypeToString[CSyntaxNode::eInt] = "int";
	m_mTypeToString[CSyntaxNode::eString] = "string";
	m_mTypeToString[CSyntaxNode::eVoid] = "void";
}

void CSemanticAnalyser::RegisterFunction( std::string sFunctionName, ScriptFunction Function, const vector< TFuncArgType >& vArgsType, TFuncArgType returnType)
{
	FuncMap::const_iterator it = m_mInterruption.find( sFunctionName );
	if (it == m_mInterruption.end())
	{
		m_mInterruption[sFunctionName].first = Function;
		m_mInterruption[sFunctionName].second.first = vArgsType;
		m_mInterruption[sFunctionName].second.second = returnType;
	}
	else
		throw CEException("Error in CSemanticAnalyser::RegisterFunction() : function '" + sFunctionName + "' already registered");
}

void CSemanticAnalyser::GetRegisteredFunctions( vector< string >& vFuncNames )
{
	for( FuncMap::iterator itFunc = m_mInterruption.begin(); itFunc != m_mInterruption.end(); itFunc++ )
		vFuncNames.push_back( itFunc->first );
}

CVarMap& CSemanticAnalyser::GetVarMap()
{
	return m_oVars;
}

const CVar* CSemanticAnalyser::GetVariable(string varName)
{
	return m_oVars.GetVariable(varName);
}

void CSemanticAnalyser::CompleteSyntaxicTree(CSyntaxNode& oTree, vector<string> vFunctions)
{
	int nUnresolvedNode = 0;
	do {
		SetType(oTree, vFunctions);
		SetTypeFromChildType(oTree);
		int lastUnresolvedNodeCount = nUnresolvedNode;
		nUnresolvedNode = GetUnresolvedNodeCount(oTree);
		if (lastUnresolvedNodeCount > 0 && lastUnresolvedNodeCount <= nUnresolvedNode) {
			CCompilationErrorException e(-1, -1);
			e.SetErrorMessage("Error : semantic analyser is not able to resolve all nodes.");
			throw e;
		}
	} while (nUnresolvedNode > 0);
	SetType(oTree, vFunctions, true);
}

void CSemanticAnalyser::SetType( CSyntaxNode& oTree, vector<string> vFunctions, bool checkAPIArgs)
{
	if (oTree.m_Lexem.m_eType == CLexem::eCall || oTree.m_Lexem.m_eType == CLexem::eAPICall)
	{
		FuncMap::iterator it = m_mInterruption.find(oTree.m_Lexem.m_sValue);
		if (it == m_mInterruption.end())
		{
			vector<string>::iterator itFunc = std::find(vFunctions.begin(), vFunctions.end(), oTree.m_Lexem.m_sValue);
			if (itFunc == vFunctions.end()) {
				string sMessage = string("Fonction \"") + oTree.m_Lexem.m_sValue + "\" non définie";
				CCompilationErrorException e(-1, -1);
				e.SetErrorMessage(sMessage);
				throw e;
			}
			else {
				oTree.m_nAddress = (unsigned int)std::distance(vFunctions.begin(), itFunc);
				oTree.m_eType = CSyntaxNode::eVoid;
			}
		}
		else {
			if (it->second.second.first.size() != oTree.m_vChild.size()) {
				ostringstream oss;
				oss << "Fonction \"" << oTree.m_Lexem.m_sValue << "\" : nombre d'argument incorrect, " << it->second.second.first.size() << " arguments requis";
				CCompilationErrorException e(-1, -1);
				e.SetErrorMessage(oss.str());
				throw e;
			}
			oTree.m_Lexem.m_eType = CLexem::eAPICall;
			oTree.m_nAddress = (unsigned int)std::distance(m_mInterruption.begin(), it);
			oTree.m_eType = GetFunctionReturnType(oTree);
			if(checkAPIArgs)
				CheckApiArgs(oTree, it->second.second.first);
		}
	}
	else if ((oTree.m_Lexem.m_eType == CLexem::Type::eComp) ||
		(oTree.m_Lexem.m_eType == CLexem::Type::eInf) ||
		(oTree.m_Lexem.m_eType == CLexem::Type::eSup)) {
		oTree.m_eType = CSyntaxNode::eInt;
	}
	else if (oTree.m_eType == CSyntaxNode::eVal ||
		oTree.m_eType == CSyntaxNode::eInt ||
		oTree.m_eType == CSyntaxNode::eFloat ||
		oTree.m_eType == CSyntaxNode::eString)
	{
		if (oTree.m_vChild.size() == 0)
		{
			switch (oTree.m_Lexem.m_eType)
			{
			case CLexem::eInt:
				oTree.m_eType = CSyntaxNode::eInt;
				break;
			case CLexem::eFloat:
				oTree.m_eType = CSyntaxNode::eFloat;
				break;
			case CLexem::eString:
			{
				oTree.m_eType = CSyntaxNode::eString;
#ifndef STRING_IN_BIN
				map< string, int >::iterator itString = m_mStringAddress.find(oTree.m_Lexem.m_sValue);
				if (itString == m_mStringAddress.end())
				{
					m_mStringAddress.insert(map< string, int >::value_type(oTree.m_Lexem.m_sValue, (int)m_mStringAddress.size()));
					oTree.m_nAddress = (unsigned int)m_mStringAddress.size() - 1;
					m_mAddressString[oTree.m_nAddress] = oTree.m_Lexem.m_sValue;
				}
				else
					oTree.m_nAddress = itString->second;
#endif // 0
				break;
			}
			case CLexem::eVar:
				AddNewVariable(oTree);
				break;
			default:
				throw 1;
				break;
			}
		}
	}

	else if (oTree.m_eType == CSyntaxNode::eInstr) {
		if ((oTree.m_vChild.size() == 1) && (oTree.m_vChild[0].m_Lexem.m_eType == CLexem::eVar)) {
			oTree.m_vChild[0].m_eType = CSyntaxNode::eCommand;
			oTree.m_vChild[0].m_Lexem.m_eType = CLexem::eNone;
		}
	}
	else if (oTree.m_Lexem.m_eType == CLexem::eFunctionDef)
		m_nVariableIndex = 0;
	else if (oTree.m_eType == CSyntaxNode::eScope)
		m_nCurrentScopeNumber++;
	else if (oTree.m_Lexem.m_eType == CLexem::eIf)
		oTree.m_eType = CSyntaxNode::eVoid;
	for( unsigned int i = 0; i < oTree.m_vChild.size(); i++ )
		SetType( oTree.m_vChild[ i ], vFunctions);
		
	if (oTree.m_eType == CSyntaxNode::eScope)
		m_nCurrentScopeNumber--;
}

void CSemanticAnalyser::SetTypeFromChildType(CSyntaxNode& oTree)
{
	vector< int > vChildToAnalayse;
	if (oTree.m_vChild.size() > 1 && (oTree.m_Lexem.IsOperation() || oTree.m_Lexem.IsComparaison() || oTree.m_Lexem.m_eType == CLexem::eAffect)) {
		if (oTree.m_vChild[0].m_eType == CSyntaxNode::eInt && oTree.m_vChild[1].m_eType == CSyntaxNode::eFloat)
			oTree.m_eType = CSyntaxNode::eFloat;
		else if (oTree.m_vChild[0].m_eType == CSyntaxNode::eFloat && oTree.m_vChild[1].m_eType == CSyntaxNode::eInt)
			oTree.m_eType = CSyntaxNode::eFloat;
		else if (oTree.m_vChild[0].m_eType == CSyntaxNode::eFloat && oTree.m_vChild[1].m_eType == CSyntaxNode::eFloat)
			oTree.m_eType = CSyntaxNode::eFloat;
		else if (oTree.m_vChild[0].m_eType == CSyntaxNode::eInt && oTree.m_vChild[1].m_eType == CSyntaxNode::eInt)
			oTree.m_eType = CSyntaxNode::eInt;
		else if (oTree.m_vChild[0].m_eType == CSyntaxNode::eString && oTree.m_vChild[1].m_eType == CSyntaxNode::eString)
			oTree.m_eType = CSyntaxNode::eString;
	}
	if (oTree.m_Lexem.m_eType == CLexem::eAffect) {
		if (!oTree.m_vChild[0].IsResolved() && oTree.m_vChild[1].IsResolved()) {
			oTree.m_vChild[0].m_eType = oTree.m_vChild[1].m_eType;
			if (oTree.m_vChild[0].m_Lexem.m_eType == CLexem::eVar) {
				CVar* pVar = m_oVars.GetVariable(oTree.m_vChild[0].m_Lexem.m_sValue);
				pVar->m_nType = oTree.m_vChild[1].m_eType;
			}
		}
		if (oTree.m_vChild[1].m_eType == CSyntaxNode::eVoid) {
			CCompilationErrorException e(-1, -1);
			e.SetErrorMessage("Error : try to assign a 'void' value to '" + oTree.m_vChild[0].m_Lexem.m_sValue + "'");
			throw e;
		}
		oTree.m_eType = oTree.m_vChild[0].m_eType;
	}
	for (CSyntaxNode& oChild : oTree.m_vChild)
		SetTypeFromChildType(oChild);
}

void CSemanticAnalyser::CheckApiArgs(CSyntaxNode& oTree, const vector< TFuncArgType >& vArgs)
{
	int argIndex = 1;
	vector<CSyntaxNode>::iterator itChild = oTree.m_vChild.begin();
	for (const TFuncArgType& type : vArgs) {
		if (m_mTypes[type] != itChild->m_eType) {
			CCompilationErrorException e(-1, -1);
			CSyntaxNode::Type eExpected = m_mTypes[type];
			string sExpected = m_mTypeToString[eExpected];
			CSyntaxNode::Type eCurrent = itChild->m_eType;
			string sCurrent = m_mTypeToString[eCurrent];
			if (!sCurrent.empty()) {
				e.SetErrorMessage(string("Warning : function '") + oTree.m_Lexem.m_sValue + "' argument " + std::to_string(argIndex) +
					" is a " + sCurrent + ", a " + sExpected + " is expected");
				throw e;
			}
		}
		argIndex++;
		itChild++;
	}
}

void CSemanticAnalyser::AddNewVariable(CSyntaxNode& oTree)
{
	string sVarName = oTree.m_Lexem.m_sValue;
	CVar* pVar = m_oVars.GetVariable(sVarName);
	if(!pVar) {
		if (m_nCurrentScopeNumber == 0)
			m_nVariableIndex = m_oVars.GetVarCountInScope(0);
		m_oVars.AddVariable(sVarName, m_nCurrentScopeNumber, m_nVariableIndex, oTree. m_eType);
		if (m_nCurrentScopeNumber != 0)
			m_nVariableIndex++;
		oTree.m_nScope = m_nCurrentScopeNumber;
	}
	else
		oTree.m_eType = (CSyntaxNode::Type)pVar->m_nType;
}

int CSemanticAnalyser::GetUnresolvedNodeCount(CSyntaxNode& node)
{
	int result = node.IsResolved() ? 0 : 1;
	for (CSyntaxNode& child : node.m_vChild)
		result += GetUnresolvedNodeCount(child);
	return result;
}

CSyntaxNode::Type CSemanticAnalyser::GetFunctionReturnType(CSyntaxNode& node)
{
	string sFunctionName = node.m_Lexem.m_sValue;
	FuncMap::iterator itFunc = m_mInterruption.find(sFunctionName);
	if (itFunc != m_mInterruption.end())
		return m_mTypes[itFunc->second.second.second];
	return node.m_eType;
}

void CSemanticAnalyser::GetFunctionAddress( map< string, int >& mFuncAddr )
{
	for( FuncMap::iterator it = m_mInterruption.begin(); it != m_mInterruption.end(); ++it )
		mFuncAddr[ it->first ] = (int)distance( m_mInterruption.begin(), it );
}

unsigned int CSemanticAnalyser::GetFuncArgsCount( int nFuncIndex )
{
	FuncMap::iterator itFunc = m_mInterruption.begin();
	std::advance( itFunc, nFuncIndex );
	return (unsigned int)itFunc->second.second.first.size();
}

float CSemanticAnalyser::CallInterruption( int nIntIndex, const vector< float >& vArgs )
{
	FuncMap::iterator itFunc = m_mInterruption.begin();
	std::advance( itFunc, nIntIndex );
	CScriptState* pState = new CScriptState;
	vector< TFuncArgType >& vArgType = itFunc->second.second.first;
	try
	{
		for( unsigned int i = 0; i < vArgs.size(); i++ )
		{
			IValue* pArg = NULL;
			switch( vArgType[ i ] )
			{
			case eFloat:
				pArg = new CValueFloat( vArgs[ i ] );
				break;
			case eInt:
				pArg = new CValueInt( (int)vArgs[ i ] );
				break;
			case eString:
				pArg = new CValueString( m_mAddressString[ (int)vArgs[ i ] ] );
				break;
			}
			pState->AddArg( pArg );
		}
		itFunc->second.first( pState );
		return pState->GetReturnValue();
	}
	catch( CBadArgCountException& e )
	{
		ostringstream oss;
		oss << itFunc->first + " : nombre d'arguments incorrect, " << itFunc->second.second.first.size() << " arguments requis";
		e.SetErrorMessage( oss.str() );
		throw e;
	}
}

