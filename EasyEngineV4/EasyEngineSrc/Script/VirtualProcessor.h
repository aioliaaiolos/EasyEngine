#include <vector>
#include <map>
#include "BinGenerator.h"
#include "SemanticAnalyser.h"

using namespace std;

#define MEM_SIZE 1024

class CVirtualProcessor
{

public:
	CVirtualProcessor(CSemanticAnalyser* pSemanticAnalyser, CBinGenerator* pBinGenerator);
	void	Execute(const vector< unsigned char >& vBinary, const vector< int >& vInstrSize);
	float	GetVariableValue(string varName);
	float	GetRegisterValue(CRegister::TType reg);

private:
	typedef void (*TInstrFunc)( unsigned char* );
	float			m_nEip;
	float			m_nEax;
	float			m_nEbx;
	float			m_nEcx;
	float			m_nEdx;
	float 			m_nEsi;
	float 			m_nEdi;
	float 			m_nEbp;
	float 			m_nEsp;
	unsigned int	m_nFlags;

	enum TFlag
	{
		CF = 1,
		Flag2 = 1 << 2,
		PF = 1 << 3,
		Flag3 = 1 << 4,
		AF = 1 << 5,
		Flag4 = 1 << 6,
		ZF = 1 << 7
	};

	map<int, int> m_mEbpValueByScope;


	float	m_pMemory[ MEM_SIZE ];

	map< CBinGenerator::TProcInstr, TInstrFunc > m_mInstrFunc;
	vector< float* >	m_vRegAddr;
	bool				m_bEnd;


	static void MovRegReg( unsigned char* pOperand );
	static void MovRegImm( unsigned char* pOperand );
	static void MovRegAddr( unsigned char* pOperand );
	static void MovAddrReg( unsigned char* pOperand );
	static void MovAddrImm( unsigned char* pOperand );

	static void AddRegReg( unsigned char* pOperand );
	static void AddRegImm( unsigned char* pOperand );

	static void MulRegReg( unsigned char* pOperand );
	static void MulRegImm( unsigned char* pOperand );

	static void DivRegReg( unsigned char* pOperand );
	static void DivRegImm( unsigned char* pOperand );

	static void SubRegImm( unsigned char* pOperand );

	static void PushReg( unsigned char* pOperand );
	static void PushImm( unsigned char* pOperand );
	static void PushAddr( unsigned char* pOperand );

	static void PopReg( unsigned char* pOperand );

	static void CallImm(unsigned char* pOperand);
	static void IntImm( unsigned char* pOperand );

	static void Ret( unsigned char* pOperand );
	static void Return(unsigned char* pOperand);

	static void CmpAddrImm(unsigned char* pOperand);
	static void JneImm(unsigned char* pOperand);

	static CVirtualProcessor* s_pCurrentInstance;
	static CSemanticAnalyser* s_pSemanticAnalyser;
	static CBinGenerator* s_pBinGenerator;

	static int GetMemRegisterAddress(unsigned char* pOperand);

};