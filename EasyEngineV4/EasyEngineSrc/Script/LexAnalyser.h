#ifndef LEXANALYSER_H
#define LEXANALYSER_H

#include <exception>
#include <string>
#include <vector>
#include <map>
using namespace std;

class IFileSystem;
class CCSVReader;

struct CLexem
{
	enum Type
	{
		eNone = -1,
		eVar = 0,
		eCall,
		eInt,
		eFloat,
		eLPar,
		eRPar,
		eAffect,
		eVirg,
		eDot,
		eGuill,
		eSub,
		eAdd,
		eDiv,
		eMult,
		ePtVirg,
		eString,
		eIdentifier,
		eFunctionDef,
		eLBraket,
		eRBraket,
		eIf,
		eComp,
		eSup,
		eInf
	};
	string			m_sValue;
	int				m_nValue;
	float			m_fValue;
	Type			m_eType;

	CLexem() : m_eType(eNone), m_nValue(0), m_fValue(0.f) {}
	CLexem(Type t) : m_eType(t), m_nValue(0), m_fValue(0.f) {}
	bool	IsOperation()const;
	bool	IsNumeric()const;
};


class CLexAnalyser
{
public:
	CLexAnalyser(string sCVSConfigName, IFileSystem*);
	void	GetLexemArrayFromScript(string sScript, vector< CLexem >& vLexem);
	
private:
	int									m_nStateCount;
	vector< vector< int > >				m_vAutomate;
	map< int, CLexem::Type >		m_mFinalStates;
	map< string, CLexem::Type >	m_mStringToLexemType;
	map<string, pair<int, int>>		m_mKeyWord;

	void					CalculStateCount( CCSVReader& r );
	void					CalculLexicalArrayFromCSV( string sCSVName, IFileSystem* pFS );
	void					CalculFinalStates( CCSVReader& r );
	void					CalculKeywords(CCSVReader& r);
	int						GenStringFromRegExpr(std::string sExpr, std::string& sOut);
	int						GenHookRegExpr(string sExpr, string& sOut);
	void					InitStringToLexemTypeArray();
	static void				ReadUntilEndComment(string sScript, unsigned int& startIndex, int& line);
	static void				ReadUntilEndLine(string sScript, unsigned int& startIndex);	
};

#endif // LEXANALYSER_H