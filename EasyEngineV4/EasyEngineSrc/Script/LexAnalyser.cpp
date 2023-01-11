#include "LexAnalyser.h"
#include "Utils/CsvReader.h"
#include "IFileSystem.h"
#include "Exception.h"

// stl
#include <sstream>
#include <algorithm>

CLexAnalyser::CLexAnalyser( string sCSVConfigName, IFileSystem* pFS )
{
	InitStringToLexemTypeArray();
	CalculLexicalArrayFromCSV( sCSVConfigName, pFS );
}

bool CLexem::IsOperation() const
{
	return ( m_eType == eAdd || m_eType == eSub || m_eType == eDiv || m_eType == eMult );
}

bool CLexem::IsCall() const
{
	return m_eType == eCall || m_eType == eAPICall;
}

bool CLexem::IsNumeric() const
{
	return m_eType == eInt || m_eType == eFloat || m_eType == eString;
}

bool CLexem::IsComparaison() const
{
	return m_eType == eComp || m_eType == eInf || m_eType == eSup;
}

void CLexAnalyser::InitStringToLexemTypeArray()
{
	m_mStringToLexemType["Identifier"] = CLexem::eCall;
	m_mStringToLexemType["Function"] = CLexem::eCall;
	m_mStringToLexemType["Var"] = CLexem::eVar;
	m_mStringToLexemType["LPar"] = CLexem::eLPar;
	m_mStringToLexemType["RPar"] = CLexem::eRPar;
	m_mStringToLexemType["Affectation"] = CLexem::eAffect;
	m_mStringToLexemType["String"] = CLexem::eString;
	m_mStringToLexemType["Sub"] = CLexem::eSub;
	m_mStringToLexemType["Add"] = CLexem::eAdd;
	m_mStringToLexemType["Div"] = CLexem::eDiv;
	m_mStringToLexemType["Int"] = CLexem::eInt;
	m_mStringToLexemType["Float"] = CLexem::eFloat;
	m_mStringToLexemType["Virg"] = CLexem::eVirg;
	m_mStringToLexemType["PTVirg"] = CLexem::ePtVirg;
	m_mStringToLexemType["Mult"] = CLexem::eMult;
	m_mStringToLexemType["FunctionDef"] = CLexem::eFunctionDef;
	m_mStringToLexemType["LBrace"] = CLexem::eLBraket;
	m_mStringToLexemType["RBrace"] = CLexem::eRBraket;
	m_mStringToLexemType["If"] = CLexem::eIf;
	m_mStringToLexemType["Comp"] = CLexem::eComp;
	m_mStringToLexemType["Sup"] = CLexem::eSup;
	m_mStringToLexemType["Inf"] = CLexem::eInf;
}

