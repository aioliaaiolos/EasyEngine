#include "AsmGenerator.h"
#include "LexAnalyser.h"
#include "SyntaxAnalyser.h"
#include "ScriptException.h"
#include "Exception.h"

#include <sstream>
#include <algorithm>

map< CRegister::TType, string > CRegister::s_mRegisterToString;

void CRegister::InitRegisterToStringMap()
{
	s_mRegisterToString[ eax ] = "eax";
	s_mRegisterToString[ ebx ] = "ebx";
	s_mRegisterToString[ ecx ] = "ecx";
	s_mRegisterToString[ edx ] = "edx";
	s_mRegisterToString[ esi ] = "esi";
	s_mRegisterToString[ edi ] = "edi";
	s_mRegisterToString[ ebp ] = "ebp";
	s_mRegisterToString[ esp ] = "esp";
}

void CRegister::GetStringName( string& sName, bool withQuotes) const
{ 
	sName = s_mRegisterToString.at(m_eValue); 
}

void CPreprocessorString::GetStringName( string& sName, bool withQuotes) const
{
	if(withQuotes)
		sName = string( "\"" ) + m_sValue + "\", 0";
	else
		sName = m_sValue;
}

void CNumeric::GetStringName( string& sName, bool withQuotes) const
{
	ostringstream oss;
	oss << m_fValue;
	sName = oss.str();
}

void CMemory::GetStringName( string& sName, bool withQuotes) const 
{
	string sBase, sIndex;
	if( m_oBase.m_eValue != CRegister::eNone )
		m_oBase.GetStringName( sBase );
	if( m_oIndex.m_eValue != CRegister::eNone )
		m_oIndex.GetStringName( sIndex );

	ostringstream oss;
	oss << "[ ";
	if( sBase.size() > 0 )
		oss << sBase;
	if( sIndex.size() > 0 )
	{
		if( sBase.size() > 0 )
			oss << " + " << sIndex;
		else
			oss << sIndex;
	}
		
	if( m_nDisplacement != 0 )
	{
		if( sBase.size() > 0 || sIndex.size() > 0 )
		{
			if( m_nDisplacement > 0 )
				oss << " + " << m_nDisplacement;
			else
				oss << " - " << -m_nDisplacement;
		}
		else
			oss << m_nDisplacement;
	}
	oss << " ]";
	sName = oss.str();
}

CAsmGenerator::CAsmGenerator():
	m_nCurrentScopeNumber( 0 ),
	m_bEaxBusy(false),
	m_nLastLabelIndex(-1),
	m_bMainRetPlaced(false)
{
	m_mTypeToMnemonic[ CLexem::eAdd ] = eAdd;
	m_mTypeToMnemonic[ CLexem::eSub ] = eSub;
	m_mTypeToMnemonic[ CLexem::eMult ] = eMul;
	m_mTypeToMnemonic[ CLexem::eDiv ] = eDiv;

	m_mMnemonicToString[eNone] = "";
	m_mMnemonicToString[ eMov ] = "mov";
	m_mMnemonicToString[ eAdd ] = "add";
	m_mMnemonicToString[ eMul ] = "mul";
	m_mMnemonicToString[ eSub ] = "sub";
	m_mMnemonicToString[ eDiv ] = "div";
	m_mMnemonicToString[ eCall ] = "call";
	m_mMnemonicToString[ ePush ] = "push";
	m_mMnemonicToString[ ePop ] = "pop";
	m_mMnemonicToString[ eInt ] = "int";
	m_mMnemonicToString[ eDB ] = "db";
	m_mMnemonicToString[ eRet ] = "ret";
	m_mMnemonicToString[eReturn] = "return";
	m_mMnemonicToString[eCmp] = "cmp";
	m_mMnemonicToString[eJe] = "je";
	m_mMnemonicToString[eJne] = "jne";
	CRegister::InitRegisterToStringMap();
}

