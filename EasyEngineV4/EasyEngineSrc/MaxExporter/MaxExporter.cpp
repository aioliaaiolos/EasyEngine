#include "Interface.h"


#include "MaxExporter.h"
#include "3dsmaxport.h"
#include <sstream>
#include <algorithm>
#include "Utils2/StringUtils.h"
#include "Utils2/RenderUtils.h"

// Interface
#include "ILoader.h"
#include "IFileSystem.h"
#include "IGeometry.h"

using namespace std;

HINSTANCE hInstance;


// Class ID. These must be unique and randomly generated!!
// If you use this as a sample project, this is the first thing
// you should change!
//#define TESTINTERFACEPLUGIN_CLASS_ID	Class_ID(0x85548e0c, 0x4a26450d)


CMaxExporterClassDesc::CMaxExporterClassDesc()
{
}

int CMaxExporterClassDesc::IsPublic() 
{ 
	return 1; 
}

const TCHAR* CMaxExporterClassDesc::ClassName() 
{ 
	return L"MaxExporter"; 
}

SClass_ID CMaxExporterClassDesc::SuperClassID() 
{ 
	return SCENE_EXPORT_CLASS_ID; 
}

const TCHAR* CMaxExporterClassDesc::Category() 
{ 
	return _T("Category"); 
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;

	return NULL;
}

CMaxExporter* CMaxExporter::s_pExporter = nullptr;

CMaxExporter::CMaxExporter():
g_bInterruptExport( false ),
m_bOpenglCoord(true),
m_bOpenglCoord2(false),
m_bLog(false),
m_bExportSkinning(true),
m_bExportBoundingBox(true),
m_bExportBBoxAtKey(false),
m_bMultipleSmGroup(false),
m_bFlipNormals(false),
m_nCurrentSmGroup(-1),
m_nMaterialCount(0),
m_pLogFile(NULL),
m_bEnableAnimationList(false),
m_nSelectedAnimationIndex(0),
m_pMaxInterface(nullptr),
m_bInitialise(false)
{
	s_pExporter = this;
	EEInterface* pInterface = new EEInterface;
	CPlugin::SetEngineInterface(pInterface);
	IGeometryManager::Desc oGMDesc(NULL, "");
	m_pFileSystem = static_cast< IFileSystem* > (CPlugin::Create(*pInterface, "stdplugs\\EasyEngine\\FileUtils.dll", "CreateFileSystem"));
	m_pFileSystem->Mount("..\\data");
	m_pFileSystem->Mount("..\\..\\EasyEngine\\data");
	m_pGeometryManager = static_cast< IGeometryManager* >(CPlugin::Create(oGMDesc, "stdplugs\\EasyEngine\\Geometry.dll", "CreateGeometryManager"));
	m_pLoaderManager = static_cast< ILoaderManager* >(CPlugin::Create(*pInterface, "stdplugs\\EasyEngine\\Loader.dll", "CreateLoaderManager"));

	m_oMaxToOpenglMatrix = CMatrix(-1, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1);
}

CMaxExporter::~CMaxExporter()
{
}

const TCHAR * CMaxExporter::LongDesc()
{
	return _T("Easy engine export (BME for geometry and BKE for animation)");
}

const TCHAR * CMaxExporter::ShortDesc()
{
	return _T("Easy engine export");
}

const TCHAR * CMaxExporter::AuthorName() 
{
	return _T("Wladimir Perrad");
}

const TCHAR * CMaxExporter::CopyrightMessage() 
{
	return _T("Anti propriété intellectuelle");
}

const TCHAR * CMaxExporter::OtherMessage1()
{
	ostringstream ossVersion;
	ossVersion << __DATE__ << __TIME__;
	CStringUtils::ConvertStringToWString(ossVersion.str(), m_wVersion);
	return m_wVersion.c_str();
}

const TCHAR * CMaxExporter::OtherMessage2() 
{
	return _T("");
}