void CLexAnalyser::GetLexemArrayFromScript( string sScript, vector< CLexem >& vLexem )
{
	unsigned int i = 0;
	int iLine = 1;
	int column = 0;
	bool bEnd = false;
	while ( i < sScript.size() )
	{
		bool bFinalState = false;
		string sValue;
		int nCurrentState = 0;
		string sKeyword;
		int keywordStateFrom = -1;
		int keywordStateTo = -1;
		while ( !bFinalState )
		{
			if (sScript[i] == '/' && sScript[i + 1] == '*')
				ReadUntilEndComment(sScript, i, iLine);

			if( sScript[ i ] == '\n')
			{
				iLine++;
				column = 0;
			}
			if (sScript[i] == '/' && sScript[i + 1] == '/')
			{
				iLine++;
				column = 0;
				ReadUntilEndLine(sScript, i);
			}

			unsigned char c = sScript[ i ];
			if( c == 0 && nCurrentState == 0 )
				return;
			int nNextState = m_vAutomate[ nCurrentState ][ c ];
			
			if (nNextState != nCurrentState) {
				map<string, pair<int, int>>::iterator itKeyword = m_mKeyWord.find(sValue);
				if (itKeyword != m_mKeyWord.end()) {
					sKeyword = sValue;
					keywordStateFrom = itKeyword->second.first;
					keywordStateTo = itKeyword->second.second;
				}
			}

			if (!sKeyword.empty() && (nNextState == keywordStateFrom)) {
				nNextState = keywordStateTo;
			}

			map< int, CLexem::Type >::iterator it = m_mFinalStates.find( nNextState );
			if (  it != m_mFinalStates.end() )
				bFinalState = true;
			else if( nNextState != 0 )
				sValue.push_back( sScript[ i ] );
			if( nNextState == -1 && it == m_mFinalStates.end() )
			{
				CCompilationErrorException e( iLine, column );
				throw e;
			}
			nCurrentState = nNextState;
			i++;
			if (c == '\t')
				column += 4;
			else
				column++;
			if( i >= sScript.size() )
				bEnd = true;
		}
		CLexem l( m_mFinalStates[ nCurrentState ] );
		switch( l.m_eType )
		{
		case CLexem::eInt:
			l.m_nValue = atoi( sValue.c_str() );
			i--;
			break;
		case CLexem::eFloat:
			l.m_fValue = atof( sValue.c_str() );
			i--;
			break;
		case CLexem::eString:
			sValue = sValue.substr( 1 );
			l.m_sValue = sValue;
			break;
		case CLexem::eVar:
		case CLexem::eCall:
		{
			auto last = std::remove_if(sValue.begin(), sValue.end(), isspace);
			sValue.erase(last, sValue.end());
			l.m_sValue = sValue;
			i--;
			break;
		}
		case CLexem::eFunctionDef:
		{
			auto last = std::remove_if(sValue.begin(), sValue.end(), isspace);
			sValue.erase(last, sValue.end());
			l.m_sValue = sValue;
			i--;
			break;
		}
		case CLexem::eIf:
		case CLexem::eAffect:
			i--;
			break;
		}
		vLexem.push_back( l );
		if( i < sScript.size() )
			bEnd = false;
		if (bEnd) {
			//NormalizeSub(vLexem);
			return;
		}
	}
}

void CLexAnalyser::NormalizeSub(vector< CLexem >& vLexem)
{
	for (int i = 0; i < vLexem.size(); i++) {
		if (vLexem[i].m_eType == CLexem::eInt && vLexem[i].m_nValue < 0 ||
			vLexem[i].m_eType == CLexem::eFloat && vLexem[i].m_fValue < 0.f) {
			if (i > 0) {
				if (vLexem[i - 1].m_eType == CLexem::eInt || vLexem[i - 1].m_eType == CLexem::eFloat ||
					vLexem[i - 1].m_eType == CLexem::eVar || vLexem[i - 1].m_eType == CLexem::eCall ||
					vLexem[i - 1].m_eType == CLexem::eRPar) {
					CLexem lSub;
					lSub.m_eType = CLexem::eSub;
					vLexem.insert(vLexem.begin() + i - 1, lSub);
					vLexem[i].m_fValue = -vLexem[i].m_fValue;
					vLexem[i].m_nValue = -vLexem[i].m_nValue;
				}
			}
		}
	}
}

void CLexAnalyser::ReadUntilEndComment(string sScript, unsigned int& startIndex, int& line)
{
	while (startIndex < sScript.size() && !(sScript[startIndex] == '*' && sScript[startIndex + 1] == '/') ) {
		startIndex++;
		if(sScript[startIndex] == '\n')
			line++;
	}
	startIndex += 2;
}

void CLexAnalyser::ReadUntilEndLine(string sScript, unsigned int& startIndex)
{
	while (startIndex < sScript.size() && (sScript[startIndex] != '\n' ) ) {
		startIndex++;
	}
}

void CLexAnalyser::CalculStateCount( CCSVReader& r )
{
	r.Rewind();
	string sLineCount;	
	for( int i = 0; i < r.GetLineCount() - 1; i++ )
	{
		string sCell;
		r.ReadCell( sCell );
		if( sCell.size () > 0 && sCell != "FinalStates" )
		{
			sLineCount = sCell;			
		}
		else if( sCell == "FinalStates" )
		{
			m_nStateCount = atoi( sLineCount.c_str() );
			break;
		}
		r.NextLine();
	}
}