void CAsmGenerator::CreateAssemblerListing(vector< CInstr >& vCodeOut, string sFileName)
{
	FILE* pFile = NULL;
	fopen_s(&pFile, sFileName.c_str(), "w");
	ostringstream oss;
	for (unsigned int i = 0; i < vCodeOut.size(); i++)
	{
		map< TMnemonic, string >::iterator itMnemonic = m_mMnemonicToString.find(vCodeOut[i].m_eMnem);
		if (itMnemonic != m_mMnemonicToString.end()) {
			if (itMnemonic->first != eNone)
				oss << m_mMnemonicToString[vCodeOut[i].m_eMnem] << " ";
			for (unsigned int j = 0; j < vCodeOut[i].m_vOperand.size(); j++)
			{
				bool putQuotes = (itMnemonic->first != eNone) && (itMnemonic->first != eJne) && (itMnemonic->first != eJe) && (itMnemonic->first != eCall);
				
				if (itMnemonic->first == eNone)
					oss << "\n";
				string s;
				vCodeOut[i].m_vOperand[j]->GetStringName(s, putQuotes);
				oss << s;
				if (j < vCodeOut[i].m_vOperand.size() - 1)
					oss << ", ";
				else
					oss << "\n";
			}
			if (vCodeOut[i].m_vOperand.size() == 0)
				oss << "\n";
		}
		else {
			CCompilationErrorException e(-1, -1);
			throw e;
		}
	}
	fwrite(oss.str().c_str(), sizeof(char), oss.str().size(), pFile);
	fclose(pFile);
}

void CAsmGenerator::FillOperandFromSyntaxNode( CNumeric* pOperand, const CSyntaxNode& oTree )
{
	switch (oTree.m_Lexem.m_eType)
	{
	case CLexem::eFloat:
		pOperand->m_fValue = oTree.m_Lexem.m_fValue;
		break;
	case CLexem::eInt:
		pOperand->m_fValue = oTree.m_Lexem.m_nValue;
		break;
	case CLexem::eString:
		pOperand->m_fValue = oTree.m_nAddress;
		break;
	}
}

void CAsmGenerator::CreateStackFrame(vector< CInstr >& vCodeOut)
{
	CInstr oIntr;
	// create stack frame
	oIntr.m_eMnem = eMov;
	CRegister* pEbp = new CRegister(CRegister::ebp);
	CRegister* pEsp = new CRegister(CRegister::esp);
	oIntr.m_vOperand.push_back(pEbp);
	oIntr.m_vOperand.push_back(pEsp);
	vCodeOut.push_back(oIntr);
}

string CAsmGenerator::GenerateNewLabel()
{
	return string("Label") + std::to_string(++m_nLastLabelIndex);
}

void CAsmGenerator::Enter(vector<CInstr>& vAssembler)
{
	GenPush(CRegister::ebp, vAssembler);
	GenMovRegReg(CRegister::ebp, CRegister::esp, vAssembler);
}

void CAsmGenerator::Leave(vector<CInstr>& vAssembler)
{
	GenMovRegReg(CRegister::esp, CRegister::ebp, vAssembler);
	GenPop(CRegister::ebp, vAssembler);
}

void CAsmGenerator::GenAssembler( const CSyntaxNode& oTree, vector< CInstr >& vCodeOut, const map<string, int>& mFuncAddr, VarMap& mVar )
{
	m_bMainRetPlaced = false;
	CInstr oIntr;
	// generate asm
	GenAssemblerFirstPass( oTree, vCodeOut, mFuncAddr, mVar );

	ResolveAddresses( vCodeOut );
}

int CAsmGenerator::GetVarCountFromScope(const CSyntaxNode& oTree, VarMap& mVar, int nScope)
{
	int nVarCountInThisScope = 0;
	VarMap::iterator itScope = mVar.find(nScope);
	if (itScope != mVar.end()) {
		for (const pair<string, CVar>& pairNameVar : itScope->second) {
			if (oTree.FindVar(pairNameVar.first)) {
				nVarCountInThisScope++;
			}
		}
	}
	return nVarCountInThisScope;
}

