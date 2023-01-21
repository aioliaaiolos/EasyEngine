#ifndef EASY_FILE_EXCEPTION_H
#define EASY_FILE_EXCEPTION_H

#include <string>
#include <sstream>
#include <exception>

using namespace std;

class  CEException : public std::exception
{
protected:
	std::string			m_sMessage;
public:
	CEException( std::string sMessage ) : 
	std::exception( sMessage.c_str() ),
	m_sMessage( sMessage ){}
	virtual void GetErrorMessage( string& sMessage ){ sMessage = m_sMessage; }
	virtual void SetErrorMessage( string sMessage ){ m_sMessage = sMessage; }
};

class CTopicException : public CEException
{
public:
	CTopicException(string sMessage) : CEException(sMessage) {}
};

class CBadFileFormat : public CEException
{
public:
	CBadFileFormat( string sMessage ) : CEException( sMessage ){}
};

class CFileException : public CEException
{
public:
	string	m_sFileName;
	CFileException( const std::string& sMessage ) :  CEException( sMessage ){}
};

class  CEasyFileException : public CEException
{
public:
	CEasyFileException( const std::string& sMessage ) : CEException(sMessage){}
};

class  CEOFException : public CEasyFileException
{
public:
	CEOFException( const std::string& sMessage ) : CEasyFileException( sMessage ){}
};

class  CEOLException : public CEException
{
public:
	CEOLException( const std::string& sMessage ) : CEException( sMessage ){}
};

class  CFileNotFoundException : public CFileException
{
public:
	CFileNotFoundException( const std::string& sFileName ) : CFileException( sFileName )
	{
	}
};

class  CRenderException : public CEException
{
public:
	CRenderException( const std::string sMessage) : CEException( sMessage ){}
};

class  CRessourceException : public CEException
{
public:
	CRessourceException( std::string sMessage ) : CEException( sMessage ){}
};

class CExtensionNotFoundException : public CRessourceException
{
public:
	CExtensionNotFoundException( string sMessage ) : CRessourceException( sMessage ){}
};

class  CRessourceNotFoundException : public CRessourceException
{
public:
	CRessourceNotFoundException( const std::string& sRessourceName ) : CRessourceException( sRessourceName ){}
};

class  CXMLParserException : public CEException
{
public:
	CXMLParserException( const std::string& sXMLFileName ) : CEException( sXMLFileName ){}
};

class  CScriptException : public CEException
{
public:
	CScriptException( const std::string& sScriptException ) : CEException( sScriptException ){}
};

class CLineCompilationErrorException : public CScriptException
{
	int m_nColumn;
public:
	CLineCompilationErrorException(int nColumn):CScriptException( "" ),m_nColumn(nColumn){}
	int GetErrorColumn(){return m_nColumn;}
};

class CCompilationErrorException : public CScriptException
{
	int m_nLine;
	int m_nColumn;
public:
	CCompilationErrorException( int nLine, int nColumn ) : CScriptException( "" ), m_nLine( nLine ), m_nColumn( nColumn ){}
	int GetErrorColumn(){return m_nColumn;}
	int GetErrorLine(){return m_nLine;}
	void GetErrorMessage( string& sMessage )
	{
		ostringstream ossMessage;
		string sErrorType;
		if( m_sMessage == "" )
			sErrorType = "Erreur de syntaxe";
		else
			sErrorType = m_sMessage;
		ostringstream ossPosition;
		if( GetErrorLine() != -1 && GetErrorColumn() != -1 )
			ossPosition << " ligne " << GetErrorLine() << " colonne " << GetErrorColumn();
		ossMessage << sErrorType << ossPosition.str() << "\n";
		sMessage = ossMessage.str();
	}
};

class CBadArgCountException : public CCompilationErrorException
{
public:
	int m_nMinRequireArgCount;
	CBadArgCountException( int nMinRequireArgCount ) : CCompilationErrorException( -1, -1 ), m_nMinRequireArgCount(nMinRequireArgCount){}
};

class CRuntimeException : public CScriptException
{
public:
	CRuntimeException() : CScriptException( "" ){}
};

class CTerminateException : public CEException
{
public:
	CTerminateException(  const std::string& sScriptException ) : CEException( sScriptException ){}
};


class CWidgetNotInitialized : public CEException
{
public:
	CWidgetNotInitialized(  const std::string& sScriptException ) : CEException( sScriptException ){}
};

class CBadTypeException : public CEException
{
public:
	CBadTypeException() : CEException( "" ){}
	CBadTypeException( string sMessage ) : CEException( sMessage ){}
};

class CMethodNotImplementedException : public CEException
{
public:
	CMethodNotImplementedException(string message) : CEException(message) {}
};

class CNodeNotFoundException : public CEException
{
public:
	CNodeNotFoundException(string message) : CEException(message) {}
};

class CNoDummyRootException : public CEException
{
public:
	CNoDummyRootException(string message) : CEException(message) {}
};

class CCharacterAlreadyExistsException : public CEException
{
public:
	CCharacterAlreadyExistsException(string message) : CEException(message) {}
};

#endif