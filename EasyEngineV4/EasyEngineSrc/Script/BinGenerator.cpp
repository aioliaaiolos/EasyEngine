#include "BinGenerator.h"
#include "AsmGenerator.h"
#include "Exception.h"

int CBinGenerator::s_tabInstr[ CAsmGenerator::eMnemonicCount ][ eTypeInstrCount ][ eTypeInstrCount ] = {{{-1}}};
//int CBinGenerator::s_InstrSize[ eTypeInstrCount ][ eTypeInstrCount ] = {{-1}};
vector< int > CBinGenerator::s_vInstrSize;

void CBinGenerator::AddImmToByteArray( float nImm, vector< unsigned char >& vBin )
{
	vBin.resize( vBin.size() + 4 );
	memcpy( &vBin[ 0 ] + vBin.size() - 4, &nImm, 4 );
}

CBinGenerator::CBinGenerator(CSyntaxAnalyser& oSyntaxAnalyser, CAsmGenerator& oCodeGenerator) :
	m_oSyntaxAnalyser(oSyntaxAnalyser),
	m_oCodeGenerator(oCodeGenerator)
{
	int iInstrNum = 1;

	// Mov
	s_tabInstr[ CAsmGenerator::eMov ][ eReg ][ eReg ] = iInstrNum++; // 1
	s_tabInstr[ CAsmGenerator::eMov ][ eReg ][ eImm ] = iInstrNum++; // 2
	s_tabInstr[ CAsmGenerator::eMov ][ eReg ][ eAddr ] = iInstrNum++; // 3
	s_tabInstr[ CAsmGenerator::eMov ][ eAddr ][ eReg ] = iInstrNum++; // 4
	s_tabInstr[ CAsmGenerator::eMov ][ eAddr ][ eImm ] = iInstrNum++; // 5

	// Operations
	for( int i = CAsmGenerator::eAdd; i <= CAsmGenerator::eDiv; i++ )
	{
		s_tabInstr[ i ][ eReg ][ eReg ] = iInstrNum++;
		s_tabInstr[ i ][ eReg ][ eImm ] = iInstrNum++;
		s_tabInstr[ i ][ eReg ][ eAddr ] = iInstrNum++;
		s_tabInstr[ i ][ eImm ][ eReg ] = iInstrNum++;
		s_tabInstr[ i ][ eAddr ][ eReg ] = iInstrNum++;
		s_tabInstr[ i ][ eAddr ][ eImm ] = iInstrNum++;
	}

	s_tabInstr[ CAsmGenerator::ePush ][ eReg ][ 0 ] = iInstrNum++; // 30
	s_tabInstr[ CAsmGenerator::ePush ][ eImm ][ 0 ] = iInstrNum++; // 31
	s_tabInstr[ CAsmGenerator::ePush ][ eAddr ][ 0 ] = iInstrNum++; // 32

	s_tabInstr[ CAsmGenerator::ePop ][ eReg ][ 0 ] = iInstrNum++; // 33
	s_tabInstr[ CAsmGenerator::ePop ][ eAddr ][ 0 ] = iInstrNum++; // 34

	s_tabInstr[ CAsmGenerator::eCall ][ eReg ][ 0 ] = iInstrNum++; // 35
	s_tabInstr[ CAsmGenerator::eCall ][ eImm ][ 0 ] = iInstrNum++; // 36
	s_tabInstr[ CAsmGenerator::eCall ][ eAddr ][ 0 ] = iInstrNum++; // 37

	s_tabInstr[ CAsmGenerator::eInt ][ eReg ][ 0 ] = iInstrNum++; // 38
	s_tabInstr[ CAsmGenerator::eInt ][ eImm ][ 0 ] = iInstrNum++; // 39
	s_tabInstr[ CAsmGenerator::eInt ][ eAddr ][ 0 ] = iInstrNum++; // 40

	s_tabInstr[ CAsmGenerator::eRet ][ 0 ][ 0 ] = iInstrNum++; // 41
	s_tabInstr[CAsmGenerator::eReturn][0][0] = iInstrNum++; // 42

	s_tabInstr[CAsmGenerator::eCmp][eAddr][eImm] = iInstrNum++; // 43
	s_tabInstr[CAsmGenerator::eCmp][eReg][eImm] = iInstrNum++; // 44
	s_tabInstr[CAsmGenerator::eCmp][eImm][eImm] = iInstrNum++; // 45
	
	s_tabInstr[CAsmGenerator::eJne][eImm][0] = iInstrNum++; // 46
	s_tabInstr[CAsmGenerator::eJae][eImm][0] = iInstrNum++; // 47
	s_tabInstr[CAsmGenerator::eJbe][eImm][0] = iInstrNum++; // 48

	s_vInstrSize.push_back( -1 ); // pour démarrer à 1

	s_vInstrSize.push_back( 2 ); // mov reg, reg
	s_vInstrSize.push_back( 6 ); // mov reg, imm
	s_vInstrSize.push_back( 4 ); // mov imm, reg
	s_vInstrSize.push_back( 4 ); // mov addr, reg
	s_vInstrSize.push_back( 7 ); // mov addr, imm

	for( int i = CAsmGenerator::eAdd; i <= CAsmGenerator::eDiv; i++ )
	{
		s_vInstrSize.push_back( 2 ); // op reg, reg
		s_vInstrSize.push_back( 6 ); // op reg, imm
		s_vInstrSize.push_back( 4 ); // op reg, addr
		s_vInstrSize.push_back( 6 ); // op imm, reg
		s_vInstrSize.push_back( 6 ); // op addr, reg
		s_vInstrSize.push_back( 10 );// op addr, imm
	}
	
	s_vInstrSize.push_back( 2 ); // 30
	s_vInstrSize.push_back( 5 ); // 31
	s_vInstrSize.push_back( 3 ); // 32

	s_vInstrSize.push_back( 2 ); // 33
	s_vInstrSize.push_back( 6 ); // 34

	s_vInstrSize.push_back( 2 ); // 35
	s_vInstrSize.push_back( 5 ); // call imm, 36
	s_vInstrSize.push_back( 6 ); // call addr, 37

	s_vInstrSize.push_back( 2 ); // 38
	s_vInstrSize.push_back( 5 ); // int imm : 39
	s_vInstrSize.push_back( 6 ); // int addr : 40

	s_vInstrSize.push_back( 1 ); // eRet, 41
	s_vInstrSize.push_back( 1 ); // eReturn, 42

	s_vInstrSize.push_back(7); // cmp addr, imm : 43
	s_vInstrSize.push_back(6); // cmp reg, imm : 44
	s_vInstrSize.push_back(9); // cmp imm, imm : 45

	s_vInstrSize.push_back(5); // jne, 46
	s_vInstrSize.push_back(5); // jae, 47
	s_vInstrSize.push_back(5); // jbe, 48
}

