#include "BinGenerator.h"
#include "AsmGenerator.h"

int CBinGenerator::s_tabInstr[ CAsmGenerator::eMnemonicCount ][ eTypeInstrCount ][ eTypeInstrCount ] = {{{-1}}};
//int CBinGenerator::s_InstrSize[ eTypeInstrCount ][ eTypeInstrCount ] = {{-1}};
vector< int > CBinGenerator::s_vInstrSize;

void CBinGenerator::AddImmToByteArray( float nImm, vector< unsigned char >& vBin )
{
	vBin.resize( vBin.size() + 4 );
	memcpy( &vBin[ 0 ] + vBin.size() - 4, &nImm, 4 );
}

CBinGenerator::CBinGenerator(CSyntaxAnalyser& oSyntaxAnalyser) :
	m_oSyntaxAnalyser(oSyntaxAnalyser)
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
	
	s_tabInstr[CAsmGenerator::eJne][eImm][0] = iInstrNum++; // 44

	s_vInstrSize.push_back( -1 ); // pour démarrer à 1

	s_vInstrSize.push_back( 2 );
	s_vInstrSize.push_back( 6 );
	s_vInstrSize.push_back( 4 );
	s_vInstrSize.push_back( 4 );
	s_vInstrSize.push_back( 7 );

	for( int i = CAsmGenerator::eAdd; i <= CAsmGenerator::eDiv; i++ )
	{
		s_vInstrSize.push_back( 2 );
		s_vInstrSize.push_back( 6 );
		s_vInstrSize.push_back( 6 );
		s_vInstrSize.push_back( 6 );
		s_vInstrSize.push_back( 6 );
		s_vInstrSize.push_back( 10 );
	}
	
	s_vInstrSize.push_back( 2 );
	s_vInstrSize.push_back( 5 );
	s_vInstrSize.push_back( 3 );

	s_vInstrSize.push_back( 2 );
	s_vInstrSize.push_back( 6 );

	s_vInstrSize.push_back( 2 ); // 35
	s_vInstrSize.push_back( 5 ); // call imm, 36
	s_vInstrSize.push_back( 6 ); // call addr, 37

	s_vInstrSize.push_back( 2 ); // 38
	s_vInstrSize.push_back( 5 ); // 39
	s_vInstrSize.push_back( 6 ); // 40

	s_vInstrSize.push_back( 1 ); // eRet, 41
	s_vInstrSize.push_back( 1 ); // eReturn, 42

	s_vInstrSize.push_back(7); // cmp, 43
	s_vInstrSize.push_back(5); // jne, 44
}

//int	CBinGenerator::GetInstrSize( int nInstrNum )
//{
//	return s_vInstrSize[ nInstrNum ];
//}

void CBinGenerator::GenBinary(const vector<CAsmGenerator::CInstr>& vAsmCode, vector< unsigned char >& vBin)
{
	for( unsigned int i = 0; i < vAsmCode.size(); i++ )
		GenInstructionBinary( vAsmCode[ i ], vBin );
}


void CBinGenerator::GenInstructionBinary( const CAsmGenerator::CInstr& oInstr, vector< unsigned char >& vBin )
{
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
					vBin.push_back( s_tabInstr[ oInstr.m_eMnem ][ eReg ][ eReg ] );
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
						vBin.push_back( s_tabInstr[ oInstr.m_eMnem ][ eReg ][ eImm ] );
						vBin.push_back( pReg1->m_eValue );
						AddImmToByteArray( pNum->m_fValue, vBin );
					}
					else {
						const CMemory* pMemory = dynamic_cast<CMemory*>(oInstr.m_vOperand[1]);
						if (pMemory) {
							// reg mem
							vBin.push_back(s_tabInstr[CAsmGenerator::eMov][eReg][eAddr]);
							vBin.push_back(pReg1->m_eValue);
							GenMemoryBinary(pMemory, vBin);
						}
					}
				}
			}
			else
			{
				// reg
				vBin.push_back( s_tabInstr[ oInstr.m_eMnem ][ eReg ][ 0 ] );
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
						vBin.push_back( s_tabInstr[ oInstr.m_eMnem ][ eImm ][ eReg ] );
						vBin.push_back( pReg2->m_eValue );
						AddImmToByteArray( pNum1->m_fValue, vBin );
					}
				}
				else
				{
					// imm
					vBin.push_back( s_tabInstr[ oInstr.m_eMnem ][ eImm ][ 0 ] );
					AddImmToByteArray( pNum1->m_fValue, vBin );
				}
			}
			else {
				const CMemory* pMem = dynamic_cast< const CMemory* >(oInstr.m_vOperand[0]);
				if (pMem) {
					if (oInstr.m_vOperand.size() < 2) {
						vBin.push_back(s_tabInstr[oInstr.m_eMnem][eAddr][0]);
						GenMemoryBinary(pMem, vBin);
					}
					else {
						const CNumeric* pNum = dynamic_cast<const CNumeric*>(oInstr.m_vOperand[1]);
						if (pNum)
						{
							// mem imm
							vBin.push_back(s_tabInstr[oInstr.m_eMnem][eAddr][eImm]);
							GenMemoryBinary(pMem, vBin);
							AddImmToByteArray(pNum->m_fValue, vBin);
						}
						else {
							const CRegister* pReg2 = dynamic_cast<const CRegister*>(oInstr.m_vOperand[1]);
							if (pReg2) {
								// mem reg
								vBin.push_back(s_tabInstr[oInstr.m_eMnem][eAddr][eReg]);
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
							int address = vBin.size();
							string label = pString->m_sValue.substr(0, pString->m_sValue.size() - 1);
							m_mLabelAddr[label].first = address;
							for (unsigned int i : m_mLabelAddr[label].second) {
								memcpy(&vBin[0] + i, &address, 4);
							}
						}
						else {
							vBin.push_back(s_tabInstr[oInstr.m_eMnem][eImm][0]);
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
		vBin.push_back( s_tabInstr[ oInstr.m_eMnem ][ 0 ][ 0 ] );
	}
}

void CBinGenerator::GenMemoryBinary(const CMemory* pMemory, vector< unsigned char >& vBin)
{
	vBin.push_back(pMemory->m_oBase.m_eValue | pMemory->m_oIndex.m_eValue << 4);
	vBin.push_back(pMemory->m_nDisplacement);
}