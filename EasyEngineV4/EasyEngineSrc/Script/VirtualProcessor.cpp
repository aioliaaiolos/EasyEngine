#include "VirtualProcessor.h"
#include "BinGenerator.h"
#include "Exception.h"
#include <sstream>


CVirtualProcessor* CVirtualProcessor::s_pCurrentInstance = nullptr;
CSemanticAnalyser* CVirtualProcessor::s_pSemanticAnalyser = nullptr;
CBinGenerator* CVirtualProcessor::s_pBinGenerator = nullptr;

CVirtualProcessor::CVirtualProcessor( CSemanticAnalyser* pSemanticAnalyser, CBinGenerator* pBinGenerator, bool bPutAllCodeIntoSameMemory):
m_nEip( 0 ),
m_nEsp( MEM_SIZE - 1 ),
m_bEnd( false ),
m_nEax( -1 ), m_nEbx( -1 ), m_nEcx( -1 ), m_nEdx( -1 ), m_nEsi( -1 ), m_nEdi( -1 ), m_nEbp( -1 ),
m_nFlags(0),
m_bPutAllCodeIntoSameMemory(bPutAllCodeIntoSameMemory)
{
	ZeroMemory( m_pMemory, MEM_SIZE * sizeof( float ) );
	s_pSemanticAnalyser = pSemanticAnalyser;
	s_pCurrentInstance = this;
	s_pBinGenerator = pBinGenerator;

	m_mInstrFunc[ CBinGenerator::eMovRegReg ] = MovRegReg;
	m_mInstrFunc[ CBinGenerator::eMovRegImm ] = MovRegImm;
	m_mInstrFunc[ CBinGenerator::eMovRegAddr ] = MovRegAddr;
	m_mInstrFunc[ CBinGenerator::eMovAddrReg ] = MovAddrReg;
	m_mInstrFunc[ CBinGenerator::eMovAddrImm ] = MovAddrImm;

	m_mInstrFunc[ CBinGenerator::eAddRegReg ] = AddRegReg;
	m_mInstrFunc[ CBinGenerator::eAddRegImm ] = AddRegImm;
#if 0
	m_mInstrFunc[ CBinGenerator::eAddRegAddr ] = AddRegAddr;
	m_mInstrFunc[ CBinGenerator::eAddImmReg ] = AddImmReg;
	m_mInstrFunc[ CBinGenerator::eAddAddrReg ] = AddAddrReg;
	m_mInstrFunc[ CBinGenerator::eAddAddrImm ] = AddAddrImm;

	m_mInstrFunc[ CBinGenerator::eSubRegReg ] = SubRegReg;
#endif // 0
	m_mInstrFunc[ CBinGenerator::eSubRegImm ] = SubRegImm;
	m_mInstrFunc[ CBinGenerator::eSubRegAddr] = SubRegAddr;
#if 0
	m_mInstrFunc[ CBinGenerator::eSubImmReg ] = SubImmReg;
	m_mInstrFunc[ CBinGenerator::eSubAddrReg ] = SubAddrReg;
	m_mInstrFunc[ CBinGenerator::eSubAddrImm ] = SubAddrImm;
#endif // 0

	m_mInstrFunc[ CBinGenerator::eMulRegReg ] = MulRegReg;
	m_mInstrFunc[ CBinGenerator::eMulRegImm ] = MulRegImm;
#if 0
	m_mInstrFunc[ CBinGenerator::eMulRegAddr ] = MulRegAddr;
	m_mInstrFunc[ CBinGenerator::eMulImmReg ] = MulImmReg;
	m_mInstrFunc[ CBinGenerator::eMulAddrReg ] = MulAddrReg;
	m_mInstrFunc[ CBinGenerator::eMulAddrImm ] = MulAddrImm;
#endif // 0
	m_mInstrFunc[ CBinGenerator::eDivRegReg ] = DivRegReg;
	m_mInstrFunc[ CBinGenerator::eDivRegImm ] = DivRegImm;

#if 0
	m_mInstrFunc[ CBinGenerator::eDivRegAddr ] = DivRegAddr;
	m_mInstrFunc[ CBinGenerator::eDivImmReg ] = DivImmReg;
	m_mInstrFunc[ CBinGenerator::eDivAddrReg ] = DivAddrReg;
	m_mInstrFunc[ CBinGenerator::eDivAddrImm ] = DivAddrImm;
#endif // 0
	m_mInstrFunc[ CBinGenerator::ePushReg ] = PushReg;
	m_mInstrFunc[ CBinGenerator::ePushImm ]	= PushImm;
	m_mInstrFunc[ CBinGenerator::ePushAddr ] = 	PushAddr;

	m_mInstrFunc[ CBinGenerator::ePopReg ] = PopReg;

#if 0
	m_mInstrFunc[ CBinGenerator::ePopAddr ] = PopAddr;

	m_mInstrFunc[ CBinGenerator::eCallReg ]	= CallReg;
#endif // 0
	m_mInstrFunc[ CBinGenerator::eCallImm ]	= CallImm;
#if 0
	m_mInstrFunc[ CBinGenerator::eCallAddr ] = 	CallAddr;

	m_mInstrFunc[ CBinGenerator::eIntReg ] = IntReg;
#endif // 0
	m_mInstrFunc[ CBinGenerator::eIntImm ] = IntImm;
#if 0
	m_mInstrFunc[ CBinGenerator::eIntAddr ] = IntAddr;
#endif // 0

	m_mInstrFunc[ CBinGenerator::eRet ] = Ret;
	m_mInstrFunc[ CBinGenerator::eReturn] = Return;

	m_mInstrFunc[CBinGenerator::eCmpAddrImm] = CmpAddrImm;
	m_mInstrFunc[CBinGenerator::eCmpRegImm] = CmpRegImm;
	m_mInstrFunc[CBinGenerator::eCmpImmImm] = CmpImmImm;

	m_mInstrFunc[CBinGenerator::eJneImm] = JneImm;
	m_mInstrFunc[CBinGenerator::eJaeImm] = JaeImm;
	m_mInstrFunc[CBinGenerator::eJbeImm] = JbeImm;


	m_vRegAddr.resize( 8 );
	m_vRegAddr[CRegister::eax] = &m_nEax;
	m_vRegAddr[CRegister::ebx] = &m_nEbx;
	m_vRegAddr[CRegister::ecx] = &m_nEcx;
	m_vRegAddr[CRegister::edx] = &m_nEdx;
	m_vRegAddr[CRegister::esi] = &m_nEsi;
	m_vRegAddr[CRegister::edi] = &m_nEdi;
	m_vRegAddr[CRegister::ebp] = &m_nEbp;
	m_vRegAddr[CRegister::esp] = &m_nEsp;

	m_nEbp = m_nEsp;

	m_mEbpValueByScope[0] = m_nEbp;
}

