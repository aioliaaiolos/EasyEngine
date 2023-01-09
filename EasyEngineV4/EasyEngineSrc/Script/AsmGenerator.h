#ifndef CODEGEN_H
#define CODEGEN_H

#include <vector>
#include "LexAnalyser.h"
#include "SemanticAnalyser.h"

using namespace std;

class CSyntaxNode;

class IOperand
{
public:
	enum TOperandType
	{
		eNone = -1,
		eRegister,
		eNumeric,
		eMemory
	};


	virtual void			GetStringName( string& sName, bool withQuotes = true) const = 0;
	virtual TOperandType	GetType()const = 0;

	
};

class CRegister : public IOperand
{
public:
	enum TType
	{
		eNone = -1,
		eax = 0,
		ebx,
		ecx,
		edx,
		esi,
		edi,
		ebp,
		esp
	};
	
	TType	m_eValue;
	CRegister() : m_eValue( eNone ){}
	CRegister( TType value ) : m_eValue( value ){}

	void			GetStringName( string& sName, bool withQuotes = true ) const;
	TOperandType	GetType()const{ return eRegister;  }
	static void	InitRegisterToStringMap();

private:
	 static map< TType, string >	s_mRegisterToString;	 
};

struct CNumeric : public IOperand
{
	double m_fValue;
	CNumeric() : m_fValue( 0.f ){}
	CNumeric( double value ) : m_fValue( value ){}
	void			GetStringName( string& sName, bool withQuotes = true) const;
	TOperandType	GetType()const{ return eNumeric; }
};

struct CMemory : public IOperand
{
	CRegister	m_oBase;
	CRegister	m_oIndex;
	int			m_nDisplacement;
	TOperandType GetType() const { return eMemory; }
	void		GetStringName( string& sName, bool withQuotes = true ) const;
};

struct CPreprocessorString : public IOperand
{
	string	m_sValue;
	CPreprocessorString( string sValue ) : m_sValue( sValue ){}
	void			GetStringName( string& sName, bool withQuotes = true) const;
	TOperandType	GetType()const{ return eNone;}
};

class CAsmGenerator
{
public:

	enum TMnemonic
	{
		eNone = -1,
		eMov = 0,
		eAdd,
		eMul,
		eSub,
		eDiv,
		eCall,
		eInt,
		ePush,
		ePop,
		eRet,
		eReturn,
		eDB,
		eCmp,
		eJe,
		eJne,
		eJae,
		eJbe,
		eMnemonicCount
	};

	struct CInstr
	{
		TMnemonic					m_eMnem;
		vector < IOperand* >		m_vOperand;
		CInstr() : m_eMnem(eNone) {}
	};

	CAsmGenerator(bool bPutAllCodeIntoSameMemory);
	void CreateAssemblerListing( const vector< CInstr >& vCodeOut, string sFileName ) const;
	void GenAssembler( const CSyntaxNode& oTree, vector< CInstr >& vCodeOut, const map<string, int>& mFuncAddr, CVarMap& mVar );
	const vector<vector<CInstr>>& GetFunctions() const;

private:
	void											GenOperation( CLexem::Type, const CSyntaxNode& child1, const CSyntaxNode& child2, vector< CInstr >& vAssembler );
	void											GenOperation( CLexem::Type, CRegister::TType, const CSyntaxNode&, vector< CInstr >& );
	void											GenOperation( CLexem::Type, CRegister::TType reg1, CRegister::TType reg2, vector< CInstr >& );
	void											GenMov( CRegister::TType, const CSyntaxNode&, vector< CInstr >& );
	void											GenPush( CRegister::TType, vector< CInstr >& );
	void											GenPush( const CSyntaxNode&, vector< CInstr >& );
	void											GenPush( float val, vector< CInstr >& vAssembler );
	void											GenPush(CMemory* pMemory, vector< CInstr >& vAssembler);
	void											GenCall( const CSyntaxNode& oNode, vector< CInstr >& );
	void											GenJe(string label, vector< CInstr >& vAssembler);
	void											GenJne(string label, vector< CInstr >& vAssembler);
	void											GenJae(string label, vector< CInstr >& vAssembler);
	void											GenJbe(string label, vector< CInstr >& vAssembler);
	void											GenCmp(CRegister::TType, CRegister::TType, vector< CInstr >& vAssembler);
	void											GenCmp(CRegister::TType reg, float val, vector< CInstr >& vAssembler);
	void											GenCmp(const CSyntaxNode& node1, const CSyntaxNode& node2, vector< CInstr >& vAssembler, const map<string, int>& mFuncAddr, CVarMap& mVar);
	void											GenCmp(CMemory* pMemory, float val, vector< CInstr >& vAssembler);
	void											GenCmp(CMemory* pMemory1, CMemory* pMemory2, vector< CInstr >& vAssembler);
	void											GenCmp(float val1, float val2, vector< CInstr >& vAssembler);
	void											PutLabel(string sLabel, vector< CInstr >& vAssembler);
	void											GenRet(vector< CInstr >&);
	void											GenReturn(vector< CInstr >&);
	void											GenMov( CRegister::TType a, CRegister::TType b, vector< CInstr >& );
	CMemory*										CreateMemoryRegister(CRegister::TType eBase, CRegister::TType eIndex, int nDisplacement);
	CMemory*										CreateVarMemoryRegister(const CVar& var);
	void											GenMovAddrImm(CMemory* pMemory, const CSyntaxNode& oImm, vector< CInstr >& vAssembler );
	void											GenMovAddrReg(CRegister::TType scrReg, CMemory* pdestMem, vector< CInstr >& vAssembler);
	void											GenMovRegReg(CRegister::TType destReg, CRegister::TType srcReg, vector< CInstr >& vAssembler);
	void											GenMovRegAddr(CRegister::TType destReg, CMemory* pSrcMemory, vector< CInstr >& vAssembler);
	void											GenMovAddrAddr(CMemory* pSrcMemory, CMemory* pDestMemory, vector< CInstr >& vAssembler);
	void											GenAddRegImm(CRegister::TType, int val, vector< CInstr >&);
	void											GenSubRegImm(CRegister::TType, int val, vector< CInstr >&);
	void											GenPop( CRegister::TType, vector< CInstr >& );
	void											FillOperandFromSyntaxNode( CNumeric* oOperand, const CSyntaxNode& oTree );
	void											ResolveAddresses( vector< CInstr >& vCodeOut );
	void											GenAssemblerFirstPass( const CSyntaxNode& oTree, vector< CInstr >& vCodeOut, const map<string, int>& mFuncAddr, CVarMap& mVar );
	void											CreateStackFrame(vector< CInstr >& vCodeOut);
	string											GenerateNewLabel();
	void											Enter(vector<CInstr>& vAssembler);
	void											Leave(vector<CInstr>& vAssembler);

	map< CLexem::Type, TMnemonic >				m_mTypeToMnemonic;
	map< TMnemonic, string >						m_mMnemonicToString;
	map< string, vector< pair< int, int > >	>		m_mStringInstr; // Pour chaque string en dur, contient le numéro de l'instruction et de l'opérande
	int												m_nCurrentScopeNumber;
	bool											m_bEaxBusy;
	int												m_nLastLabelIndex;
	bool											m_bMainRetPlaced;
	bool											m_bPutAllCodeIntoSameMemory;
	vector<vector<CInstr>>							m_vAssemblerFunctions;
};

#endif // CODEGEN_H