void CAsmGenerator::GenAssemblerFirstPass( const CSyntaxNode& oTree, vector< CInstr >& vAssembler, const map<string, int>& mFuncAddr, VarMap& mVar )
{
	int nVarCountInThisScope = 0;
	if ( oTree.m_eType == CSyntaxNode::eScope)
		m_nCurrentScopeNumber++;
	if (oTree.m_eType == CSyntaxNode::eProg || oTree.m_eType == CSyntaxNode::eScope) {
		nVarCountInThisScope = GetVarCountFromScope(oTree, mVar, m_nCurrentScopeNumber);
		if (nVarCountInThisScope > 0)
			GenSubRegImm(CRegister::esp, nVarCountInThisScope, vAssembler);
	}

	if( oTree.m_Lexem.IsOperation() )
	{
		if(oTree.m_vChild.size() > 0 && oTree.m_vChild[ 0 ].m_vChild.size() == 0 && oTree.m_vChild[ 1 ].m_vChild.size() == 0 )
			GenOperation( oTree.m_Lexem.m_eType, oTree.m_vChild[ 0 ], oTree.m_vChild[ 1 ], vAssembler );
		else 
		{
			if (oTree.m_vChild.size() < 2)
				throw CCompilationErrorException(-1, -1);
			if( oTree.m_vChild[ 0 ].m_vChild.size() > 0 )
			{
				GenAssemblerFirstPass(oTree.m_vChild[ 0 ], vAssembler, mFuncAddr, mVar );
				m_bEaxBusy = true;
			}
			if( oTree.m_vChild[ 1 ].m_vChild.size() == 0 )
			{
				if( !m_bEaxBusy)
					GenMov( CRegister::eax, oTree.m_vChild[ 0 ], vAssembler );
				GenOperation( oTree.m_Lexem.m_eType, CRegister::eax, oTree.m_vChild[ 1 ], vAssembler );
			}
			else
			{
				if(m_bEaxBusy)
				{
					GenPush( CRegister::eax, vAssembler );
					GenAssemblerFirstPass( oTree.m_vChild[ 1 ], vAssembler, mFuncAddr, mVar );
					GenMov( CRegister::ebx, CRegister::eax, vAssembler );
					GenPop( CRegister::eax, vAssembler );
					GenOperation( oTree.m_Lexem.m_eType, CRegister::eax, CRegister::ebx, vAssembler );
				}
				else
				{
					GenAssemblerFirstPass(oTree.m_vChild[ 1 ], vAssembler, mFuncAddr, mVar );
					GenMov( CRegister::ebx, CRegister::eax, vAssembler );
					GenMov( CRegister::eax, oTree.m_vChild[ 0 ], vAssembler );
					GenOperation( oTree.m_Lexem.m_eType, CRegister::eax, CRegister::ebx, vAssembler );
				}
			}
		}
	}
	else if( oTree.m_Lexem.m_eType == CLexem::eCall )
	{
		for( unsigned int i = 0; i < oTree.m_vChild.size(); i++ )
		{
			int nIndex = (int)oTree.m_vChild.size() - i - 1;
			if( oTree.m_vChild[ nIndex ].m_Lexem.IsNumeric() )
				GenPush( oTree.m_vChild[ nIndex ], vAssembler );
			else if (oTree.m_vChild[nIndex].m_Lexem.m_eType == CLexem::eVar) {
				CVar& v = mVar[m_nCurrentScopeNumber][oTree.m_vChild[nIndex].m_Lexem.m_sValue];
				CMemory* pMemory = CreateMemoryRegister(CRegister::ebp, CRegister::eNone, -4 * v.m_nRelativeStackPosition);
				GenPush(pMemory, vAssembler);
			} 
			else {
				GenAssemblerFirstPass( oTree.m_vChild[ nIndex ], vAssembler, mFuncAddr, mVar );
				GenPush( CRegister::eax, vAssembler );
			}
		}
		GenCall( oTree, vAssembler );
	}
	else if( oTree.m_Lexem.m_eType == CLexem::eAffect )
	{
		if (oTree.m_vChild.empty()) {
			CCompilationErrorException e(-1, -1);
			throw e;
		}
		CVar& destVar = mVar[ m_nCurrentScopeNumber ][ oTree.m_vChild[ 0 ].m_Lexem.m_sValue ];
		if( !destVar.m_bIsDeclared )
			destVar.m_bIsDeclared = true;
		
		const CSyntaxNode& srcNode = oTree.m_vChild[1];
		CMemory* pDestMemory = CreateMemoryRegister(CRegister::ebp, CRegister::TType::eNone, -4 * destVar.m_nRelativeStackPosition);
		if (oTree.m_vChild[1].m_Lexem.m_eType == CLexem::eVar) {
			CVar& v2 = mVar[m_nCurrentScopeNumber][oTree.m_vChild[1].m_Lexem.m_sValue];
			CMemory* pSrcMemory = CreateMemoryRegister(CRegister::ebp, CRegister::eNone, -4 * v2.m_nRelativeStackPosition);
			GenMovAddrAddr(pSrcMemory, pDestMemory, vAssembler);
		}
		else if (oTree.m_vChild[1].m_eType == CSyntaxNode::eInt || oTree.m_vChild[1].m_eType == CSyntaxNode::eFloat || oTree.m_vChild[1].m_eType == CSyntaxNode::eString) {
			if (oTree.m_vChild[1].m_vChild.size() > 0) {
				GenAssemblerFirstPass(oTree.m_vChild[1], vAssembler, mFuncAddr, mVar);
				GenMovAddrReg(CRegister::eax, pDestMemory, vAssembler);
			}
			else
				GenMovAddrImm(pDestMemory, srcNode, vAssembler);
		}
		else if (oTree.m_vChild[1].m_Lexem.m_eType == CLexem::eCall) {
			GenAssemblerFirstPass(oTree.m_vChild[1], vAssembler, mFuncAddr, mVar);
			GenMovAddrReg(CRegister::eax, pDestMemory, vAssembler);
		}
	}
	else if (oTree.m_eType == CSyntaxNode::eCommand) {
		GenReturn(vAssembler);
	}
	else if (oTree.m_eType == CSyntaxNode::eFunctionDef) {
		if (!m_bMainRetPlaced) {
			GenRet(vAssembler);
			m_bMainRetPlaced = true;
		}
		PutLabel(oTree.m_Lexem.m_sValue, vAssembler);
		Enter(vAssembler);
		for (unsigned int i = 0; i < oTree.m_vChild.size(); i++)
			GenAssemblerFirstPass(oTree.m_vChild[i], vAssembler, mFuncAddr, mVar);
		Leave(vAssembler);
		GenRet(vAssembler);
	}
	else if (oTree.m_Lexem.m_eType == CLexem::eIf) {
		GenAssemblerFirstPass(oTree.m_vChild[0], vAssembler, mFuncAddr, mVar);
		string sLabel = GenerateNewLabel();
		GenJne(sLabel, vAssembler);
		GenAssemblerFirstPass(oTree.m_vChild[1], vAssembler, mFuncAddr, mVar);
		PutLabel(sLabel, vAssembler);
	}
	else if (oTree.m_Lexem.m_eType == CLexem::TLexem::eComp) {
		GenCmp(oTree.m_vChild[0], oTree.m_vChild[1], vAssembler, mFuncAddr, mVar);
	}
	else
	{
		for (int i = 0; i < oTree.m_vChild.size(); i++) {
			GenAssemblerFirstPass(oTree.m_vChild[i], vAssembler, mFuncAddr, mVar);
			int n = (int)oTree.m_vChild.size() - 2;
			if (i < n) {
				if ((oTree.m_eType == CSyntaxNode::eProg) && oTree.m_vChild[i].m_eType == CSyntaxNode::eInstr && oTree.m_vChild[i + 1].m_eType == CSyntaxNode::eFunctionDef) {
					nVarCountInThisScope = GetVarCountFromScope(oTree, mVar, 0);
					if (nVarCountInThisScope > 0) {
						GenAddRegImm(CRegister::esp, nVarCountInThisScope, vAssembler);
						GenMovRegReg(CRegister::esp, CRegister::eax, vAssembler);
					}
				}
			}
		}
	}

	if (oTree.m_eType == CSyntaxNode::eScope) {
		m_nCurrentScopeNumber--;
	}
}