void CVirtualProcessor::Execute( const vector<unsigned char>& vBinary, const vector< int >& vInstrSize )
{
	unsigned int iCurrInstr = 0;
	unsigned char pInstr[ 12 ];
	m_bEnd = false;
	m_nEip = 0;
	do
	{
		if( m_nEip >= vBinary.size() )
		{
			CRuntimeException e;
			throw e;
		}
		iCurrInstr = vBinary[ (int)m_nEip ];
		if (iCurrInstr > vInstrSize.size())
			throw CEException("Runtime error");
		int nInstrSize = vInstrSize[ iCurrInstr ];
		if( nInstrSize > 1 )
			memcpy( pInstr, &vBinary[ (int)m_nEip + 1 ], nInstrSize - 1 );
		m_nEip += nInstrSize;
		TInstrFunc f = m_mInstrFunc[ (CBinGenerator::TProcInstr)iCurrInstr ];
		if( f )
			f( pInstr );
		else
		{
			ostringstream oss;
			oss << "Erreur, instruction " << iCurrInstr << " inexistante";
			throw CEException("Runtime error");
		}
		if ( (m_nEip == vBinary.size()) || (m_nEip == -1))
			m_bEnd = true;
	}
	while( !m_bEnd );
}

float CVirtualProcessor::GetVariableValue(string varName)
{
	const CVar* var = s_pSemanticAnalyser->GetVariable(varName);
	if (var) {
		int ebp = 0;
		if (var->m_nScopePos == 0)
			return m_pMemory[var->m_nRelativeStackPosition];
		else {
			ebp = m_mEbpValueByScope[var->m_nScopePos];
			return m_pMemory[ebp - var->m_nRelativeStackPosition];
		}
	}
	return -1.f;
}

