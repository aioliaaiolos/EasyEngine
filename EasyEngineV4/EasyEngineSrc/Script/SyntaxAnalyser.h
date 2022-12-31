#ifndef SYNTAXANALYSER_H
#define SYNTAXANALYSER_H

#define VERSION2

#include "LexAnalyser.h"

#include <vector>
#include <map>
#include <set>

using namespace std;

class CSyntaxNode
{
public:
	enum NODE_TYPE
	{
		eNone = -1,
		eVal = 0,
		eOp,
		eInstr,
		eInt,
		eFloat,
		eString,
		eAPICall,
		eFunctionCall,
		eProg,
		eVecArgs,
		ePar,
		eScope,
		eCommand,
		eFunctionDef
	};

	CSyntaxNode();
	CSyntaxNode( CLexAnalyser::CLexem );
	static bool				IsValue(NODE_TYPE node);
	CLexAnalyser::CLexem	m_Lexem;
	vector< CSyntaxNode >	m_vChild;
	NODE_TYPE				m_Type;
	unsigned int			m_nAddress;
	void*					m_pUserData;
	int						m_nScope;
};

enum TParenthesisReductionType
{
	eNormal = 0,
	eFunc,
	eFuncDef
};

class CSyntaxAnalyser
{
	void ReduceScopes(CSyntaxNode& oTree);
	void ReduceParenthesis( CSyntaxNode& oTree );
	void ReduceInstruction( CSyntaxNode& oTree );
	void ReduceAllOperations(  CSyntaxNode& oNode );
	void ReduceOperations( CSyntaxNode& oNode, const vector< CLexAnalyser::CLexem::TLexem >& vType );
	void DeleteTempNodes( CSyntaxNode& oNode );
	void ReduceVecArgs( CSyntaxNode& oNode );
	void DeleteParNodes( CSyntaxNode& oNode );

public:
	void GetSyntaxicTree( const vector< CLexAnalyser::CLexem >& vLexem, CSyntaxNode& oTree );
	void ReduceLargestClosingLexem(vector< CSyntaxNode >& vNode, CLexAnalyser::CLexem::TLexem leftLexem, CLexAnalyser::CLexem::TLexem rightLexem, CSyntaxNode::NODE_TYPE nt, TParenthesisReductionType rt, unsigned int iFirst = 0);

	set<string> m_mFunctions;
	vector<string>	m_vFunctions;
};

#endif //SYNTAXANALYSER_H