int CLexAnalyser::GenStringFromRegExpr(string sExpr, string& sOut)
{
	unsigned int i;

	for( i = 0; i < sExpr.size(); i++ )
	{
		try
		{
			if( sExpr[ i ] == ' ' )
				continue;
			if( sExpr[ i ] == '[' )
			{
				i += GenHookRegExpr( sExpr.substr(i), sOut );
				continue;
			}
			if( sExpr[i] == '|' )
			{
				i += GenStringFromRegExpr( sExpr.substr(i+1), sOut );
				continue;
			}
			if( sExpr[ i ] == '\'' )
			{
				if( sExpr.substr(i).size() < 3 )
				{
					CLineCompilationErrorException e(i);
					throw e;
				}
				if( sExpr[ i + 1 ] == '\\' )
				{
					if( sExpr[ i + 2 ] == 'n' )
					{
						sOut.push_back( '\n' );
						i += 3;
					}
					else if( sExpr[ i + 2 ] == '\'' )
					{
						sOut.push_back( '\\' );
						i += 2;
					}
					else if (sExpr[i + 2] == 'r')
					{
						sOut.push_back('\r');
						i += 3;
					}
					else if (sExpr[i + 2] == 't')
					{
						sOut.push_back('\t');
						i += 3;
					}
					else
					{
						CLineCompilationErrorException e(i);
						throw e;
					}
				}
				else
				{
					sOut.push_back( sExpr[i+1] );
					i += 2;
				}
				continue;
			}
			if( sExpr[ i ] == 'S' )
			{
				sOut.push_back( ' ' );
				continue;
			}
			if ( sExpr[ i ] == 'P' )
			{
				sOut.push_back( ';' );
				continue;
			}
		}
		catch( CLineCompilationErrorException& e )
		{
			CLineCompilationErrorException e2(i + e.GetErrorColumn() );
			throw e2;
		}
		CLineCompilationErrorException e3(i);
		throw e3;
	}
	return i;
}

int CLexAnalyser::GenHookRegExpr(string sExpr, string& sOut)
{	
	char cBegin = 0, cEnd = 0;
	int state = 0;
	for( unsigned int i = 0; i < sExpr.size(); i++ )
	{
		switch( state )		
		{
		case -1:
			{
				CLineCompilationErrorException e(i);
				throw e;
			}
		case 0:
			if( sExpr[ i ] == '[' )
				state = 1;
			else
				state = -1;
			break;
		case 1:			
			if ( sExpr[ i ] == '\'' )
				state = 2;
			else if( sExpr[ i ] != ' ' )
				state = -1;
			break;			
		case 2:
			if ( sExpr[ i ] == ' ' )
				state = -1;
			else
			{
				cBegin = sExpr[ i ];
				state = 3;
			}
			break;
		case 3:
			if( sExpr[ i ] == '\'' )
				state = 4;
			else
				state = -1;
			break;
		case 4:
			if( sExpr[ i ] == '.' )
				state = 5;
			else if ( sExpr[ i ] != ' ' )
				state = -1;
			break;
		case 5:
			if( sExpr[ i ] == '.' )
				state = 6;
			else
				state = -1;
			break;
		case 6:
			if( sExpr[ i ] == '\'' )
				state = 7;
			else if( sExpr[ i ] != ' ' )
				state = -1;
			break;
		case 7:
			if( sExpr[ i ] == ' ' )
				state = -1;
			else
			{
				cEnd = sExpr[ i ];
				state = 8;
			}
			break;
		case 8:
			if( sExpr[ i ] == '\'' )			
				state = 9;
			else
				state = -1;
			break;
		case 9:
			if( sExpr[ i ] == ']' )
			{
				for (char c = cBegin; c <= cEnd; c++ )
					sOut.push_back( c );
				return i;
			}
			else if(  sExpr[ i ] != ' ' )
				state = -1;
			break;
		}
	}
	return 0;
}

void CLexAnalyser::CalculFinalStates( CCSVReader& r )
{
	r.Rewind();
	string sCell;
	bool bEof = false;
	do
	{
		sCell.clear();
		bEof = !r.ReadCell( sCell );
		r.NextLine();
	}
	while( sCell != "FinalStates" && !bEof);
	sCell.clear();
	while( r.ReadCell( sCell ) && !sCell.empty())
	{
		int nState = atoi( sCell.c_str() );
		sCell.clear();
		r.ReadCell( sCell );
		map< string, CLexem::Type >::iterator it = m_mStringToLexemType.find( sCell );
		if( it != m_mStringToLexemType.end() )
			m_mFinalStates[ nState ] = it->second;
		else
		{
			exception e( sCell.c_str() );
			throw e;
		}
		sCell.clear();
		r.NextLine();
	}
}