unsigned int CMaxExporter::Version()
{
	return 100;
}

static INT_PTR CALLBACK AboutBoxDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch (msg) {
	case WM_INITDIALOG:
		CenterWindow(hWnd, GetParent(hWnd)); 
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EndDialog(hWnd, 1);
			break;
		}
		break;
		default:
			return FALSE;
	}
	return TRUE;
}       

void CMaxExporter::ShowAbout(HWND hWnd)
{
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutBoxDlgProc, 0);
}

bool CMaxExporter::IsBone( Object* pObject )
{
	return ( pObject->CanConvertToType( Class_ID( BONE_CLASS_ID, 0 ) ) == TRUE || 
		pObject->CanConvertToType( BONE_OBJ_CLASSID ) == TRUE ||
		pObject->CanConvertToType( Class_ID( DUMMY_CLASS_ID, 0 ) ) == TRUE );
}

bool CMaxExporter::IsBone( INode* pNode )
{
	Object* pObject = pNode->EvalWorldState( 0 ).obj;
	if( !pObject )
		return false;
	return IsBone( pObject );
}

   

// Dummy function for progress bar
DWORD WINAPI fn(LPVOID arg)
{
	return(0);
}


void CMaxExporter::MaxMatrixToEngineMatrix( const Matrix3& mMax, CMatrix& mEngine )
{
	Point3& row0 = mMax.GetRow( 0 );
	mEngine.m_00 = row0.x;
	mEngine.m_10 = row0.y;
	mEngine.m_20 = row0.z;

	Point3& row1 = mMax.GetRow( 1 );
	mEngine.m_01 = row1.x;
	mEngine.m_11 = row1.y;
	mEngine.m_21 = row1.z;

	Point3& row2 = mMax.GetRow( 2 );
	mEngine.m_02 = row2.x;
	mEngine.m_12 = row2.y;
	mEngine.m_22 = row2.z;

	Point3& row3 = mMax.GetRow( 3 );
	mEngine.m_03 = row3.x;
	mEngine.m_13 = row3.y;
	mEngine.m_23 = row3.z;
}

void CMaxExporter::EngineMatrixToMaxMatrix( const CMatrix& oEngine, Matrix3& mMax )
{
	mMax.SetRow( 0, Point3( oEngine.m_00, oEngine.m_10, oEngine.m_20 ) );
	mMax.SetRow( 1, Point3( oEngine.m_01, oEngine.m_11, oEngine.m_21 ) );
	mMax.SetRow( 2, Point3( oEngine.m_02, oEngine.m_12, oEngine.m_22 ) );
	mMax.SetRow( 3, Point3( oEngine.m_03, oEngine.m_13, oEngine.m_23 ) );
}

Mesh& CMaxExporter::GetMeshFromNode(INode* pMesh)
{
	Object* pObject = pMesh->EvalWorldState(0).obj;
	TriObject* pTriObject = static_cast< TriObject* > (pObject->ConvertToType(0, Class_ID(TRIOBJ_CLASS_ID, 0)));
	return pTriObject->GetMesh();
}

void CMaxExporter::GetVertexArrayFromMesh(Mesh& mesh, vector<float>& vertex)
{
	Matrix3 oTM;
	mesh.getBoundingBox();
	for (int iVertex = 0; iVertex < mesh.getNumVerts(); iVertex++)
	{
		Point3& oVertex = oTM * mesh.verts[iVertex];
		vertex.push_back(oVertex.x);
		vertex.push_back(oVertex.y);
		vertex.push_back(oVertex.z);
	}
}

