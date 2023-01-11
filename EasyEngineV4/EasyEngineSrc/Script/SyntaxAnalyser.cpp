#include "SyntaxAnalyser.h"
#include "LexAnalyser.h"



CSyntaxNode::CSyntaxNode():
m_eType( eNone ),
m_nAddress( -1 ),
m_nScope(0)
{
}

CSyntaxNode::CSyntaxNode( CLexem l ):
m_eType(eNone),
m_nAddress(-1),
m_Lexem( l ),
m_nScope(0)
{
}

bool CSyntaxNode::FindVar(string sVarName) const
{
	if (m_Lexem.m_sValue == sVarName)
		return true;
	for (const CSyntaxNode& child : m_vChild) {
		if (child.FindVar(sVarName))
			return true;
	}
	return false;
}

bool CSyntaxNode::IsResolved()
{
	return m_eType != CSyntaxNode::eVal && m_eType != CSyntaxNode::eNone;
}

void CSyntaxAnalyser::ReduceInstruction( CSyntaxNode& oTree )
{
	unsigned int i = 0;
	int iBeginInstr = 0;
	while( i < oTree.m_vChild.size() )
	{
		if( oTree.m_vChild[ i ].m_Lexem.m_eType == CLexem::ePtVirg )
		{
			CSyntaxNode oInstr;
			oInstr.m_eType = CSyntaxNode::eInstr;
			for( unsigned int j = iBeginInstr; j < i; j++ )
				oInstr.m_vChild.push_back( oTree.m_vChild[ j ] );
			oTree.m_vChild.erase( oTree.m_vChild.begin() + iBeginInstr, oTree.m_vChild.begin() + i + 1 );
			i = iBeginInstr;
			iBeginInstr++;
			oTree.m_vChild.insert( oTree.m_vChild.begin() + i, 1, oInstr );
		}
		i++;
	}

	for (unsigned int i = 0; i < oTree.m_vChild.size(); i++)
		ReduceInstruction(oTree.m_vChild[i]);
}


void CSyntaxAnalyser::GetSyntaxicTree( const vector< CLexem >& vLexem, CSyntaxNode& oTree )
{
	vector< CSyntaxNode > vSyntax;
	for( unsigned int i = 0; i < vLexem.size(); i++ )
		vSyntax.push_back( CSyntaxNode( vLexem[ i ] ) );
	// On réduit toutes les valeurs
	for( unsigned int i = 0; i < vSyntax.size(); i++ )
	{
		if( vSyntax[ i ].m_Lexem.m_eType == CLexem::eFloat ||  
			vSyntax[ i ].m_Lexem.m_eType == CLexem::eInt ||
			vSyntax[ i ].m_Lexem.m_eType == CLexem::eString ||
			vSyntax[ i ].m_Lexem.m_eType == CLexem::eVar ||
			vSyntax[i].m_Lexem.m_eType == CLexem::eCall)
		{
			vSyntax[ i ].m_eType = CSyntaxNode::eVal;
		}
	}
	oTree.m_eType = CSyntaxNode::eProg;
	for( unsigned int i = 0; i < vSyntax.size(); i++ )
		oTree.m_vChild.push_back( vSyntax[ i ] );
	ReduceParenthesis(oTree);
	ReduceScopes(oTree);
	ReduceInstruction( oTree );
	
	ReduceAllOperations( oTree );
	DeleteParNodes( oTree );
	ReduceVecArgs( oTree );
}

void CSyntaxAnalyser::DeleteParNodes( CSyntaxNode& oNode )
{
	if( oNode.m_eType == CSyntaxNode::ePar )
	{
		oNode.m_eType = oNode.m_vChild[ 0 ].m_eType;
		oNode.m_Lexem = oNode.m_vChild[ 0 ].m_Lexem;
		oNode.m_vChild.swap( oNode.m_vChild[ 0 ].m_vChild );
	}
	for( unsigned int i = 0; i < oNode.m_vChild.size(); i++ )
		DeleteParNodes( oNode.m_vChild[ i ] );
}