void CAsmGenerator::ResolveAddresses( vector< CInstr >& vCodeOut )
{
#if STRING_IN_BIN
	map< string, vector< pair< int, int > > >::iterator itString = m_mStringInstr.begin();
	for ( ; itString != m_mStringInstr.end(); ++itString )
	{
		for( unsigned int i = 0; i < itString->second.size(); i++ )
		{
			pair< int, int >& p = itString->second[ i ];
			CNumeric* pStringAddress = static_cast< CNumeric* >( vCodeOut[ p.first ].m_vOperand[ p.second ] );
			pStringAddress->m_fValue = vCodeOut.size();
		}
		CInstr oInstr;
		oInstr.m_eMnem = eDB;
		CPreprocessorString* pString = new CPreprocessorString( itString->first );
		oInstr.m_vOperand.push_back( pString );
		vCodeOut.push_back( oInstr );
	}
#endif // 0
}

void CAsmGenerator::GenCall( const CSyntaxNode& oNode, vector< CInstr >& vAssembler )
{
	CInstr oInstr;
	if (oNode.m_eType == CSyntaxNode::eAPICall) {
		oInstr.m_eMnem = CAsmGenerator::eInt;
		CNumeric* pNumeric = new CNumeric(oNode.m_nAddress);
		oInstr.m_vOperand.push_back(pNumeric);
	}
	else {
		oInstr.m_eMnem = CAsmGenerator::eCall;
		CPreprocessorString* pFunctionName = new CPreprocessorString(oNode.m_Lexem.m_sValue);
		oInstr.m_vOperand.push_back(pFunctionName);
	}
	
	vAssembler.push_back( oInstr );
}

