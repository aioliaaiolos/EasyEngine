#include "IScriptManager.h"
#include "IFileSystem.h"
#include "AsmGenerator.h"

#include <vector>

class CLexAnalyser;
class CSyntaxAnalyser;
class CSemanticAnalyser;
class CAsmGenerator;
class CBinGenerator;
class CVirtualProcessor;

using namespace std;

class CScriptManager : public IScriptManager
{
public:
	CScriptManager(EEInterface& oInterface);
	~CScriptManager();
	void	RegisterFunction(std::string sFunctionName, ScriptFunction Function, const vector< TFuncArgType >& vArgsType, TFuncArgType returnType) override;
	void	ExecuteCommand(std::string sCommand) override;
	void	ExecuteByteCode(const vector<unsigned char>& vByteCode) override;
	void	GetRegisteredFunctions(vector< string >& vFuncNames) override;
	float	GetVariableValue(string variableName) override;
	float	GetRegisterValue(string sRegisterName) override;
	string	GetName() override;
	void	GenerateAssemblerListing(bool generate) override;
	void	Compile(string script, vector<unsigned char>& vByteCode);

private:
	CLexAnalyser*					m_pLexAnalyser;
	CSyntaxAnalyser*				m_pSyntaxAnalyser;
	CSemanticAnalyser*				m_pSemanticAnalyser;
	CAsmGenerator*					m_pAsmGenerator;
	CBinGenerator*					m_pBinGenerator;
	CVirtualProcessor*				m_pProc;
	map<string, CRegister::TType>	m_mRegisterFromName;
	bool							m_bGeneratedAssemblerListing;
	bool							m_bPutAllCodeInSameMemory;


};

extern "C" _declspec(dllexport) IScriptManager* CreateScriptManager(EEInterface& oInterface);