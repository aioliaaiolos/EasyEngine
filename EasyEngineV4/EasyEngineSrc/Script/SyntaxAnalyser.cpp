#include "SyntaxAnalyser.h"
#include "LexAnalyser.h"



CSyntaxNode::CSyntaxNode():
m_Type( eNone ),
m_nAddress( -1 ),
m_pUserData( NULL )
{
}

CSyntaxNode::CSyntaxNode( CLexAnalyser::CLexem l ):
m_Lexem( l ),
m_nAddress( -1 )
{
}

bool CSyntaxNode::IsValue(NODE_TYPE node)
{
	return (node == eVal) || (node == eInt) || (node == eFloat) || (node == eString);
}

void CSyntaxAnalyser::ReduceInstruction( CSyntaxNode& oTree )
{
	unsigned int i = 0;
	int iBeginInstr = 0;
	while( i < oTree.m_vChild.size() )
	{
		if (oTree.m_vChild[i].m_Lexem.m_eType == CLexAnalyser::CLexem::eFunctionDef) {
			int offset = i + 1;
			while (oTree.m_vChild[offset].m_Lexem.m_eType != CLexAnalyser::CLexem::eRPar)
				offset++;
			iBeginInstr = offset + 1;
			i = iBeginInstr - 1;
		}
		if( oTree.m_vChild[ i ].m_Lexem.m_eType == CLexAnalyser::CLexem::ePtVirg )
		{
			CSyntaxNode oInstr;
			oInstr.m_Type = CSyntaxNode::eInstr;
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


void CSyntaxAnalyser::GetSyntaxicTree( const vector< CLexAnalyser::CLexem >& vLexem, CSyntaxNode& oTree )
{
	vector< CSyntaxNode > vSyntax;
	for( unsigned int i = 0; i < vLexem.size(); i++ )
		vSyntax.push_back( CSyntaxNode( vLexem[ i ] ) );
	// On réduit toutes les valeurs
	for( unsigned int i = 0; i < vSyntax.size(); i++ )
	{
		if( vSyntax[ i ].m_Lexem.m_eType == CLexAnalyser::CLexem::eFloat ||  
			vSyntax[ i ].m_Lexem.m_eType == CLexAnalyser::CLexem::eInt ||
			vSyntax[ i ].m_Lexem.m_eType == CLexAnalyser::CLexem::eString ||
			vSyntax[ i ].m_Lexem.m_eType == CLexAnalyser::CLexem::eVar ||
			vSyntax[i].m_Lexem.m_eType == CLexAnalyser::CLexem::eCall)
		{
			vSyntax[ i ].m_Type = CSyntaxNode::eVal;
		}
	}
	oTree.m_Type = CSyntaxNode::eProg;
	for( unsigned int i = 0; i < vSyntax.size(); i++ )
		oTree.m_vChild.push_back( vSyntax[ i ] );
	ReduceScopes(oTree);
	ReduceInstruction( oTree );
	ReduceParenthesis( oTree );
	ReduceAllOperations( oTree );
	DeleteParNodes( oTree );
	ReduceVecArgs( oTree );
}

void CSyntaxAnalyser::DeleteParNodes( CSyntaxNode& oNode )
{
	if( oNode.m_Type == CSyntaxNode::ePar )
	{
		oNode.m_Type = oNode.m_vChild[ 0 ].m_Type;
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
		if( oNode.m_vChild[ i ].m_Type == CSyntaxNode::eVecArgs )
		{
			if( oNode.m_vChild[ i ].m_vChild.size() > 0 )
			{
				oNode.m_vChild.swap( oNode.m_vChild[ i ].m_vChild );
				for( unsigned int j = 0; j < oNode.m_vChild.size(); j++ )
					if( oNode.m_vChild[ j ].m_Lexem.m_eType == CLexAnalyser::CLexem::eVirg )
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
		if( oNode.m_vChild[ i ].m_Type == CSyntaxNode::ePar ) {
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
	vector< CLexAnalyser::CLexem::TLexem > vTypes;
	vTypes.push_back( CLexAnalyser::CLexem::eMult );
	vTypes.push_back( CLexAnalyser::CLexem::eDiv );
	ReduceOperations( oNode, vTypes  );
	vTypes.clear();
	vTypes.push_back( CLexAnalyser::CLexem::eAdd );
	vTypes.push_back( CLexAnalyser::CLexem::eSub );
	ReduceOperations( oNode, vTypes );
	vTypes.clear();
	vTypes.push_back( CLexAnalyser::CLexem::eAffect );
	ReduceOperations( oNode, vTypes );
}

void CSyntaxAnalyser::ReduceOperations( CSyntaxNode& oNode, const vector< CLexAnalyser::CLexem::TLexem >& vType )
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
				bool bLeftIsParOrVal = ( ( oNode.m_vChild[ i - 1 ].m_Type == CSyntaxNode::eVal ) || ( oNode.m_vChild[ i - 1 ].m_Type == CSyntaxNode::ePar ) );
				bool bRightIsParOrVal = ( ( oNode.m_vChild[ i + 1 ].m_Type == CSyntaxNode::eVal ) || (oNode.m_vChild[ i + 1 ].m_Type == CSyntaxNode::ePar ) );
				if( bLeftIsParOrVal && bRightIsParOrVal ) {
					oNode.m_vChild[ i ].m_vChild.push_back( oNode.m_vChild[ i - 1 ] );
					oNode.m_vChild[ i ].m_vChild.push_back( oNode.m_vChild[ i + 1 ] );
					oNode.m_vChild[ i ].m_Type = CSyntaxNode::eVal;
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


void GetClosingLexemIndices(const vector< CSyntaxNode >& vNode, CLexAnalyser::CLexem::TLexem LClosing, CLexAnalyser::CLexem::TLexem RClosing,
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

void CSyntaxAnalyser::ReduceLargestClosingLexem(vector<CSyntaxNode>& vNode, CLexAnalyser::CLexem::TLexem leftLexem, CLexAnalyser::CLexem::TLexem rightLexem, 
	CSyntaxNode::NODE_TYPE nt, TParenthesisReductionType rt, unsigned int iFirst)
{
	int iBegin, iEnd;
	GetClosingLexemIndices(vNode, leftLexem, rightLexem, iBegin, iEnd, iFirst);
	if (iBegin != -1 && iEnd != -1)	{
		CSyntaxNode oNode;
		oNode.m_Type = nt;
		for (int j = iBegin + 1; j < iEnd; j++)
			oNode.m_vChild.push_back(vNode[j]);
		vNode.erase(vNode.begin() + iBegin, vNode.begin() + iEnd + 1);
		if (rt == eNormal)
			vNode.insert(vNode.begin() + iBegin, oNode);
		else if(rt == eFunc)
			vNode[iBegin - 1].m_vChild.push_back(oNode);
		else if (rt == eFuncDef) {
			vector<CSyntaxNode>::iterator itNode = vNode.begin() + iBegin - 1;
			while (itNode->m_Lexem.m_sValue != "function")
				itNode--;
			itNode = vNode.erase(itNode);
			itNode->m_Lexem.m_eType = CLexAnalyser::CLexem::TLexem::eFunctionDef;
			itNode->m_vChild.push_back(oNode);
			itNode->m_Type = CSyntaxNode::NODE_TYPE::eFunctionDef;
			m_mFunctions.insert(itNode->m_Lexem.m_sValue);
			vector<string>::iterator itFuncName = std::find(m_vFunctions.begin(), m_vFunctions.end(), itNode->m_Lexem.m_sValue);
			if (itFuncName == m_vFunctions.end())
				m_vFunctions.push_back(itNode->m_Lexem.m_sValue);
		}
	}
}

void CSyntaxAnalyser::ReduceScopes(CSyntaxNode& oTree)
{
	for (unsigned int i = 0; i < oTree.m_vChild.size(); i++)
	{
		int iPred = i > 0 ? i - 1 : 0;
		if (oTree.m_vChild[i].m_Lexem.m_eType == CLexAnalyser::CLexem::eLBraket)
			ReduceLargestClosingLexem(oTree.m_vChild, CLexAnalyser::CLexem::TLexem::eLBraket, CLexAnalyser::CLexem::TLexem::eRBraket, CSyntaxNode::eScope, eFuncDef, i);
	}
	for (unsigned int i = 0; i < oTree.m_vChild.size(); i++)
		ReduceScopes(oTree.m_vChild[i]);
}

void CSyntaxAnalyser::ReduceParenthesis( CSyntaxNode& oTree )
{
	for( unsigned int i = 0; i < oTree.m_vChild.size(); i++ )
	{
		int iPred = i > 0 ? i - 1 : 0;
		if( oTree.m_vChild[ i ].m_Lexem.m_eType == CLexAnalyser::CLexem::eLPar ) {
			if( oTree.m_vChild[ iPred ].m_Lexem.m_eType == CLexAnalyser::CLexem::eCall ||
				oTree.m_vChild[iPred].m_Lexem.m_eType == CLexAnalyser::CLexem::eFunctionDef)
				ReduceLargestClosingLexem(oTree.m_vChild, CLexAnalyser::CLexem::TLexem::eLPar, CLexAnalyser::CLexem::TLexem::eRPar, CSyntaxNode::eVecArgs, eFunc, i);
			else
				ReduceLargestClosingLexem(oTree.m_vChild, CLexAnalyser::CLexem::TLexem::eLPar, CLexAnalyser::CLexem::TLexem::eRPar, CSyntaxNode::ePar, eNormal, i);
		}
	}
	for( unsigned int i = 0; i < oTree.m_vChild.size(); i++ )	
		ReduceParenthesis( oTree.m_vChild[ i ] );
}