float CVirtualProcessor::GetRegisterValue(CRegister::TType reg)
{
	return *m_vRegAddr[reg];
}

void CVirtualProcessor::MovRegReg( unsigned char* pOperand )
{
	int r1 = pOperand[ 0 ] >> 4;
	int r2 = pOperand[ 0 ] & 0x0f;
	*s_pCurrentInstance->m_vRegAddr[ r1 ] = *s_pCurrentInstance->m_vRegAddr[ r2 ];
}

void CVirtualProcessor::MovRegImm( unsigned char* pOperand )
{
	int r = pOperand[ 0 ];
	float fOperand;
	memcpy( &fOperand, &pOperand[ 1 ], 4 );
	*s_pCurrentInstance->m_vRegAddr[ r ] = fOperand;
}

int CVirtualProcessor::GetMemRegisterAddress(unsigned char* pOperand)
{
	unsigned char baseIndexId;
	char displacement;
	float fimm;
	int imm;
	memcpy(&baseIndexId, &pOperand[0], 1);
	memcpy(&displacement, &pOperand[1], 1);
	displacement /= 4;
	unsigned char baseId = baseIndexId & 0xf;
	unsigned char indexId = (baseIndexId >> 4);
	int base = baseId < s_pCurrentInstance->m_vRegAddr.size() ? *s_pCurrentInstance->m_vRegAddr[baseId] : 0;
	int index = indexId == 0xf ? 0 : *s_pCurrentInstance->m_vRegAddr[indexId];
	int address = base + index + displacement;
	return address;
}

void CVirtualProcessor::MovRegAddr(unsigned char* pOperand)
{
	int address = GetMemRegisterAddress(pOperand + 1);	
	int memValue = s_pCurrentInstance->m_pMemory[address];
	char regId = pOperand[0];
	*s_pCurrentInstance->m_vRegAddr[regId] = memValue;
}

void CVirtualProcessor::MovAddrReg( unsigned char* pOperand )
{
	int address = GetMemRegisterAddress(pOperand);
	char regId = pOperand[2];
	int regValue = *s_pCurrentInstance->m_vRegAddr[regId];
	s_pCurrentInstance->m_pMemory[address] = regValue;
}

void CVirtualProcessor::Cmp(float dest, float src)
{
	// dst < src : cf = 1, zf = 0
	// dst > src : cf = 0, zf = 0
	// dst = src : cf = 0, zf = 1

	if (dest < src) {
		s_pCurrentInstance->m_nFlags |= (unsigned int)TFlag::CF;
		s_pCurrentInstance->m_nFlags &= ~(unsigned int)TFlag::ZF;
	}
	else if (dest > src) {
		s_pCurrentInstance->m_nFlags &= ~(unsigned int)TFlag::CF;
		s_pCurrentInstance->m_nFlags &= ~(unsigned int)TFlag::ZF;
	}
	else {
		s_pCurrentInstance->m_nFlags &= ~(unsigned int)TFlag::CF;
		s_pCurrentInstance->m_nFlags |= (unsigned int)TFlag::ZF;
	}
}

void CVirtualProcessor::CmpAddrImm(unsigned char* pOperand)
{
	int address = GetMemRegisterAddress(pOperand);
	float fimm;
	int imm;
	memcpy(&fimm, &pOperand[2], 4);
	imm = fimm;
	float memValue = s_pCurrentInstance->m_pMemory[address];
	Cmp(memValue, imm);
}

void CVirtualProcessor::CmpRegImm(unsigned char* pOperand)
{
	int reg = (int)pOperand[0];
	float imm;
	memcpy(&imm, &pOperand[1], 4);
	float regValue = *s_pCurrentInstance->m_vRegAddr[reg];
	Cmp(regValue, imm);
}

void CVirtualProcessor::CmpImmImm(unsigned char* pOperand)
{
	float src, dst;
	memcpy(&dst, pOperand, 4);
	memcpy(&src, pOperand + 4, 4);
	Cmp(dst, src);
}

void CVirtualProcessor::JneImm(unsigned char* pOperand)
{
	unsigned char jmpAddr = pOperand[0];
	int zf = s_pCurrentInstance->m_nFlags & (unsigned int)TFlag::ZF;
	if(zf == 0)
		s_pCurrentInstance->m_nEip = jmpAddr;
}

