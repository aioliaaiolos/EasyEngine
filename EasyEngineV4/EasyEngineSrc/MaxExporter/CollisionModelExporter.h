#include "MaxExporter.h"
#include "ILoader.h"

class ICylinder;

class CollisionModelExporterClassDesc : public CMaxExporterClassDesc
{
	void*		Create(BOOL loading = FALSE);
	Class_ID	ClassID();
};

class CollisionModelExporter : public CMaxExporter
{
public:
						CollisionModelExporter();
	void				GetPrimitives(Interface* pInterface, vector<IGeometry*>& primitives);
	bool				DumpCollisionModel(const TCHAR* name, ILoader::CCollisionModelInfos& model);
	bool				isCylinder(Object* pObject);
	int					ExtCount();
	const TCHAR*		Ext(int n);
	int					DoExport(const TCHAR *name, ExpInterface *ei, Interface *pInterface, BOOL suppressPrompts, DWORD options);
	void				StoreCylinderInfos(INode* pMesh, ICylinder& cylinder);
	void				StoreBoxInfos(INode* pMesh, IBox& box);
	bool				StoreMeshInfos(INode* pMesh, IBox& box);

private:
	vector<wstring>		m_vExtension;

};