void CMaxExporter::GetAnimation( Interface* pInterface, const map< int, INode* >& mBone, map< int, vector< CKey > >& mBones )
{
	bool bTCB = false;
	
	//IGameScene* pGameScene = GetIGameInterface();
	//bool bInitialise = pGameScene->InitialiseIGame();

	CMatrix oWorldNodeTM, oParentTM, oInverseParentTM;
	//int nExportAnimationTimeStart = m_nExportAnimationStart * g_nTickPerFrame;
	//int nExportAnimationTimeEnd = m_nExportAnimationEnd * g_nTickPerFrame;

	for ( map< int, INode* >::const_iterator itNode = mBone.begin(); itNode != mBone.end(); ++itNode )
	{
		INode* pNode = itNode->second;
		Control* pControl = pNode->GetTMController()->GetRotationController();
		IKeyControl* pKc = GetKeyControlInterface( pControl );
		if ( pKc )
		{
			for ( int iKey = 0; iKey < pKc->GetNumKeys(); iKey++ )
			{
				ITCBRotKey key;
				pKc->GetKey( iKey, &key );
				
				Matrix3 m = pNode->GetNodeTM( key.time );
				MaxMatrixToEngineMatrix( m, oWorldNodeTM );
				if ( pNode->GetParentNode() )
				{
					Matrix3 mParentTM = pNode->GetParentNode()->GetNodeTM( key.time );
					MaxMatrixToEngineMatrix( mParentTM, oParentTM );
					oParentTM.GetInverse( oInverseParentTM );
				}
				CKey oKey;
				oKey.m_eType = CKey::eRotKey;
				oKey.m_nTimeValue = key.time;
				oKey.m_oWorldTM = oWorldNodeTM;
				oKey.m_oLocalTM = oInverseParentTM * oWorldNodeTM;
				oKey.m_oAngleAxis = key.val;
				oKey.m_oLocalTM.GetQuaternion( oKey.m_qLocal );
				mBones[ itNode->first ].push_back( oKey );
			}
			bTCB = true;
		}
		else
		{
			MSTR sClassName;
			pControl->GetClassName( sClassName );
			const wchar_t* sName = pNode->GetName();
			m_vNonTCBBoneNames.push_back( sName );
		}
		pControl = pNode->GetTMController()->GetPositionController();
		pKc = GetKeyControlInterface( pControl );
		if( pKc )
		{
			for ( int iKey = 0; iKey < pKc->GetNumKeys(); iKey++ )
			{
				ITCBPoint3Key key;
				pKc->GetKey( iKey, &key );
				
				Matrix3 m = pNode->GetNodeTM( key.time );
				MaxMatrixToEngineMatrix( m, oWorldNodeTM );
				if ( pNode->GetParentNode() )
				{
					Matrix3 mParentTM = pNode->GetParentNode()->GetNodeTM( key.time );
					MaxMatrixToEngineMatrix( mParentTM, oParentTM );
					oParentTM.GetInverse( oInverseParentTM );
				}
				CKey oKey;
				oKey.m_eType = CKey::ePosKey;
				oKey.m_nTimeValue = key.time;
				oKey.m_oWorldTM = oWorldNodeTM;
				oKey.m_oLocalTM = oInverseParentTM * oWorldNodeTM;
				oKey.m_oLocalTM.GetQuaternion( oKey.m_qLocal );
				map< int, vector< CKey > >::iterator itBone = mBones.find( itNode->first );
				bool bKeyExists = false;
				//if( itBone != mBones.end() )
				//{
				//	if( mBones[ itNode->first ].size() > iKey )
				//	{
				//		CMatrix& oLocal = mBones[ itNode->first ][ iKey ].m_oLocalTM;
				//		oLocal.SetAffinePart( 0.f, 0.f, 0.f );
				//		oLocal *= oKey.m_oLocalTM;
				//		bKeyExists = true;
				//	}
				//}
				//if( !bKeyExists )
					mBones[ itNode->first ].push_back( oKey );
			}
		}

	}
	if ( bTCB == false )
		MessageBoxA( NULL, "Votre animation ne comporte pas de controleurs TCB, elle n'a pas été exportée", "", MB_ICONWARNING );
}

