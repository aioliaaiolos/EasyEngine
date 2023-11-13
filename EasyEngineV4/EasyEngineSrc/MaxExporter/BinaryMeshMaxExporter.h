#include "MaxExporter.h"

// stl
#include <vector>
#include <string>
#include <map>

// Engine
#include "../Utils2/chunk.h"
#include "math/matrix.h"
#include "ILoader.h"
#include "IGeometry.h"

using namespace std;

class IRenderer;

class CBinaryMeshMaxExporterClassDesc : public CMaxExporterClassDesc
{
public:
	void*		Create(BOOL loading = FALSE);
	Class_ID	ClassID();
};


class CBinaryMeshMaxExporter : public CMaxExporter
{
	vector< std::wstring >				m_vExtension;
	static CBinaryMeshMaxExporter*		s_pCurrentInstance;
	
	
	void			GetBonesIDByName( INode* pRoot, std::map< std::string, int >& mBoneIDByName, map< string, int >& mDummyIDByName) const;
	void			GetBoneByID( const std::map< std::string, INode* >& mBoneByName, const std::map< std::string, int >& mBoneIDByName, const map< string, int >& mDummyIDByName, std::map< int, INode* >& mBoneByID );
	void			StoreSkinningToMeshInfos( const IWeightTable& wt, ILoader::CMeshInfos& mi );	
	void			StoreMeshToMeshInfos( Interface* pInterface, INode* pMesh, ILoader::CMeshInfos& mi ) override;
	void			StoreSkeletonToSkeletonMap( const map< int, INode* >& mNodeID, ILoader::CAnimatableMeshData& ami );
	bool			DumpModels( const std::string& sFilePath, ILoader::CAnimatableMeshData& ami );
	void			UpdateVersionFile( string sVersion );

	static INT_PTR CALLBACK ExportDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	

public:
	CBinaryMeshMaxExporter();
	~CBinaryMeshMaxExporter();
	int				ExtCount();
	const TCHAR*	Ext(int n);
	int				DoExport(const TCHAR *name,ExpInterface *ei, Interface *i, BOOL suppressPrompts=FALSE, DWORD options = 0 );
	
};

