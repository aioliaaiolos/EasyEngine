#ifndef BINGENERATOR_H
#define BINGENERATOR_H

#include <vector>
#include "AsmGenerator.h"

using namespace std;


class CBinGenerator
{
	enum TTypeInstr
	{
		eReg = 0,
		eImm,
		eAddr,
		eTypeInstrCount
	};

public:
	enum TProcInstr
	{
		eMovRegReg = 1, // op = reg1(0.5) + reg2(0.5)
		eMovRegImm,		// op = reg(1) + imm(4)
		eMovRegAddr,	// op = reg(1) + reg base(0.5) + reg index(0.5) + displacement(1) 
		eMovAddrReg,	// op = reg base(0.5) + reg index(0.5) + displacement(1) + reg(1)
		eMovAddrImm,	// op = reg base(0.5) + reg index(0.5) + displacement(1) + Imm(4)

		eAddRegReg,		// op = reg1(0.5) + reg2(0.5)
		eAddRegImm,		// op = reg(1) + Imm(4)
		eAddRegAddr,	// op = reg(1) + reg base(0.5) + reg index(0.5) + displacement(1)
		eAddImmReg,		// op = reg(1) + imm(4)
		eAddAddrReg,	// op = reg addr( 0.5) + reg(0.5) + addr(4)
		eAddAddrImm,	// op = reg addr(1) + addr(4) + imm(4)

		eMulRegReg,		// op = reg1(0.5) + reg2(0.5)
		eMulRegImm,		// op = reg(1) + Imm(4)
		eMulRegAddr,	// op = reg(0.5) + reg addr(0.5) + addr(4)
		eMulImmReg,		// op = reg(1) + imm(4)
		eMulAddrReg,	// op = reg addr( 0.5) + reg(0.5) + addr(4)
		eMulAddrImm,	// op = reg addr(1) + addr(4) + imm(4)

		eSubRegReg,		// op = reg1(0.5) + reg2(0.5)
		eSubRegImm,		// op = reg(1) + Imm(4)
		eSubRegAddr,	// op = reg(0.5) + reg addr(0.5) + addr(4)
		eSubImmReg,		// op = reg(1) + imm(4)
		eSubAddrReg,	// op = reg addr( 0.5) + reg(0.5) + addr(4)
		eSubAddrImm,	// op = reg addr(1) + addr(4) + imm(4)		

		eDivRegReg,		// op = reg1(0.5) + reg2(0.5)
		eDivRegImm,		// op = reg(1) + Imm(4)
		eDivRegAddr,	// op = reg(0.5) + reg addr(0.5) + addr(4)
		eDivImmReg,		// op = reg(1) + imm(4)
		eDivAddrReg,	// op = reg addr( 0.5) + reg(0.5) + addr(4)
		eDivAddrImm,	// op = reg addr(1) + addr(4) + imm(4)

		ePushReg,		// op = reg(1)
		ePushImm,		// op = imm(4)
		ePushAddr,		// op = reg base(0.5) + reg index(0.5) + displacement(1)

		ePopReg,
		ePopAddr,

		eCallReg,		// op = reg(1)
		eCallImm,		// op = imm(4)
		eCallAddr,		// op = reg(1) + addr(4)

		eIntReg,
		eIntImm,
		eIntAddr,

		eRet,
		eReturn
	};

private:
	typedef void (*TGen2RegsInstr)( CRegister::TType r1, CRegister::TType r2, vector< int >& vBin );
	typedef void (*TGenRegImmInstr)( CRegister::TType r1, double fValue, vector< int >& vBin );

	map< CAsmGenerator::TMnemonic, int > MnemToInstrNum;

	void				GenInstructionBinary(const CAsmGenerator::CInstr& oInstr, vector< unsigned char >& vBin);
	void				GenMemoryBinary(const CMemory* pMemory, vector< unsigned char >& vBin);
	CSyntaxAnalyser&	m_oSyntaxAnalyser;
	
	static void AddImmToByteArray( float nImm, vector< unsigned char >& vBin );

	static int				s_tabInstr[ CAsmGenerator::eMnemonicCount ][ eTypeInstrCount ][ eTypeInstrCount ];	

public:
	CBinGenerator(CSyntaxAnalyser& oSyntaxAnalyser);
	void	GenBinary(const vector< CAsmGenerator::CInstr >& vAsmCode, const vector<vector<CAsmGenerator::CInstr>>& vAsmFunctionsCode, vector< unsigned char >& vBin);
	static vector< int >	s_vInstrSize;
	vector<vector<unsigned char>> m_vFunctionsBin;
	//int		GetInstrSize( int nInstrNum );
};

#endif // BINGENERATOR_H