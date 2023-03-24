#ifndef MAX_EXPORTER_H
#define MAX_EXPORTER_H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "stdmat.h"
#include "decomp.h"
#include "shape.h"
#include "interpik.h"
#include "IGame.h"
#include "IGameModifier.h"

// math
#include "math/matrix.h"
#include "math/quaternion.h"

// stl
#include <vector>
#include <string>
#include <map>

// Engine
#include "ILoader.h"

using namespace std;

TCHAR* GetString(int id);

class IFileSystem;
class IGeometryManager;
class ILoaderManager;
class IWeightTable;

const int g_nTickPerFrame = 160;

struct CKey
{
	enum TKey{ eRotKey = 0, ePosKey };
	CKey() : m_nTimeValue( 0 ){}
	int					m_nTimeValue;
	CMatrix				m_oWorldTM;
	CMatrix				m_oLocalTM;
	AngAxis				m_oAngleAxis;
	CQuaternion			m_qLocal;
	TKey				m_eType;
};

// Class de base pour tous les plugins d'export 3DSMAX
class CMaxExporter : public SceneExport 
{
public:
	CMaxExporter();
	~CMaxExporter();

	// SceneExport methods
	virtual int			ExtCount() = 0;     // Number of extensions supported 
	virtual const 		TCHAR * Ext(int n) = 0;     // Extension #n (i.e. "ASC")
	const 				TCHAR * LongDesc();     // Long ASCII description (i.e. "Ascii Export") 
	const 				TCHAR * ShortDesc();    // Short ASCII description (i.e. "Ascii")
	const 				TCHAR * AuthorName();    // ASCII Author name
	const 				TCHAR * CopyrightMessage();   // ASCII Copyright message 
	const 				TCHAR * OtherMessage1();   // Other message #1
	const 				TCHAR * OtherMessage2();   // Other message #2
	unsigned int		Version();     // Version number * 100 (i.e. v3.01 = 301) 
	void				ShowAbout(HWND hWnd);  // Show DLL's "About..." box
	virtual int			DoExport(const TCHAR *name,ExpInterface *ei, Interface *i, BOOL suppressPrompts=FALSE, DWORD options = 0 ) = 0;
	void				ConvertPoint3ToCVector(const Point3& p, CVector& v);

	static std::vector<ClassDesc*>	m_vLibClassDesc;

protected:

	void				GetAnimation(Interface* pInterface, const std::map< int, INode* >& mBone, std::map< int, std::vector< CKey > >& vBones);
	Mesh&				GetMeshFromNode(INode* pMesh);
	void				GetVertexArrayFromMesh(Mesh& mesh, vector<float>& vertex);
	void				GetGeometry(Interface* pInterface, vector< ILoader::CMeshInfos >& vMeshInfos, INode* pRoot);
	void				GetMeshesIntoHierarchy(Interface* pInterface, INode* pNode, vector< ILoader::CMeshInfos >& vMeshInfos);
	void				WriteLog(string sMessage);
	void				GetWeightTable(IWeightTable& oWeightTable, const std::map< std::string, int >& mBoneID, string sObjectName, IGameNode* pGameNode = nullptr);
	void				GetBonesBoundingBoxes(const Mesh& oMesh, const IWeightTable& oWeightTable, const Matrix3& oModelTM, map< int, IBox* >& mBoneBox);
	void				StoreMaxMaterialToMaterialInfos(Mtl* pMaterial, ILoader::CMaterialInfos& mi);
	void				GetNormals(Mesh& oMesh, std::vector< float >& vFaceNormal, std::vector< float >& vVertexNormal);
	Point3				GetVertexNormal(Mesh& oMesh, int faceNo, RVertex* rv);
	void				GetFacesMtlArray(Mesh& oMesh, std::vector< unsigned short >& vMtlIDArray);
	void				StoreMaxColorToVector(const Color c, vector< float >& v);
	void				GetMaterialTextureName(Mtl* pMaterial, std::string& sTextureName, int nMapIndex)const;
	virtual void		StoreMeshToMeshInfos(Interface* pInterface, INode* pMesh, ILoader::CMeshInfos& mi) {}
	virtual void		GetSkeleton(INode* pRoot, map< string, INode* >& mBone);
	bool				IsCollisionMesh(string sObjectName);

	static bool			IsBone(Object* pObject);
	static bool			IsBone(INode* pNode);
	static void			MaxMatrixToEngineMatrix(const Matrix3& mMax, CMatrix& mEngine);
	static void			EngineMatrixToMaxMatrix(const CMatrix& oEngine, Matrix3& mMax);
	static INT_PTR CALLBACK OnExportAnim(HWND, UINT, WPARAM, LPARAM);

	vector< wstring >	m_vNonTCBBoneNames;
	bool				g_bInterruptExport;
	bool				m_bOpenglCoord;
	bool				m_bOpenglCoord2;
	IFileSystem*		m_pFileSystem;
	IGeometryManager*	m_pGeometryManager;
	ILoaderManager*		m_pLoaderManager;
	bool				m_bLog;
	bool				m_bExportSkinning;
	map< string, int >	m_mBoneIDByName;
	wstring				m_wVersion;
	CMatrix				m_oMaxToOpenglMatrix;
	bool				m_bExportBoundingBox;
	FILE*				m_pLogFile;
	map< int, IBox* >	m_mBoneBox;
	bool				m_bExportBBoxAtKey;
	int					m_nMaterialCount;
	bool				m_bMultipleSmGroup;
	bool				m_bFlipNormals;
	int					m_nCurrentSmGroup;
	TimeValue			m_nAnimationStart;
	TimeValue			m_nAnimationEnd;
	bool				m_bEnableAnimationList;
	HWND				m_hWndComboBox;
	vector<string>		m_vPathList;
	string				m_sSelectedAnimation;
	int					m_nSelectedAnimationIndex;
	Interface*			m_pMaxInterface;
	bool				m_bInitialise;
	static CMaxExporter* s_pExporter;
};

class CMaxExporterClassDesc : public ClassDesc 
{
public:
	CMaxExporterClassDesc();
	int					IsPublic();
	virtual void*		Create(BOOL loading = FALSE) = 0;
	const TCHAR*		ClassName();
	SClass_ID			SuperClassID();
	virtual Class_ID	ClassID() = 0;
	const TCHAR*		Category();
};


#endif // MAX_EXPORTER_H