void CSyntaxAnalyser::ReduceVecArgs( CSyntaxNode& oNode )
{
	for( unsigned int i = 0; i < oNode.m_vChild.size(); i++ )
	{
		if( oNode.m_vChild[ i ].m_eType == CSyntaxNode::eVecArgs )
		{
			if( oNode.m_vChild[ i ].m_vChild.size() > 0 )
			{
				oNode.m_vChild.swap( oNode.m_vChild[ i ].m_vChild );
				for( unsigned int j = 0; j < oNode.m_vChild.size(); j++ )
					if( oNode.m_vChild[ j ].m_Lexem.m_eType == CLexem::eVirg )
						oNode.m_vChild.erase( oNode.m_vChild.begin() + j );
			}
			else
			{
				oNode.m_vChild.erase( oNode.m_vChild.begin() + i );
			}
			break;
		}
	}
	for( unsigned int i = 0; i < oNode.m_vChild.size(); i++ )
		ReduceVecArgs( oNode.m_vChild[ i ] );
}

void CSyntaxAnalyser::DeleteTempNodes( CSyntaxNode& oNode )
{
	for( unsigned int i = 0; i < oNode.m_vChild.size(); i++ ) {
		if( oNode.m_vChild[ i ].m_eType == CSyntaxNode::ePar ) {
			if( oNode.m_vChild[ i ].m_vChild.size() == 1 && oNode.m_vChild[ i ].m_vChild[ 0 ].m_Lexem.IsOperation() )
				oNode.m_vChild[ i ] = oNode.m_vChild[ i ].m_vChild[ 0 ];
			else
				throw 1;
		}
	}
	for( unsigned int i = 0; i < oNode.m_vChild.size(); i++ )
		DeleteTempNodes( oNode.m_vChild[ i ] );
}

void CSyntaxAnalyser::ReduceAllOperations(  CSyntaxNode& oNode )
{
	vector< CLexem::Type > vTypes;
	vTypes.push_back( CLexem::eMult );
	vTypes.push_back( CLexem::eDiv );
	ReduceOperations( oNode, vTypes  );
	vTypes.clear();
	vTypes.push_back( CLexem::eAdd );
	vTypes.push_back( CLexem::eSub );
	ReduceOperations( oNode, vTypes );
	vTypes.clear();
	vTypes.push_back(CLexem::eComp);
	vTypes.push_back(CLexem::eSup);
	vTypes.push_back(CLexem::eInf);
	ReduceOperations(oNode, vTypes);
	vTypes.clear();
	vTypes.push_back( CLexem::eAffect );
	ReduceOperations( oNode, vTypes );
}

void CSyntaxAnalyser::ReduceOperations( CSyntaxNode& oNode, const vector< CLexem::Type >& vType )
{
	for( unsigned int i = 0; i < oNode.m_vChild.size(); i ++ ) {
		bool bIsType = false;
		for( unsigned int j = 0; j < vType.size(); j++ ) {
			if( oNode.m_vChild[ i ].m_Lexem.m_eType == vType[ j ] ) {
				bIsType = true;
				break;
			}
		}
		if( bIsType ) {			
			if( ( i > 0 ) && ( i < oNode.m_vChild.size() - 1 ) ) {
				bool bLeftIsParOrVal = ( ( oNode.m_vChild[ i - 1 ].m_eType == CSyntaxNode::eVal ) || ( oNode.m_vChild[ i - 1 ].m_eType == CSyntaxNode::ePar ) );
				bool bRightIsParOrVal = ( ( oNode.m_vChild[ i + 1 ].m_eType == CSyntaxNode::eVal ) || (oNode.m_vChild[ i + 1 ].m_eType == CSyntaxNode::ePar ) );
				if( bLeftIsParOrVal && bRightIsParOrVal ) {
					oNode.m_vChild[ i ].m_vChild.push_back( oNode.m_vChild[ i - 1 ] );
					oNode.m_vChild[ i ].m_vChild.push_back( oNode.m_vChild[ i + 1 ] );
					oNode.m_vChild[ i ].m_eType = CSyntaxNode::eVal;
					vector< CSyntaxNode >::iterator itErase = oNode.m_vChild.erase( oNode.m_vChild.begin() + i - 1 );
					oNode.m_vChild.erase( itErase + 1 );
					i--;
				}
			}
		}
	}
	for( unsigned int i = 0; i < oNode.m_vChild.size(); i++ )
		ReduceOperations( oNode.m_vChild[ i ], vType );
}

int min(int a, int b)
{
	return a < b ? a : b;
}


void GetClosingLexemIndices(const vector< CSyntaxNode >& vNode, CLexem::Type LClosing, CLexem::Type RClosing,
	int& iBegin, int& iEnd, unsigned int iFirst = 0, unsigned int iLast = -1)
{
	int lPar = 0;
	iBegin = iEnd = -1;
	for (unsigned int i = iFirst; i < min(iLast, (unsigned int)vNode.size()); i++) {
		if (vNode[i].m_Lexem.m_eType == LClosing) {
			lPar = 1;
			iBegin = i;
			break;
		}
	}
	unsigned int i = iBegin + 1;
	while (lPar > 0 && i < vNode.size()) {
		if (vNode[i].m_Lexem.m_eType == RClosing)
			lPar--;
		if (vNode[i].m_Lexem.m_eType == LClosing)
			lPar++;
		i++;
	}
	if (lPar == 0)
		iEnd = i - 1;
}

