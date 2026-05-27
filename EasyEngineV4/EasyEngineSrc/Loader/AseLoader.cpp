#include "Aseloader.h"


// stl
#include <string>
#include <vector>

// Engine
#include "../Utils2/EasyFile.h"
#include "../Utils2/StringUtils.h"

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/filereadstream.h"
#include <fstream>
#include <rapidjson/prettywriter.h>

using namespace rapidjson;


using namespace std;


CAseLoader::CAseLoader(void)
{	

}

CAseLoader::~CAseLoader(void)
{
}

void CAseLoader::Export(string sFileName, ILoader::IRessourceInfos& ri)
{
#ifdef OLD_VERSION
	string sExt;
	CStringUtils::GetExtension(sFileName, sExt);

	const CSceneInfos* pInfos = static_cast< const CSceneInfos* >(&ri);
	CAsciiFileStorage fs;
	fs.OpenFile(sFileName, IFileStorage::TOpenMode::eWrite);
	fs << *pInfos;
	fs.CloseFile();
#endif

#if 0
	const CSceneInfos* pInfos = static_cast< const CSceneInfos* >(&ri);

	Document doc;
	doc.SetObject();
	Value topics(kArrayType);

	Value header(kObjectType);
	Value sceneFileName(kStringType);
	sceneFileName.SetString(pInfos->m_sSceneFileName.c_str(), doc.GetAllocator());
	header.AddMember("sceneFileName", sceneFileName, doc.GetAllocator());
	Value originalSceneFileName(kStringType);
	originalSceneFileName.SetString(pInfos->m_sOriginalSceneFileName.c_str(), doc.GetAllocator());
	header.AddMember("originalSceneFileName", originalSceneFileName, doc.GetAllocator());
	Value name(kStringType);
	name.SetString(pInfos->m_sName.c_str(), doc.GetAllocator());
	header.AddMember("name", name, doc.GetAllocator());
	Value backgroundSolor(kStringType);
	name.SetString(pInfos->m_oBackgroundColor, doc.GetAllocator());
	header.AddMember("name", name, doc.GetAllocator());


	int nObjectCount = (int)m_vObject.size();
	store << m_sSceneFileName << m_sOriginalSceneFileName << m_sName << m_oBackgroundColor << m_bUseDisplacementMap << m_sDiffuseFileName << nObjectCount << m_nMapLength << m_fMapHeight;
	for (int i = 0; i < nObjectCount; i++)
		store << *m_vObject.at(i);
	//store << m_vObject;
	return *this;



	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	writer.SetIndent('\t', 1);
	doc.Accept(writer);
	std::ofstream ofs(sFileName);
	ofs << buffer.GetString();
	ofs.close();

#endif
}


//	Charge le tableau d'index 
void CAseLoader::LoadIndexArray( int nNumFace, vector< unsigned int >& vIndexArray )
{
	m_CurrentFile.SetPointerNext("*MESH_FACE_LIST");
	for ( int i = 0; i < nNumFace; i++ )
	{
		m_CurrentFile.SetPointerNext("A:");
		string sBuffer;
		m_CurrentFile.GetLine( sBuffer );
		vector< float > vFloat;
		CStringUtils::ExtractFloatFromString( sBuffer, vFloat, 3 );
		vIndexArray.push_back( (unsigned int) ( vFloat[ 0 ] ) );
		vIndexArray.push_back( (unsigned int) ( vFloat[ 1 ] ) );
		vIndexArray.push_back( (unsigned int) ( vFloat[ 2 ] ) );
	}
}