void CAsmGenerator::GenJe(string label, vector< CInstr >& vAssembler)
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::eJe;
	CPreprocessorString* pLabel = new CPreprocessorString(label);
	oInstr.m_vOperand.push_back(pLabel);
	vAssembler.push_back(oInstr);
}

void CAsmGenerator::GenJne(string label, vector< CInstr >& vAssembler)
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::eJne;
	CPreprocessorString* pLabel = new CPreprocessorString(label);
	oInstr.m_vOperand.push_back(pLabel);
	vAssembler.push_back(oInstr);
}

void CAsmGenerator::GenCmp(CRegister::TType reg1, CRegister::TType reg2, vector< CInstr >& vAssembler)
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::eCmp;
	CRegister* r1 = new CRegister(reg1);
	CRegister* r2 = new CRegister(reg2);
	oInstr.m_vOperand.push_back(r1);
	oInstr.m_vOperand.push_back(r2);
	vAssembler.push_back(oInstr);
}

void CAsmGenerator::GenCmp(CRegister::TType reg, float val, vector< CInstr >& vAssembler)
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::eCmp;
	CRegister* r = new CRegister(reg);
	oInstr.m_vOperand.push_back(r);
	CNumeric* pValue = new CNumeric(val);
	oInstr.m_vOperand.push_back(pValue);
	vAssembler.push_back(oInstr);
}

void CAsmGenerator::GenCmp(CMemory* pMemory, float val, vector< CInstr >& vAssembler)
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::eCmp;
	oInstr.m_vOperand.push_back(pMemory);
	CNumeric* pValue = new CNumeric(val);
	oInstr.m_vOperand.push_back(pValue);
	vAssembler.push_back(oInstr);
}

void CAsmGenerator::GenCmp(CMemory* pMemory1, CMemory* pMemory2, vector< CInstr >& vAssembler)
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::eCmp;
	oInstr.m_vOperand.push_back(pMemory1);
	oInstr.m_vOperand.push_back(pMemory2);
	vAssembler.push_back(oInstr);
}

void CAsmGenerator::GenCmp(float val1, float val2, vector< CInstr >& vAssembler)
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::eCmp;
	CNumeric* pValue1 = new CNumeric(val1);
	oInstr.m_vOperand.push_back(pValue1);
	CNumeric* pValue2 = new CNumeric(val2);
	oInstr.m_vOperand.push_back(pValue2);
	vAssembler.push_back(oInstr);
}