void CSyntaxAnalyser::ReduceLargestClosingLexem(vector<CSyntaxNode>& vNode, CLexem::Type leftLexem, CLexem::Type rightLexem, 
	CSyntaxNode::Type nt, TParenthesisReductionType rt, unsigned int iFirst)
{
	int iBegin, iEnd;
	GetClosingLexemIndices(vNode, leftLexem, rightLexem, iBegin, iEnd, iFirst);
	if (iBegin != -1 && iEnd != -1)	{
		CSyntaxNode oNode;
		oNode.m_eType = nt;
		for (int j = iBegin + 1; j < iEnd; j++)
			oNode.m_vChild.push_back(vNode[j]);
		vNode.erase(vNode.begin() + iBegin, vNode.begin() + iEnd + 1);

		vector<CSyntaxNode>::iterator itNode = vNode.begin() + iBegin - 1;
		if (itNode->m_Lexem.m_eType == CLexem::Type::eCall || 
			(itNode->m_Lexem.m_eType == CLexem::Type::eIf) || 
			(itNode->m_Lexem.m_eType == CLexem::Type::eFunctionDef)) 
		{
			if ( (itNode->m_Lexem.m_eType == CLexem::Type::eCall) && (itNode != vNode.begin())) {
				vector<CSyntaxNode>::iterator itPreviousNode = itNode - 1;
				if (itPreviousNode->m_Lexem.m_eType == CLexem::Type::eFunctionDef) {
					itNode->m_Lexem.m_eType = CLexem::Type::eFunctionDef;
					itNode->m_eType = CSyntaxNode::eFunctionDef;
					itNode = vNode.erase(itPreviousNode);
					vector<string>::iterator itFuncName = std::find(m_vFunctions.begin(), m_vFunctions.end(), itNode->m_Lexem.m_sValue);
					if (itFuncName == m_vFunctions.end())
						m_vFunctions.push_back(itNode->m_Lexem.m_sValue);
				}
			}
			vNode[iBegin - 1].m_vChild.push_back(oNode);
		}
		else {
			itNode++;
			vNode.insert(itNode, oNode);			
		}
	}
}

void CSyntaxAnalyser::ReduceScopes(CSyntaxNode& oTree)
{
	for (unsigned int i = 0; i < oTree.m_vChild.size(); i++)
	{
		int iPred = i > 0 ? i - 1 : 0;
		if (oTree.m_vChild[i].m_Lexem.m_eType == CLexem::eLBraket) {
			TParenthesisReductionType rt = oTree.m_vChild[iPred].m_Lexem.m_eType == eFuncDef ? eFuncDef : eFunc;
			ReduceLargestClosingLexem(oTree.m_vChild, CLexem::Type::eLBraket, CLexem::Type::eRBraket, CSyntaxNode::eScope, rt, i);
		}
	}
	for (unsigned int i = 0; i < oTree.m_vChild.size(); i++)
		ReduceScopes(oTree.m_vChild[i]);
}

void CSyntaxAnalyser::ReduceParenthesis( CSyntaxNode& oTree )
{
	for( unsigned int i = 0; i < oTree.m_vChild.size(); i++ )
	{
		int iPred = i > 0 ? i - 1 : 0;
		if( oTree.m_vChild[ i ].m_Lexem.m_eType == CLexem::eLPar ) {
			if( oTree.m_vChild[ iPred ].m_Lexem.m_eType == CLexem::eCall ||
				oTree.m_vChild[iPred].m_Lexem.m_eType == CLexem::eFunctionDef)
				ReduceLargestClosingLexem(oTree.m_vChild, CLexem::Type::eLPar, CLexem::Type::eRPar, CSyntaxNode::eVecArgs, eFunc, i);
			else
				ReduceLargestClosingLexem(oTree.m_vChild, CLexem::Type::eLPar, CLexem::Type::eRPar, CSyntaxNode::ePar, eNormal, i);
		}
	}
	for( unsigned int i = 0; i < oTree.m_vChild.size(); i++ )	
		ReduceParenthesis( oTree.m_vChild[ i ] );
}