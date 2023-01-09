#include "Interface.h"
#include "ScriptManager.h"
#include "LexAnalyser.h"
#include "SyntaxAnalyser.h"
#include "SemanticAnalyser.h"
#include "AsmGenerator.h"
#include "BinGenerator.h"
#include "VirtualProcessor.h"
#include "Exception.h"

// stl
#include <chrono>

//#define CREATE_ASSEMBLING_LISTING

CScriptManager::CScriptManager(EEInterface& oInterface) :
m_bGeneratedAssemblerListing(false),
m_bPutAllCodeInSameMemory(false)
{
	IFileSystem* pFileSystem = static_cast<IFileSystem*>(oInterface.GetPlugin("FileSystem"));
	m_pLexAnalyser = new CLexAnalyser( "lexanalyser.csv", pFileSystem);
	m_pSyntaxAnalyser = new CSyntaxAnalyser;
	m_pSemanticAnalyser = new CSemanticAnalyser();
	m_pAsmGenerator = new CAsmGenerator(m_bPutAllCodeInSameMemory);
	m_pBinGenerator = new CBinGenerator(*m_pSyntaxAnalyser, *m_pAsmGenerator);
	m_pProc = new CVirtualProcessor( m_pSemanticAnalyser, m_pBinGenerator, m_bPutAllCodeInSameMemory);

	m_mRegisterFromName["eax"] = CRegister::eax;
	m_mRegisterFromName["ebx"] = CRegister::ebx;
	m_mRegisterFromName["ecx"] = CRegister::ecx;
	m_mRegisterFromName["edx"] = CRegister::edx;
	m_mRegisterFromName["esp"] = CRegister::esp;
	m_mRegisterFromName["ebp"] = CRegister::ebp;
	m_mRegisterFromName["esi"] = CRegister::esi;
	m_mRegisterFromName["edi"] = CRegister::edi;
	
}

CScriptManager::~CScriptManager()
{
	delete m_pLexAnalyser;
	delete m_pSyntaxAnalyser;
	delete m_pSemanticAnalyser;
	delete m_pAsmGenerator;
	delete m_pBinGenerator;
	delete m_pProc;
}

void CScriptManager::GenerateAssemblerListing(bool generate)
{
	m_bGeneratedAssemblerListing = generate;
}

void CScriptManager::Compile(string sScript, vector<unsigned char>& vByteCode)
{
	string s;
	vector< CLexem > vLexem;
	m_pLexAnalyser->GetLexemArrayFromScript(sScript, vLexem);
	
	if (vLexem.size() == 0)
		return;
	CSyntaxNode oTree;
	m_pSyntaxAnalyser->GetSyntaxicTree(vLexem, oTree);
	m_pSemanticAnalyser->CompleteSyntaxicTree(oTree, m_pSyntaxAnalyser->m_vFunctions);
	vector< CAsmGenerator::CInstr > vAssembler;
	map< string, int > mFuncAddr;
	m_pSemanticAnalyser->GetFunctionAddress(mFuncAddr);
	m_pAsmGenerator->GenAssembler(oTree, vAssembler, mFuncAddr, m_pSemanticAnalyser->GetVarMap());
	if (m_bGeneratedAssemblerListing) {
		m_pAsmGenerator->CreateAssemblerListing(vAssembler, "test.asm");
		for (int i = 0; i < m_pAsmGenerator->GetFunctions().size(); i++) {
			m_pAsmGenerator->CreateAssemblerListing(m_pAsmGenerator->GetFunctions()[i], string("test-function") + std::to_string(i) + ".asm");
		}
	}
	m_pBinGenerator->GenBinary(vAssembler, vByteCode);
}

void CScriptManager::ExecuteCommand( std::string sCommand )
{
	vector<unsigned char> vByteCode;
	Compile(sCommand, vByteCode);
	ExecuteByteCode(vByteCode);
}

void CScriptManager::ExecuteByteCode(const vector<unsigned char>& vByteCode)
{
	m_pProc->Execute(vByteCode, CBinGenerator::s_vInstrSize);
}

void CScriptManager::GetRegisteredFunctions( vector< string >& vFuncNames )
{
	m_pSemanticAnalyser->GetRegisteredFunctions( vFuncNames );
	for (const string& s : m_pSyntaxAnalyser->m_vFunctions)
		vFuncNames.push_back(s);
}

float CScriptManager::GetVariableValue(string variableName)
{
	return m_pProc->GetVariableValue(variableName);
}

float CScriptManager::GetRegisterValue(string sRegisterName)
{
	return m_pProc->GetRegisterValue(m_mRegisterFromName[sRegisterName]);
}

string CScriptManager::GetName()
{
	return "ScriptManager";
}

void CScriptManager::RegisterFunction( std::string sFunctionName, ScriptFunction Function, const vector< TFuncArgType >& vArgsType, TFuncArgType returnType)
{
	m_pSemanticAnalyser->RegisterFunction( sFunctionName, Function, vArgsType, returnType);
}

extern "C" _declspec(dllexport) IScriptManager* CreateScriptManager(EEInterface& oInterface)
{
	return new CScriptManager(oInterface);
}