void CAsmGenerator::GenCmp(const CSyntaxNode& node1, const CSyntaxNode& node2, vector< CInstr >& vAssembler, const map<string, int>& mFuncAddr, VarMap& mVar)
{
	if (node1.m_vChild.size() > 0) {
		if ((node2.m_vChild.size() == 0)) {
			GenAssemblerFirstPass(node1, vAssembler, mFuncAddr, mVar);
			if (node2.m_Lexem.m_eType == CLexem::TLexem::eInt) {
				GenCmp(CRegister::eax, node2.m_Lexem.m_nValue, vAssembler);
			}
			else if (node2.m_Lexem.m_eType == CLexem::TLexem::eFloat) {
				GenCmp(CRegister::eax, node2.m_Lexem.m_fValue, vAssembler);
			}
		}
		CCompilationErrorException e(-1, -1);
		e.SetErrorMessage("Erreur : cas non encore géré par le compilateur");
	}
	else if (node2.m_vChild.size() > 0) {
		CCompilationErrorException e(-1, -1);
		e.SetErrorMessage("Erreur : cas non encore géré par le compilateur");
	}
	else {
		float val;
		map< string, CVar >::iterator itNode = mVar[m_nCurrentScopeNumber].find(node1.m_Lexem.m_sValue);
		if (itNode != mVar[m_nCurrentScopeNumber].end()) {
			CMemory* pMemory = CreateMemoryRegister(CRegister::ebp, CRegister::TType::eNone, -4 * itNode->second.m_nRelativeStackPosition);
			itNode = mVar[m_nCurrentScopeNumber].find(node2.m_Lexem.m_sValue);
			if (itNode != mVar[m_nCurrentScopeNumber].end()) {
				CMemory* pMemory2 = CreateMemoryRegister(CRegister::ebp, CRegister::TType::eNone, -4 * itNode->second.m_nRelativeStackPosition);
				GenCmp(pMemory, pMemory2, vAssembler);
			}
			else if (node2.m_Lexem.m_eType == CLexem::eInt) {
				GenCmp(pMemory, node2.m_Lexem.m_nValue, vAssembler);
			}
			else if (node2.m_Lexem.m_eType == CLexem::eFloat) {
				GenCmp(pMemory, node2.m_Lexem.m_fValue, vAssembler);
			}
		}
		else {
			itNode = mVar[m_nCurrentScopeNumber].find(node2.m_Lexem.m_sValue);
			if (itNode != mVar[m_nCurrentScopeNumber].end()) {
				CMemory* pMemory = CreateMemoryRegister(CRegister::ebp, CRegister::TType::eNone, -4 * itNode->second.m_nRelativeStackPosition);
				if (node1.m_Lexem.m_eType == CLexem::eInt) {
					GenCmp(pMemory, node1.m_Lexem.m_nValue, vAssembler);
				}
				else if (node1.m_Lexem.m_eType == CLexem::eFloat) {
					GenCmp(pMemory, node1.m_Lexem.m_fValue, vAssembler);
				}
			}
			else if (node1.m_Lexem.m_eType == CLexem::eInt) {
				if (node1.m_Lexem.m_eType == CLexem::eInt)
					GenCmp(node1.m_Lexem.m_nValue, node2.m_Lexem.m_nValue, vAssembler);
				else if (node2.m_Lexem.m_eType == CLexem::eFloat)
					GenCmp(node1.m_Lexem.m_nValue, node2.m_Lexem.m_fValue, vAssembler);
			}
			else if (node1.m_Lexem.m_eType == CLexem::eFloat) {
				if (node2.m_Lexem.m_eType == CLexem::eInt)
					GenCmp(node1.m_Lexem.m_fValue, node2.m_Lexem.m_nValue, vAssembler);
				else if (node2.m_Lexem.m_eType == CLexem::eFloat)
					GenCmp(node1.m_Lexem.m_fValue, node2.m_Lexem.m_fValue, vAssembler);
			}
			else if (node1.m_Lexem.m_eType == CLexem::eCall) {
				GenAssemblerFirstPass(node1, vAssembler, mFuncAddr, mVar);
				if (node2.m_Lexem.m_eType == CLexem::eInt)
					GenCmp(CRegister::eax, node2.m_Lexem.m_nValue, vAssembler);
				else if (node2.m_Lexem.m_eType == CLexem::eFloat)
					GenCmp(CRegister::eax, node2.m_Lexem.m_fValue, vAssembler);
			}
			else {
				CCompilationErrorException e(-1, -1);
				e.SetErrorMessage("Erreur : cas non encore géré par le compilateur");
			}
		}

	}
	/*
	GenPush(CRegister::eax, vAssembler);
	if (oTree.m_vChild[0].m_vChild.size() > 0)
	GenAssemblerFirstPass(oTree.m_vChild[1], vAssembler, mFuncAddr, mVar);
	else {

	}
	GenPop(CRegister::ebx, vAssembler);
	GenCmp(CRegister::ebx, CRegister::eax, vAssembler);*/
}