void CMaxExporter::ConvertPoint3ToCVector(const Point3& p, CVector& v)
{
	v.m_x = p.x;
	v.m_y = p.y;
	v.m_z = p.z;
	v.m_w = 1;
}


void CMaxExporter::GetGeometry(Interface* pInterface, vector< ILoader::CMeshInfos >& vMeshInfos, INode* pRoot)
{
	for (int iNode = 0; iNode < pRoot->NumberOfChildren(); iNode++)
	{
		INode* pNode = pRoot->GetChildNode(iNode);
		int hidden = pNode->IsHidden();
		if (hidden == 0) {
			wstring wName = pNode->GetName();
			std::string sName(wName.begin(), wName.end());
			if (!IsCollisionMesh(sName)) {
				Object* pObject = pNode->EvalWorldState(0).obj;
				if (IsBone(pObject))
				{
					GetMeshesIntoHierarchy(pInterface, pNode, vMeshInfos);
					continue;
				}
				if (pObject->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)) == TRUE)
				{
					ILoader::CMeshInfos mi;
					StoreMeshToMeshInfos(pInterface, pNode, mi);
					if (g_bInterruptExport)
						break;
					vMeshInfos.push_back(mi);
				}
			}
		}
	}
}

void CMaxExporter::StoreMaxMaterialToMaterialInfos(Mtl* pMaterial, ILoader::CMaterialInfos& mi)
{
	m_nMaterialCount++;
	StoreMaxColorToVector(pMaterial->GetAmbient(), mi.m_vAmbient);
	StoreMaxColorToVector(pMaterial->GetDiffuse(), mi.m_vDiffuse);
	StoreMaxColorToVector(pMaterial->GetSpecular(), mi.m_vSpecular);
	mi.m_fShininess = pMaterial->GetShininess() * 128.f;
	wstring wName(pMaterial->GetName());
	string sName(wName.begin(), wName.end());
	mi.m_sName = sName;
	GetMaterialTextureName(pMaterial, mi.m_sDiffuseMapName, 1);
	int nMtlCount = 0, iMtl = 0;
	Mtl* pSubMtl = NULL;
	while (pSubMtl = pMaterial->GetSubMtl(iMtl++))nMtlCount++;
	mi.m_vSubMaterials.resize(nMtlCount);
	for (iMtl = 0; iMtl < nMtlCount; iMtl++)
	{
		MSTR sSlotName = pMaterial->GetSubMtlSlotName(iMtl);
		int nID = (int)(sSlotName[1] - '0');
		pSubMtl = pMaterial->GetSubMtl(iMtl);
		StoreMaxMaterialToMaterialInfos(pSubMtl, mi.m_vSubMaterials[iMtl]);
		mi.m_vSubMaterials[iMtl].m_nID = iMtl;
	}
}

void CMaxExporter::GetMaterialTextureName(Mtl* pMaterial, string& sTextureName, int nMapIndex) const
{
	Texmap* pTexmap = pMaterial->GetSubTexmap(nMapIndex);
	if (pTexmap)
	{
		BitmapTex* pTexture = (BitmapTex*)pTexmap;
		if (pTexture)
		{
			if (pTexture->GetMapName() == nullptr) {
				exception e("Texture invalide");
				throw e;
			}
			wstring wTexture(pTexture->GetMapName());
			string sTexture(wTexture.begin(), wTexture.end());
			sTextureName = sTexture;
			int nLastSlashPos = (int)sTextureName.find_last_of("\\");
			sTextureName = sTextureName.substr(nLastSlashPos + 1, sTextureName.size() - nLastSlashPos);
		}
	}
	if (sTextureName.size() == 0)
		sTextureName = "NONE";
}

void CMaxExporter::StoreMaxColorToVector(const Color c, vector< float >& v)
{
	v.push_back(c.r);
	v.push_back(c.g);
	v.push_back(c.b);
	v.push_back(1);
}