void CVirtualProcessor::JaeImm(unsigned char* pOperand)
{
	unsigned char jmpAddr = pOperand[0];
	int cf = s_pCurrentInstance->m_nFlags & (unsigned int)TFlag::CF;
	if (cf == 0 )
		s_pCurrentInstance->m_nEip = jmpAddr;
}

void CVirtualProcessor::JbeImm(unsigned char* pOperand)
{
	unsigned char jmpAddr = pOperand[0];
	int cf = s_pCurrentInstance->m_nFlags & (unsigned int)TFlag::CF;
	int zf = s_pCurrentInstance->m_nFlags & (unsigned int)TFlag::ZF;
	if ( (cf != 0) || (zf != 0))
		s_pCurrentInstance->m_nEip = jmpAddr;
}

void CVirtualProcessor::MovAddrImm( unsigned char* pOperand )
{
	int address = GetMemRegisterAddress(pOperand);
	float fimm;
	int imm;
	memcpy(&fimm, &pOperand[2], 4);
	imm = fimm;
	s_pCurrentInstance->m_pMemory[address] = imm;
}

void CVirtualProcessor::AddRegReg( unsigned char* pOperand )
{
	int r1 = pOperand[ 0 ] >> 4;
	int r2 = pOperand[ 0 ] & 0x0f;
	s_pCurrentInstance->m_nEax = *s_pCurrentInstance->m_vRegAddr[ r1 ] + *s_pCurrentInstance->m_vRegAddr[ r2 ];
}

void CVirtualProcessor::AddRegImm( unsigned char* pOperand )
{
	int r = (int)pOperand[ 0 ];
	float f;
	memcpy( &f, &pOperand[ 1 ], 4 );
	s_pCurrentInstance->m_nEax = *s_pCurrentInstance->m_vRegAddr[ r ] + f;
}

#if 0
void CVirtualProcessor::AddRegAddr( unsigned char* pOperand )
{
	throw 1;
}

void CVirtualProcessor::AddImmReg( unsigned char* pOperand )
{
	throw 1;
}

void CVirtualProcessor::AddAddrReg( unsigned char* pOperand )
{
	throw 1;
}

void CVirtualProcessor::AddAddrImm( unsigned char* pOperand )
{
	throw 1;
}

#endif // 0

void CVirtualProcessor::MulRegReg( unsigned char* pOperand )
{
	int r1 = (int)pOperand[ 0 ] >> 4;
	int r2 = pOperand[ 0 ] & 0x0f;
	s_pCurrentInstance->m_nEax = *s_pCurrentInstance->m_vRegAddr[ r1 ] * *s_pCurrentInstance->m_vRegAddr[ r2 ];
}

void CVirtualProcessor::MulRegImm( unsigned char* pOperand )
{
	int r = (int)pOperand[ 0 ];
	float f;
	memcpy( &f, &pOperand[ 1 ], 4 );
	s_pCurrentInstance->m_nEax = *s_pCurrentInstance->m_vRegAddr[ r ] * f;
}

void CVirtualProcessor::SubRegImm( unsigned char* pOperand )
{
	int r = (int)pOperand[ 0 ];
	float f;
	memcpy( &f, &pOperand[ 1 ], 4 );
	*s_pCurrentInstance->m_vRegAddr[r] -= f;
}

void CVirtualProcessor::SubRegAddr(unsigned char* pOperand)
{
	int r = (int)pOperand[0];
	int address = GetMemRegisterAddress(pOperand + 1);
	float fValue = s_pCurrentInstance->m_pMemory[address];
	*s_pCurrentInstance->m_vRegAddr[r] -= fValue;
}

void CVirtualProcessor::DivRegReg( unsigned char* pOperand )
{
	int r1 = pOperand[ 0 ] >> 4;
	int r2 = pOperand[ 0 ] & 0x0f;
	s_pCurrentInstance->m_nEax = *s_pCurrentInstance->m_vRegAddr[ r1 ] / *s_pCurrentInstance->m_vRegAddr[ r2 ];
}

void CVirtualProcessor::DivRegImm( unsigned char* pOperand )
{
	int r = pOperand[ 0 ];
	float f;
	memcpy( &f, &pOperand[ 1 ], 4 );
	s_pCurrentInstance->m_nEax = *s_pCurrentInstance->m_vRegAddr[ r ] / f;
}