void CAsmGenerator::PutLabel(string sLabel, vector< CInstr >& vAssembler)
{
	CInstr oInstr;
	CPreprocessorString* pLabel = new CPreprocessorString(sLabel + ":");
	oInstr.m_vOperand.push_back(pLabel);
	vAssembler.push_back(oInstr);
}

void CAsmGenerator::GenRet(vector< CInstr >& vAssembler)
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::eRet;
	vAssembler.push_back(oInstr);
}

void CAsmGenerator::GenReturn(vector< CInstr >& vAssembler)
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::eReturn;
	vAssembler.push_back(oInstr);
}

void CAsmGenerator::GenPop( CRegister::TType reg, vector< CInstr >& vAssembler )
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::ePop;
	CRegister* pRegister = new CRegister( reg );
	oInstr.m_vOperand.push_back( pRegister );
	vAssembler.push_back( oInstr );
}


void CAsmGenerator::GenMov( CRegister::TType reg, const CSyntaxNode& oNode, vector< CInstr >& vAssembler )
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::eMov;
	CRegister* pReg = new CRegister( reg );
	CNumeric* pNum = new CNumeric;
	FillOperandFromSyntaxNode( pNum, oNode );
	oInstr.m_vOperand.push_back( pReg );
	oInstr.m_vOperand.push_back( pNum );
	vAssembler.push_back( oInstr );
	if( oNode.m_Lexem.m_eType == CLexem::eString )
		m_mStringInstr[ oNode.m_Lexem.m_sValue ].push_back( pair< int, int >::pair( (int)vAssembler.size() - 1, 1 ) );
}

void CAsmGenerator::GenMov( CRegister::TType a, CRegister::TType b, vector< CInstr >& vAssembler )
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::eMov;
	CRegister* pRegister1 = new CRegister( a );
	CRegister* pRegister2 = new CRegister( b );
	oInstr.m_vOperand.push_back( pRegister1 );
	oInstr.m_vOperand.push_back( pRegister2 );
	vAssembler.push_back( oInstr );
}

CMemory* CAsmGenerator::CreateMemoryRegister(CRegister::TType eBase, CRegister::TType eIndex, int nDisplacement)
{
	CMemory* pMemory = new CMemory;
	pMemory->m_oBase = eBase;
	pMemory->m_oIndex = eIndex;
	pMemory->m_nDisplacement = nDisplacement;
	return pMemory;
}

void CAsmGenerator::GenMovAddrImm(CMemory* pMemory, const CSyntaxNode& oImm, vector< CInstr >& vAssembler )
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::eMov;
	
	CNumeric* pNum = new CNumeric;
	FillOperandFromSyntaxNode( pNum, oImm );

	oInstr.m_vOperand.push_back(pMemory);
	oInstr.m_vOperand.push_back( pNum );
	vAssembler.push_back( oInstr );
}

void CAsmGenerator::GenMovAddrReg(CRegister::TType scrReg, CMemory* pDestMemory, vector< CInstr >& vAssembler)
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::eMov;
	oInstr.m_vOperand.push_back(pDestMemory);
	oInstr.m_vOperand.push_back(new CRegister(scrReg));
	vAssembler.push_back(oInstr);
}

void CAsmGenerator::GenMovRegReg(CRegister::TType destReg, CRegister::TType srcReg, vector< CInstr >& vAssembler)
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::eMov;
	oInstr.m_vOperand.push_back(new CRegister(destReg));
	oInstr.m_vOperand.push_back(new CRegister(srcReg));
	vAssembler.push_back(oInstr);
}

void CAsmGenerator::GenMovRegAddr(CRegister::TType destReg, CMemory* pSrcMemory, vector< CInstr >& vAssembler)
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::eMov;
	oInstr.m_vOperand.push_back(new CRegister(destReg));
	oInstr.m_vOperand.push_back(pSrcMemory);
	vAssembler.push_back(oInstr);
}

void CAsmGenerator::GenMovAddrAddr(CMemory* pSrcMemory, CMemory* pDestMemory, vector< CInstr >& vAssembler)
{
	if (m_bEaxBusy)
		GenPush(CRegister::eax, vAssembler);
	GenMovRegAddr(CRegister::eax, pSrcMemory, vAssembler);
	GenMovAddrReg(CRegister::eax, pDestMemory, vAssembler);
	if (m_bEaxBusy)
		GenPop(CRegister::eax, vAssembler);
}