void CMaxExporter::GetBonesBoundingBoxes(const Mesh& oMesh, const IWeightTable& oWeightTable, const Matrix3& oModelTM, map< int, IBox* >& mBoneBox)
{
	for (int iVertex = 0; iVertex < oMesh.getNumVerts(); iVertex++)
	{
		Point3& oVertex = oModelTM * oMesh.verts[iVertex];
		map< int, float > mBoneWeights;
		oWeightTable.Get(iVertex, mBoneWeights);
		for (map< int, float >::const_iterator itBone = mBoneWeights.begin(); itBone != mBoneWeights.end(); itBone++)
		{
			if (itBone->second > 0.5f)
			{
				CVector v(oVertex.x, oVertex.y, oVertex.z);
				map< int, IBox* >::iterator itBox = mBoneBox.find(itBone->first);
				if (itBox == mBoneBox.end())
				{
					IBox* pBox = m_pGeometryManager->CreateBox();
					mBoneBox[itBone->first] = pBox;
				}
				mBoneBox[itBone->first]->AddPoint(v);
			}
		}
	}
}

void CMaxExporter::GetMeshesIntoHierarchy(Interface* pInterface, INode* pNode, vector< ILoader::CMeshInfos >& vMeshInfos)
{
	Object* pObject = pNode->EvalWorldState(0).obj;
	if (pObject) {
		if (IsBone(pObject))
		{
			for (int iBone = 0; iBone < pNode->NumberOfChildren(); iBone++)
				GetMeshesIntoHierarchy(pInterface, pNode->GetChildNode(iBone), vMeshInfos);
		}
		else
		{
			if (pObject->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)) == TRUE)
			{
				ILoader::CMeshInfos mi;
				StoreMeshToMeshInfos(pInterface, pNode, mi);
				vMeshInfos.push_back(mi);
			}
		}
	}
}

void CMaxExporter::WriteLog(string sMessage)
{
	if (m_bLog)
		fwrite(sMessage.c_str(), sizeof(char), sMessage.size(), m_pLogFile);
}

void CMaxExporter::GetSkeleton(INode* pRoot, map< string, INode* >& mBone)
{
	for (int iNode = 0; iNode < pRoot->NumberOfChildren(); iNode++)
	{
		INode* pNode = pRoot->GetChildNode(iNode);
		Object* pObject = pNode->EvalWorldState(0).obj;
		if (IsBone(pObject))
		{
			wstring wname(pNode->GetName());
			string name(wname.begin(), wname.end());
			mBone[name] = pNode;
		}
		GetSkeleton(pNode, mBone);
	}
}

bool CMaxExporter::IsCollisionMesh(string sObjectName)
{
	if ((sObjectName.find("Wall") == -1) &&
		(sObjectName.find("Door") == -1) &&
		(sObjectName.find("Roof") == -1) &&
		(sObjectName.find("Floor") == -1) &&
		(sObjectName.find("Ground") == -1) )
		return false;
	return true;
}

