#include "ILoadSaveAnimation.h"

#include "AnimationBBoxExporter.h"
#include "ILoader.h"
#include "IGeometry.h"

#include <algorithm>

#define ANIMATIONBBOXPLUGIN_CLASS_ID	Class_ID(0x85548e11, 0x4a264512)

void* CAnimationBBoxExporterClassDesc::Create(BOOL loading)
{
	return new CAnimationBBoxExporter;
}

Class_ID CAnimationBBoxExporterClassDesc::ClassID()
{
	return ANIMATIONBBOXPLUGIN_CLASS_ID;
}


CAnimationBBoxExporter::CAnimationBBoxExporter()
{
	m_vExtension.push_back(L"bbox");
}


CAnimationBBoxExporter::~CAnimationBBoxExporter()
{
}


int	CAnimationBBoxExporter::ExtCount()
{
	return (int)m_vExtension.size();
}

const TCHAR* CAnimationBBoxExporter::Ext(int n)
{
	return m_vExtension[n].c_str();
}



int	CAnimationBBoxExporter::DoExport(const TCHAR *name, ExpInterface *ei, Interface *pInterface, BOOL suppressPrompts, DWORD options)
{
	try
	{
		Interval interval = pInterface->GetAnimRange();
		m_nAnimationStart = pInterface->GetAnimRange().Start() / g_nTickPerFrame;
		m_nAnimationEnd = pInterface->GetAnimRange().End() / g_nTickPerFrame;
		m_bEnableAnimationList = true;

		if (DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ANIMEXPORT), pInterface->GetMAXHWnd(), OnExportAnim, (LPARAM)this) == 1)
		{
			wstring wFileName = name;
			string sFileName(wFileName.begin(), wFileName.end());
			WIN32_FIND_DATAA fd;
			HANDLE hFile = FindFirstFileA(sFileName.c_str(), &fd);
			if (hFile != INVALID_HANDLE_VALUE)
				CopyFileA(sFileName.c_str(), (sFileName + ".bak").c_str(), false);
			ILoader::CAnimatableMeshData mi;
			GetGeometry(pInterface, mi.m_vMeshes, pInterface->GetRootNode());
			if (mi.m_vMeshes.size() > 0) {
				ILoader::CAnimationBBoxInfos bboxInfos;
				bboxInfos.mKeyBoundingBoxes = mi.m_vMeshes[0].m_oKeyBoundingBoxes;
				m_pLoaderManager->Export(sFileName, bboxInfos);
				MessageBoxA(nullptr, "Export termine", "Succes", MB_OK);
			}
			else {
				MessageBoxA(nullptr, "Vous devez charger un modele et une animation", "Erreur", MB_ICONERROR);
			}
		}
	}
	catch (exception& e)
	{
		MessageBoxA(NULL, e.what(), "", MB_ICONERROR);
	}

	return TRUE;
}

void CAnimationBBoxExporter::StoreMeshToMeshInfos(Interface* pInterface, INode* pMesh, ILoader::CMeshInfos& mi)
{
	//const int fps = 30;
	for (int iFrame = m_nAnimationStart; iFrame <= m_nAnimationEnd; iFrame++) {
		Box3 box;
		//pMesh->ResetTransform(iFrame * 160, FALSE);
		pMesh->EvalWorldState(iFrame * 160).obj->GetDeformBBox(0, box);
		IBox* pBox = m_pGeometryManager->CreateBox();
		CVector dim = CVector(box.pmax.x - box.pmin.x, box.pmax.y - box.pmin.y, box.pmax.z - box.pmin.z);
		CVector vmin(-dim.m_x / 2.f, -dim.m_y / 2.f, -dim.m_z / 2.f);
		pBox->Set(vmin, dim);
		mi.m_oKeyBoundingBoxes[m_sSelectedAnimation].insert(map< int, IBox* >::value_type(iFrame * 160, pBox));
	}
}