void CAsmGenerator::GenAddRegImm(CRegister::TType destReg, int val, vector< CInstr >& vAssembler)
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::eAdd;
	CRegister* pRegister = new CRegister(destReg);
	CNumeric* pNumeric = new CNumeric(val);
	oInstr.m_vOperand.push_back(pRegister);
	oInstr.m_vOperand.push_back(pNumeric);
	vAssembler.push_back(oInstr);
}

void CAsmGenerator::GenSubRegImm(CRegister::TType destReg, int val, vector< CInstr >& vAssembler)
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::eSub;
	CRegister* pRegister = new CRegister(destReg);
	CNumeric* pNumeric = new CNumeric(val);
	oInstr.m_vOperand.push_back(pRegister);
	oInstr.m_vOperand.push_back(pNumeric);
	vAssembler.push_back(oInstr);
}

void CAsmGenerator::GenPush( const CSyntaxNode& oNode, vector< CInstr >& vAssembler )
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::ePush;
	CNumeric* pNumeric = new CNumeric;
	FillOperandFromSyntaxNode( pNumeric, oNode );
	oInstr.m_vOperand.push_back( pNumeric );
	vAssembler.push_back( oInstr );
	if( oNode.m_Lexem.m_eType == CLexem::eString )
	{
		m_mStringInstr[ oNode.m_Lexem.m_sValue ].push_back( pair< int, int >::pair( (int)vAssembler.size() - 1, 0 ) );
	}
}

void CAsmGenerator::GenPush( CRegister::TType reg, vector< CInstr >& vAssembler )
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::ePush;
	CRegister* pRegister = new CRegister( reg );
	oInstr.m_vOperand.push_back( pRegister );
	vAssembler.push_back( oInstr );
}

void CAsmGenerator::GenPush( float val, vector< CInstr >& vAssembler )
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::ePush;
	CNumeric* pNumeric = new CNumeric( val );
	oInstr.m_vOperand.push_back( pNumeric );
	vAssembler.push_back( oInstr );
}

void CAsmGenerator::GenPush(CMemory* pMemory, vector< CInstr >& vAssembler)
{
	CInstr oInstr;
	oInstr.m_eMnem = CAsmGenerator::ePush;
	oInstr.m_vOperand.push_back(pMemory);
	vAssembler.push_back(oInstr);
}

void CAsmGenerator::GenOperation( CLexem::TLexem optype, CRegister::TType reg1, CRegister::TType reg2, vector< CInstr >& vAssembler )
{
	CInstr oInstr;
	CRegister* pReg1 = new CRegister( reg1 );
	CRegister* pReg2 = new CRegister( reg2 );
	oInstr.m_vOperand.push_back( pReg1 );
	oInstr.m_vOperand.push_back( pReg2 );
	oInstr.m_eMnem = m_mTypeToMnemonic[ optype ];
	vAssembler.push_back( oInstr );
}

void CAsmGenerator::GenOperation( CLexem::TLexem optype, const CSyntaxNode& child1, const CSyntaxNode& child2, vector< CInstr >& vAssembler )
{
	CInstr oInstr;
	GenMov( CRegister::eax, child1, vAssembler );
	CRegister* pOp1 = new CRegister( CRegister::eax );
	CNumeric* pOp2 = new CNumeric();
	FillOperandFromSyntaxNode( pOp2, child2 );
	oInstr.m_vOperand.push_back( pOp1 );
	oInstr.m_vOperand.push_back( pOp2 );
	oInstr.m_eMnem = m_mTypeToMnemonic[ optype ];
	vAssembler.push_back( oInstr );
}

void CAsmGenerator::GenOperation( CLexem::TLexem optype, CRegister::TType reg, const CSyntaxNode& oNode, vector< CInstr >& vAssembler )
{
	CInstr oInstr;
	oInstr.m_eMnem = m_mTypeToMnemonic[ optype ];
	CRegister* pRegister = new CRegister( reg );
	CNumeric* pNumeric = new CNumeric;
	FillOperandFromSyntaxNode( pNumeric, oNode );
	oInstr.m_vOperand.push_back( pRegister );
	oInstr.m_vOperand.push_back( pNumeric );
	vAssembler.push_back( oInstr );
}