void CMaxExporter::GetWeightTable(IWeightTable& oWeightTable, const map< string, int >& mBoneID, string sObjectName, IGameNode* pGameNode)
{
	WriteLog("\nCBinaryMeshMaxExporter::GetWeightTable() : debut");
	IGameScene* pGameScene = GetIGameInterface();
	if(!m_bInitialise)
		m_bInitialise = pGameScene->InitialiseIGame();
	
	int nodeCount = 0;
	if (pGameNode) {
		INode* pRoot = m_pMaxInterface->GetRootNode();
		vector<ILoader::CMeshInfos> vMeshInfos;
		//GetMeshesIntoHierarchy(m_pMaxInterface, pRoot, vMeshInfos);
		//GetGeometry(m_pMaxInterface, vMeshInfos, pRoot);
	}
	
	nodeCount = pGameNode ? pGameNode->GetChildCount() : pGameScene->GetTopLevelNodeCount(); //pGameScene->GetTopLevelNodeCount();
	pGameScene->SetStaticFrame(0);
	for (int i = 0; i < nodeCount; i++)
	{
		IGameNode* pChildGameNode = nullptr;
		if (!pGameNode)
			pChildGameNode = pGameScene->GetTopLevelNode(i);
		else
			pChildGameNode = pGameNode->GetNodeChild(i);
		if (!pChildGameNode)
			continue;
		wstring wNodeName = pChildGameNode->GetName();
		wstring wObjectName(sObjectName.begin(), sObjectName.end());
		if (wNodeName != wObjectName) {
			GetWeightTable(oWeightTable, mBoneID, sObjectName, pChildGameNode);
			continue;
		}
		IGameObject* pGameObject = pChildGameNode->GetIGameObject();
		IGameSkin* pGameSkin = pGameObject->GetIGameSkin();
		if (pGameSkin)
		{
			if (pGameSkin->GetNumOfSkinnedVerts() == 0)
				MessageBoxA(nullptr, "Avertissement : le skin ne contient aucun vertex", "Pas de vertex dans le skin", MB_ICONWARNING);
			for (int iVertexIndex = 0; iVertexIndex < pGameSkin->GetNumOfSkinnedVerts(); iVertexIndex++)
			{
				int nBoneCount = 0;
				for (int iBoneIndex = 0; iBoneIndex < pGameSkin->GetNumberOfBones(iVertexIndex); iBoneIndex++)
				{
					INode* pBone = pGameSkin->GetBone(iVertexIndex, iBoneIndex);
					float fWeight = pGameSkin->GetWeight(iVertexIndex, iBoneIndex);
					if (fWeight > 0.01f)
					{
						wstring wName(pBone->GetName());
						string sName(wName.begin(), wName.end());
						map< string, int >::const_iterator itBone = mBoneID.find(sName);
						oWeightTable.Add(iVertexIndex, itBone->second, fWeight);
						nBoneCount++;
					}
				}
				if (nBoneCount > 4)
				{
					ostringstream oss;
					oss << "Erreur : le vertex " << iVertexIndex << " est influencé par plus de 4 bones";
					CEException e(oss.str());
					throw e;
				}
			}
			break;
		}
	}


	WriteLog("\nCBinaryMeshMaxExporter::GetWeightTable() : fin");
}


void CMaxExporter::GetNormals(Mesh& oMesh, std::vector< float >& vFaceNormal, std::vector< float >& vVertexNormal)
{
	vector<int> vIndex;
	float fNormalFactor = 1.f;
	if (m_bFlipNormals)
	{
		fNormalFactor = -1;
		vIndex.push_back(1);
		vIndex.push_back(0);
		vIndex.push_back(2);
	}
	else
	{
		vIndex.push_back(0);
		vIndex.push_back(1);
		vIndex.push_back(2);
	}
	CMatrix mTransform(-1, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1);
	for (int iFaceNormal = 0; iFaceNormal < oMesh.getNumFaces(); iFaceNormal++)
	{
		Point3& oNormal = oMesh.getFaceNormal(iFaceNormal);
		vFaceNormal.push_back(fNormalFactor * oNormal.x);
		vFaceNormal.push_back(fNormalFactor * oNormal.y);
		vFaceNormal.push_back(fNormalFactor * oNormal.z);
		Face& f = oMesh.faces[iFaceNormal];
		for (int vx = 0; vx < 3; vx++)
		{
			Point3& oVertexNormal = GetVertexNormal(oMesh, iFaceNormal, oMesh.getRVertPtr(f.getVert(vIndex[vx])));
			vVertexNormal.push_back(fNormalFactor * oVertexNormal.x);
			vVertexNormal.push_back(fNormalFactor * oVertexNormal.y);
			vVertexNormal.push_back(fNormalFactor * oVertexNormal.z);
		}
	}
}