void CLexAnalyser::CalculKeywords(CCSVReader& r)
{
	r.Rewind();
	string sCell;
	bool bEof = false;
	do
	{
		sCell.clear();
		bEof = !r.ReadCell(sCell);
		r.NextLine();
	} while (sCell != "Keywords" && !bEof);
	sCell.clear();
	while (r.ReadCell(sCell))
	{
		string sKeyWord = sCell;
		sCell.clear();
		r.ReadCell(sCell);
		int nState1 = atoi(sCell.c_str());
		sCell.clear();
		r.ReadCell(sCell);
		int nState2 = atoi(sCell.c_str());
		m_mKeyWord[sKeyWord] = pair<int, int>(nState1, nState2);
		sCell.clear();
		r.NextLine();
	}
}

void CLexAnalyser::CalculLexicalArrayFromCSV( string sCSVName, IFileSystem* pFS )
{
	FILE* pFile = pFS->OpenFile( sCSVName.c_str(), "r" );
	if( !pFile )
	{
		string sMessage = string( "Impossible d'ouvrir \"" ) + sCSVName + "\"";
		CFileException e( sMessage );
		throw e;
	}
	string d;
	pFS->GetLastDirectory( d );
	fclose( pFile );
	CCSVReader r( ';' );
	string sCSVConfigPath;
	if( d.size() > 0 )
		sCSVConfigPath = d + "\\" + sCSVName;
	else
		sCSVConfigPath = sCSVName;
	r.OpenFile( sCSVConfigPath );
	string sExpr, sOut;
	CalculStateCount( r );
	CalculFinalStates( r );
	CalculKeywords(r);
	r.Rewind();
	m_vAutomate.resize( m_nStateCount + 1 );
	for( unsigned int i = 0; i < m_vAutomate.size(); i++ )
		for( int j = 0; j < 255; j++ )
			m_vAutomate[ i ].push_back( -1 );
	int iLine = 1;
	int iColumn = 1;

	try
	{
		int iCurrentCell = 1;
		CCSVReader r2( ';' );
		r2.OpenFile( sCSVConfigPath );
		r.ReadCell( sExpr );
		vector< int > vNonFinalStates;
		while( r.ReadCell( sExpr ) )
		{
			sOut.clear();
			iColumn += GenStringFromRegExpr( sExpr, sOut ) + 1;

			r2.NextLine();
			for( int i = 0; i < m_nStateCount; i++ )
			{
				string s2, sCurrentState;
				r2.ReadCell( sCurrentState );
				if( sCurrentState == "" )
					break;
				int nCurrentState = atoi( sCurrentState.c_str() );
				vNonFinalStates.push_back( nCurrentState );
				bool bEndLine = false;
				for( int l = 1; l <= iCurrentCell; l++ )
				{
					s2.clear();
					bEndLine = !r2.ReadCell( s2 );
				}
				int nValue = -1;
				if( s2.size() > 0 )
					nValue = atoi( s2.c_str() );				 
				for( unsigned int j = 0; j < sOut.size(); j++ )
					m_vAutomate[ nCurrentState ][ sOut[ j ] ] = nValue;
				if( !bEndLine )
					r2.NextLine();
			}
			r2.Rewind();
			sExpr.clear();
			iCurrentCell++;
		}
		iColumn += GenStringFromRegExpr( sExpr, sOut ) + 1;
	}
	catch( CLineCompilationErrorException& e)
	{
		ostringstream oss;
		oss << "\"" << sCSVName << "\" : Erreur de compilation ligne " << iLine << " colonne " << iColumn + e.GetErrorColumn();
		MessageBox( NULL, oss.str().c_str(), "Erreur de compilation", MB_ICONERROR );
		r.Close();
		exit(0);
	}
	r.Close();
}