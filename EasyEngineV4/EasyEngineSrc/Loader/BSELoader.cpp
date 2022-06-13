#include "BSELoader.h"
#include "IFileSystem.h"
#include "../Utils2/StringUtils.h"

CBSELoader::CBSELoader( IFileSystem& oFileSystem ):
m_oFileSystem( oFileSystem )
{
}

void CBSELoader::WriteAnimationSpeed(FILE* pFile, const map< string, float>& mapAnimationName)
{
	int animationCount = (int)mapAnimationName.size();
	fwrite(&animationCount, sizeof(int), 1, pFile);
	for (map< string, float>::const_iterator it = mapAnimationName.begin();	it != mapAnimationName.end(); it++) {
		int n = (int)it->first.size();
		fwrite(&n, sizeof(int), 1, pFile);
		fwrite(it->first.c_str(), sizeof(char), it->first.size(), pFile);
		fwrite(&it->second, sizeof(float), 1, pFile);
	}
}

void CBSELoader::ReadAnimationSpeed(FILE* pFile, map< string, float>& mapAnimationSpeed)
{
	int animationCount = 0;
	Read(&animationCount, sizeof(int), 1, pFile);
	for (int i = 0; i < animationCount; i++) {
		int n = 0;
		char animName[64];
		float speed = 0.f;
		Read(&n, sizeof(int), 1, pFile);
		Read(animName, sizeof(char), n, pFile);
		animName[n] = '\0';
		Read(&speed, sizeof(float), 1, pFile);
		string sAnimName = animName;
		mapAnimationSpeed[sAnimName] = speed;
	}
}

void CBSELoader::Export( string sFileName, IRessourceInfos& ri )
{
	string sExt;
	CStringUtils::GetExtension(sFileName, sExt);

	const CSceneInfos* pInfos = static_cast< const CSceneInfos* >( &ri );
	CBinaryFileStorage fs;
	fs.OpenFile(sFileName, IFileStorage::TOpenMode::eWrite);
	fs << *pInfos;
	fs.CloseFile();	
}

void CBSELoader::Load( string sFileName, IRessourceInfos& ri, IFileSystem& oFileSystem)
{
	CSceneInfos& si = static_cast< CSceneInfos&>(ri);
	CBinaryFileStorage fs;	
	if (!fs.OpenFile(sFileName, IFileStorage::TOpenMode::eRead)) {
		FILE* pFile = oFileSystem.OpenFile(sFileName, "r");
		if (pFile)
			fclose(pFile);
		string sDir;
		oFileSystem.GetLastDirectory(sDir);
		if (!fs.OpenFile(sDir + "/" + sFileName, IFileStorage::TOpenMode::eRead)) {
			CFileNotFoundException e(sFileName);
			throw e;
		}
	}
	fs >> si;
	fs.CloseFile();
}