Point3 CMaxExporter::GetVertexNormal(Mesh& oMesh, int faceNo, RVertex* rv)
{
	Face& f = oMesh.faces[faceNo];
	DWORD smGroup = f.smGroup;
	if (m_nCurrentSmGroup == -1)
		m_nCurrentSmGroup = smGroup;
	else
		if (m_nCurrentSmGroup != smGroup)
			m_bMultipleSmGroup = true;
	int numNormals = 0;
	Point3 vertexNormal;

	// Is normal specified
	// SPCIFIED is not currently used, but may be used in future versions.
	if (rv->rFlags & SPECIFIED_NORMAL)
		vertexNormal = rv->rn.getNormal();

	// If normal is not specified it's only available if the face belongs
	// to a smoothing group
	else if ((numNormals = rv->rFlags & NORCT_MASK) != 0 && smGroup)
	{
		// If there is only one vertex is found in the rn member.
		if (numNormals == 1)
		{
			vertexNormal = rv->rn.getNormal();
		}
		else
		{
			// If two or more vertices are there you need to step through them
			// and find the vertex with the same smoothing group as the current face.
			// You will find multiple normals in the ern member.
			for (int i = 0; i < numNormals; i++)
			{
				if (rv->ern[i].getSmGroup() & smGroup)
				{
					vertexNormal = rv->ern[i].getNormal();
				}
			}
		}
	}
	else
	{
		// Get the normal from the Face if no smoothing groups are there
		vertexNormal = oMesh.getFaceNormal(faceNo);
	}
	return vertexNormal;
}

void CMaxExporter::GetFacesMtlArray(Mesh& oMesh, std::vector< unsigned short >& vMtlIDArray)
{
	MtlID id0 = oMesh.faces[0].getMatID();
	for (int iFace = 0; iFace < oMesh.getNumFaces(); iFace++)
	{
		int nID = oMesh.faces[iFace].getMatID();
		vMtlIDArray.push_back(nID);
	}
}

#include <shlobj.h>


static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{

	if (uMsg == BFFM_INITIALIZED)
	{
		std::string tmp = (const char *)lpData;
		std::cout << "path: " << tmp << std::endl;
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}

	return 0;
}

std::string BrowseFolder(std::string saved_path)
{
	char path[MAX_PATH];

	const char * path_param = saved_path.c_str();

	BROWSEINFOA bi = { 0 };
	bi.lpszTitle = ("Browse for folder...");
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = BrowseCallbackProc;
	bi.lParam = (LPARAM)path_param;

	LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);

	if (pidl != 0)
	{
		//get the name of the folder and put it in path
		SHGetPathFromIDListA(pidl, path);

		//free memory used
		IMalloc * imalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&imalloc)))
		{
			imalloc->Free(pidl);
			imalloc->Release();
		}

		return path;
	}

	return "";
}