void CVirtualProcessor::PushReg( unsigned char* pOperand )
{
	s_pCurrentInstance->m_pMemory[ (int)s_pCurrentInstance->m_nEsp ] = *s_pCurrentInstance->m_vRegAddr[ pOperand[ 0 ] ];
	s_pCurrentInstance->m_nEsp -= 1.f;
}

void CVirtualProcessor::PushImm( unsigned char* pOperand )
{
	memcpy( &s_pCurrentInstance->m_pMemory[ (int)s_pCurrentInstance->m_nEsp ], pOperand, 4 );
	s_pCurrentInstance->m_nEsp -= 1.f;
}

void CVirtualProcessor::PushAddr( unsigned char* pOperand )
{
	int address = GetMemRegisterAddress(pOperand);
	float memValue = s_pCurrentInstance->m_pMemory[address];

	memcpy(&s_pCurrentInstance->m_pMemory[(int)s_pCurrentInstance->m_nEsp], &memValue, 4);
	s_pCurrentInstance->m_nEsp -= 1.f;
}

void CVirtualProcessor::PopReg( unsigned char* pOperand )
{
	*s_pCurrentInstance->m_vRegAddr[ (int)pOperand[ 0 ]  ] = s_pCurrentInstance->m_pMemory[ (int)++s_pCurrentInstance->m_nEsp ];
}

void CVirtualProcessor::CallImm(unsigned char* pOperand)
{
	if (s_pCurrentInstance->m_bPutAllCodeIntoSameMemory) {
		int address;
		memcpy(&address, pOperand, 4);
		float nextInstructionAddress = s_pCurrentInstance->m_nEip;
		s_pCurrentInstance->m_pMemory[(int)s_pCurrentInstance->m_nEsp] = nextInstructionAddress;
		s_pCurrentInstance->m_nEsp--;
		s_pCurrentInstance->m_nEip = address;
	}
	else {
		float address;
		memcpy(&address, pOperand, 4);
		int backupIp = s_pCurrentInstance->m_nEip;
		bool backupEnd = s_pCurrentInstance->m_bEnd;
		int backupEsp = s_pCurrentInstance->m_nEsp;
		s_pCurrentInstance->m_nEip = 0;
		s_pCurrentInstance->Execute(s_pBinGenerator->GetFunctions()[(int)address], CBinGenerator::s_vInstrSize);
		s_pCurrentInstance->m_nEip = backupIp;
		s_pCurrentInstance->m_bEnd = backupEnd;
		s_pCurrentInstance->m_nEsp = backupEsp;
	}
}

void CVirtualProcessor::IntImm( unsigned char* pOperand )
{
	float fIndex;
	memcpy( &fIndex, pOperand, 4 );
	vector< TFuncArgType > vArgType;
	int nFuncArgCount = s_pSemanticAnalyser->GetFuncArgsCount( (int)fIndex );
	vector< float > vArgs;
	for( int i = 0; i < nFuncArgCount; i++ )
		vArgs.push_back( s_pCurrentInstance->m_pMemory[ (int)s_pCurrentInstance->m_nEsp + i + 1 ] );
	s_pCurrentInstance->m_nEsp += nFuncArgCount;
	s_pCurrentInstance->m_nEax = s_pSemanticAnalyser->CallInterruption( (int)fIndex, vArgs );
}

void CVirtualProcessor::Ret( unsigned char* pOperand )
{
	if (s_pCurrentInstance->m_bPutAllCodeIntoSameMemory) {
		int returnAddress = -1;
		if (s_pCurrentInstance->m_nEsp + 1 < MEM_SIZE) {
			s_pCurrentInstance->m_nEsp++;
			returnAddress = s_pCurrentInstance->m_pMemory[(int)s_pCurrentInstance->m_nEsp];
		}
		s_pCurrentInstance->m_nEip = returnAddress;
	}
	else {
		if (s_pCurrentInstance->m_nEsp == MEM_SIZE - 1)
			s_pCurrentInstance->m_bEnd = true;
		else
			MessageBox(NULL, "RET : Problème de pile incohérente", "", MB_ICONERROR);
	}
}

void CVirtualProcessor::Return(unsigned char* pOperand)
{
	s_pCurrentInstance->m_bEnd = true;
}
