#include "ColLoader.h"
#include "IGeometry.h"
#include "IFileSystem.h"

using namespace std;

CColLoader::CColLoader(IGeometryManager& oGeometryManager):
	m_oGeometryManager(oGeometryManager)
{
}


void CColLoader::Load(string sFileName, ILoader::IRessourceInfos& ri, IFileSystem& oFileSystem)
{
	ILoader::CCollisionModelInfos* pCmi = dynamic_cast<ILoader::CCollisionModelInfos*>(&ri);
	if (pCmi) {
		ILoader::CCollisionModelInfos& cmi = *pCmi;
		string sFilePath;
		FILE* pFile = oFileSystem.OpenFile(sFileName, "rb");
		if (pFile) {
			fclose(pFile);
			oFileSystem.GetLastDirectory(sFilePath);
			sFilePath += "\\" + sFileName;
		}
		CBinaryFileStorage fs;
		if (!fs.OpenFile(sFilePath, IFileStorage::eRead)) {
			CFileNotFoundException e(sFilePath);
			throw e;
		}
		int count = 0;
		fs >> count;
		for (int i = 0; i < count; i++) {
			int type = 0;
			fs >> type;

			IGeometry* pGeometry = NULL;
			switch ((IGeometry::Type)type) {
			case IGeometry::eCylinder:
				pGeometry = m_oGeometryManager.CreateCylinder();				
				break;
			case IGeometry::eBox:
				pGeometry = m_oGeometryManager.CreateBox();				
				break;
			}
			*pGeometry << fs;
			cmi.m_vPrimitives.push_back(pGeometry);
		}
		fs.CloseFile();
	}
}

void CColLoader::Export(string sFileName, ILoader::IRessourceInfos& ri)
{
	const ILoader::CCollisionModelInfos* pCmi = dynamic_cast<const ILoader::CCollisionModelInfos*>(&ri);
	if (pCmi) {
		const ILoader::CCollisionModelInfos& cmi = *pCmi;
		CBinaryFileStorage fs;
		if (fs.OpenFile(sFileName, IFileStorage::eWrite)) {
			fs << (int)cmi.m_vPrimitives.size();
			for (int i = 0; i < cmi.m_vPrimitives.size(); i++)
				(*cmi.m_vPrimitives[i]) >> fs;
			fs.CloseFile();
		}
		else{			
			CFileNotFoundException e(sFileName);
			throw e;
		}
	}
}