INT_PTR CALLBACK CMaxExporter::OnExportAnim(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	wchar_t pFirst[16], pLast[16];

	wstring wFirst, wLast, wText, wText2;
	string sFirst, sLast, sText, sText2, path, mask = "";
	BOOL findFile = TRUE;

	HWND hFirst = GetDlgItem(hWnd, ID_EDITFIRST);
	HWND hLast = GetDlgItem(hWnd, ID_EDITLAST);
	HWND hComboAnimList = GetDlgItem(hWnd, IDC_COMBOBOX_ANIMLIST);
	HWND hEditAnimName = GetDlgItem(hWnd, IDC_EDIT_ANIMNAME);
	ostringstream oss;
	BROWSEINFOA bi;
	WIN32_FIND_DATAA findFileData;

	switch (msg)
	{
	case WM_INITDIALOG:
		s_pExporter = (CMaxExporter*)lParam;
		CenterWindow(hWnd, GetParent(hWnd));
		oss << s_pExporter->m_nAnimationStart;
		sText = oss.str().c_str();
		wText.assign(sText.begin(), sText.end());
		SetWindowText(hFirst, wText.c_str());
		oss.str("");
		oss << s_pExporter->m_nAnimationEnd;
		sText2.assign(oss.str().c_str());
		wText2.assign(sText2.begin(), sText2.end());
		SetWindowText(hLast, wText2.c_str());		
		
		if (s_pExporter->m_bEnableAnimationList) {
			// s_pExporter->m_hWndComboBox = CreateWindow(WC_COMBOBOX, TEXT(""), CBS_SIMPLE | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 20, 121, 150, 2000, hWnd, NULL, (HINSTANCE)GetWindowLongW(hWnd, 0), NULL);
			s_pExporter->m_hWndComboBox = CreateWindow(WC_COMBOBOX, TEXT(""), CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, 20, 121, 150, 200, hWnd, NULL, (HINSTANCE)GetWindowLongW(hWnd, 0), NULL);

			//path = BrowseFolder("C:/Users/wladi/Documents/Developpement/EasyEngine2/trunk/EasyEngineV4/EasyEngine/Data/Animations");
			//path = "C:/Users/wladi/Documents/Developpement/EasyEngine2/trunk/EasyEngineV4/EasyEngine/Data/Animations";
			path = "D:/Developpement/EasyEngine-git/trunk/EasyEngineV4/EasyEngine/Data/Animations";
			if (path != "") {
				path = path + "/*.bke";
				HANDLE hFile = FindFirstFileA(path.c_str(), &findFileData);
				while (findFile) {
					string animName = findFileData.cFileName;
					animName = animName.substr(0, animName.find("."));
					string animNameLower = animName;
					std::transform(animName.begin(), animName.end(), animNameLower.begin(), tolower);
					SendMessageA(s_pExporter->m_hWndComboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)animNameLower.c_str());
					s_pExporter->m_vPathList.push_back(animNameLower);
					findFile = FindNextFileA(hFile, &findFileData);
				}
				SendMessage(s_pExporter->m_hWndComboBox, CB_SETCURSEL, (WPARAM)0, (LPARAM)s_pExporter->m_nSelectedAnimationIndex);
				s_pExporter->m_nSelectedAnimationIndex = SendMessageA(s_pExporter->m_hWndComboBox, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				s_pExporter->m_sSelectedAnimation = s_pExporter->m_vPathList[s_pExporter->m_nSelectedAnimationIndex];
			}
		}
		
		break;
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		break;
	case WM_COMMAND:
		if (lParam == (LPARAM)s_pExporter->m_hWndComboBox) {
			s_pExporter->m_nSelectedAnimationIndex = SendMessageA(s_pExporter->m_hWndComboBox, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
			if (s_pExporter->m_nSelectedAnimationIndex >= 0) {
				s_pExporter->m_sSelectedAnimation = s_pExporter->m_vPathList[s_pExporter->m_nSelectedAnimationIndex];
				SetWindowTextA(hEditAnimName, s_pExporter->m_sSelectedAnimation.c_str());
			}
		}
		switch (wParam)
		{
		case IDEXPORTANIM:
			GetWindowText(hFirst, pFirst, 16);
			GetWindowText(hLast, pLast, 16);

			wFirst = pFirst;
			wLast = pLast;
			sFirst.assign(wFirst.begin(), wFirst.end());
			sLast.assign(wLast.begin(), wLast.end());
			s_pExporter->m_nAnimationStart = atoi(sFirst.c_str());
			s_pExporter->m_nAnimationEnd = atoi(sLast.c_str());
			EndDialog(hWnd, 1);
			break;
		case IDCANCELANIM:
			//g_pExporter->m_bCancelExport = true;
			EndDialog(hWnd, 0);
			break;
		}
		break;
	}
	return 0;
}