//int	CBinGenerator::GetInstrSize( int nInstrNum )
//{
//	return s_vInstrSize[ nInstrNum ];
//}

void CBinGenerator::GenBinary(const vector<CAsmGenerator::CInstr>& vAsmCode, vector< unsigned char >& vBin)
{
	for( unsigned int i = 0; i < vAsmCode.size(); i++ )
		GenInstructionBinary( vAsmCode[ i ], vBin );

	if (m_oSyntaxAnalyser.m_vFunctions.size() > m_vFunctionsBin.size()) {
		int oldSize = m_vFunctionsBin.size();
		m_vFunctionsBin.resize(m_oCodeGenerator.GetFunctions().size());
		for (unsigned int i = oldSize; i < m_oCodeGenerator.GetFunctions().size(); i++)
			for (unsigned int j = 0; j < m_oCodeGenerator.GetFunctions()[i].size(); j++)
				GenInstructionBinary(m_oCodeGenerator.GetFunctions()[i][j], m_vFunctionsBin[i]);
	}
}

const vector<vector<unsigned char>>& CBinGenerator::GetFunctions() const
{
	return m_vFunctionsBin;
}

void CBinGenerator::GenInstructionBinary( const CAsmGenerator::CInstr& oInstr, vector< unsigned char >& vBin )
{
	int instrIndex = 0;
	if( oInstr.m_vOperand.size() > 0 )
	{
		const CRegister* pReg1 = dynamic_cast< const CRegister* >( oInstr.m_vOperand[ 0 ] );
		if( pReg1 )
		{
			if( oInstr.m_vOperand.size() > 1 )
			{
				const CRegister* pReg2 = dynamic_cast< const CRegister* >( oInstr.m_vOperand[ 1 ] );
				if( pReg2 )
				{
					// reg reg
					instrIndex = s_tabInstr[oInstr.m_eMnem][eReg][eReg];
					vBin.push_back(instrIndex);
					int r1 = pReg1->m_eValue << 4;
					int r2 = pReg2->m_eValue;
					vBin.push_back( r1 | r2 );
				}
				else 
				{
					const CNumeric* pNum = dynamic_cast< const CNumeric* >( oInstr.m_vOperand[ 1 ] );
					if( pNum )
					{
						// reg imm
						instrIndex = s_tabInstr[oInstr.m_eMnem][eReg][eImm];
						vBin.push_back(instrIndex);
						vBin.push_back( pReg1->m_eValue );
						AddImmToByteArray( pNum->m_fValue, vBin );
					}
					else {
						const CMemory* pMemory = dynamic_cast<CMemory*>(oInstr.m_vOperand[1]);
						if (pMemory) {
							// reg mem
							instrIndex = s_tabInstr[oInstr.m_eMnem][eReg][eAddr];
							vBin.push_back(instrIndex);
							vBin.push_back(pReg1->m_eValue);
							GenMemoryBinary(pMemory, vBin);
						}
					}
				}
			}
			else
			{
				// reg
				instrIndex = s_tabInstr[oInstr.m_eMnem][eReg][0];
				vBin.push_back(instrIndex);
				vBin.push_back( pReg1->m_eValue );
			}
		}
		else
		{
			const CNumeric* pNum1 = dynamic_cast< const CNumeric* >( oInstr.m_vOperand[ 0 ] );
			if( pNum1 )
			{
				if( oInstr.m_vOperand.size() > 1 )
				{
					const CRegister* pReg2 = dynamic_cast< const CRegister* >( oInstr.m_vOperand[ 1 ] );
					if( pReg2 )
					{
						// reg imm
						instrIndex = s_tabInstr[oInstr.m_eMnem][eImm][eReg];
						vBin.push_back(instrIndex);
						vBin.push_back( pReg2->m_eValue );
						AddImmToByteArray( pNum1->m_fValue, vBin );
					}
					else {
						const CNumeric* pNum2 = dynamic_cast< const CNumeric* >(oInstr.m_vOperand[1]);
						if (pNum2) {
							instrIndex = s_tabInstr[oInstr.m_eMnem][eImm][eImm];
							vBin.push_back(instrIndex);							
							AddImmToByteArray(pNum1->m_fValue, vBin);
							AddImmToByteArray(pNum2->m_fValue, vBin);
						}
					}
				}
				else
				{
					// imm
					instrIndex = s_tabInstr[oInstr.m_eMnem][eImm][0];
					vBin.push_back(instrIndex);
					AddImmToByteArray( pNum1->m_fValue, vBin );
				}
			}
			else {
				const CMemory* pMem = dynamic_cast< const CMemory* >(oInstr.m_vOperand[0]);
				if (pMem) {
					if (oInstr.m_vOperand.size() < 2) {
						instrIndex = s_tabInstr[oInstr.m_eMnem][eAddr][0];
						vBin.push_back(instrIndex);
						GenMemoryBinary(pMem, vBin);
					}
					else {
						const CNumeric* pNum = dynamic_cast<const CNumeric*>(oInstr.m_vOperand[1]);
						if (pNum)
						{
							// mem imm
							instrIndex = s_tabInstr[oInstr.m_eMnem][eAddr][eImm];
							vBin.push_back(instrIndex);
							GenMemoryBinary(pMem, vBin);
							AddImmToByteArray(pNum->m_fValue, vBin);
						}
						else {
							const CRegister* pReg2 = dynamic_cast<const CRegister*>(oInstr.m_vOperand[1]);
							if (pReg2) {
								// mem reg
								instrIndex = s_tabInstr[oInstr.m_eMnem][eAddr][eReg];
								vBin.push_back(instrIndex);
								GenMemoryBinary(pMem, vBin);
								vBin.push_back(pReg2->m_eValue);
							}
						}
					}
				}
				else {
					const CPreprocessorString* pString = dynamic_cast<const CPreprocessorString* >(oInstr.m_vOperand[0]);
					if (pString) {
						if (oInstr.m_eMnem == CAsmGenerator::eNone) {
							instrIndex = -1;
							int address = vBin.size();
							string label = pString->m_sValue.substr(0, pString->m_sValue.size() - 1);
							m_mLabelAddr[label].first = address;
							for (unsigned int i : m_mLabelAddr[label].second) {
								memcpy(&vBin[0] + i, &address, 4);
							}
						}
						else {
							instrIndex = s_tabInstr[oInstr.m_eMnem][eImm][0];
							vBin.push_back(instrIndex);
							m_mLabelAddr[pString->m_sValue].second.push_back(vBin.size());
							for (int i = 0; i < 4; i++)
								vBin.push_back(0);
						}
					}
				}
			}
		}
	}
	else
	{
		instrIndex = -1;
		vBin.push_back( s_tabInstr[ oInstr.m_eMnem ][ 0 ][ 0 ] );
	}

	if (instrIndex == 0) {
		CCompilationErrorException e(-1, -1);
		e.SetErrorMessage("Bytecode generation error : instruction not exists");
		throw e;
	}
}

void CBinGenerator::GenMemoryBinary(const CMemory* pMemory, vector< unsigned char >& vBin)
{
	vBin.push_back(pMemory->m_oBase.m_eValue | pMemory->m_oIndex.m_eValue << 4);
	vBin.push_back(pMemory->m_nDisplacement);
}