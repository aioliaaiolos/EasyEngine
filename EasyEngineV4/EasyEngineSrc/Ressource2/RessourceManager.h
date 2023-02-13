#ifndef RESSOURCEMANAGER_CPP
#ifndef MESH_CPP
#error
#endif //  MESH_CPP
#endif // RESSOURCEMANAGER_CPP



#ifndef RESSOURCEMANAGER_H
#define RESSOURCEMANAGER_H

#include <windows.h>
#include <map>
#include <string>
#include "../Utils2/Chunk.h"
#include "IRessource.h"
#include "IRessource.h"
#include "ILoader.h"
#include "Texture.h"

using namespace std;

class IMeshLoader;
class ITextureLoader;
class CTextureBase;
class CMesh;
class CMaterial;
class CLight;
class CHierarchyMesh;
class CLoaderManager;
class IRenderer;
class ILoaderManager;
class ITestMesh;
class ITestShaderManager;
class IDrawTool;
class IEntityManager;
class ICollisionManager;
class IHeightMap;
struct CMaterialInfos;
class CTimeManager;

typedef unsigned int uint;

class CRessourceManager : public IRessourceManager
{
public:


						CRessourceManager(EEInterface& oInterface);
	virtual 			~CRessourceManager();
	void				EnableCatchingException( bool bEnable );
	bool				IsCatchingExceptionEnabled();
	void				SetCurrentRenderer( IRenderer* pRenderer );
	IRessource*			GetRessource( const std::string& sRessourceFileName, bool bDuplicate = false );
	ITexture*			CreateTexture2D(IShader* pShader, int nUnitTexture, vector< unsigned char >& vData, int nWidth, int nHeight, IRenderer::TPixelFormat eFormat );
	ITexture*			CreateTexture2D(string sFileName, bool bGenerateMipmaps);
	IAnimatableMesh*	CreateMesh(ILoader::CAnimatableMeshData& oData, IRessource* pMaterial = NULL);
	IMesh*				CreatePlane(int slices, int size, string diffuseTexture = "NONE") override;
	IMesh*				CreatePlane2(int slices, int size, float height, string heightTexture, string diffuseTexture) override;
	ITestMesh*			GetTestRessource( const std::string& sRessourceFileName, ITestShaderManager&);
	int					GetLightCount();
	void				SetDrawTool( IDrawTool* pDrawTool ){ m_pDrawTool = pDrawTool; }
	IRessource*			CreateLight( CVector Color, IRessource::TLight type, float fIntensity);
	void				SetLightIntensity( IRessource* pLight, float fIntensity );
	float				GetLightIntensity( IRessource* pRessource );
	CVector				GetLightColor( IRessource* pRessource );
	IRessource::TLight	GetLightType( IRessource* pRessource );
	void				PopErrorMessage( string& sMessage );
	void				DestroyAllRessources();
	ITexture*			CreateRenderTexture(int width, int height, string sShaderName);
	string				GetName() override;
	void				RemoveAllLights(IRenderer& oRenderer) override;
	void				Reset() override;

private:
	typedef IRessource*(*TRessourceCreation)(string sFileName, EEInterface& oInterface);
	typedef ITestMesh*(*TTestRessourceCreation)(string sFileName, CRessourceManager*, ITestShaderManager&, IRenderer&);
	EEInterface&									m_oInterface;
	std::map< std::string, IRessource* >			m_mRessource;
	ILoaderManager&									m_oLoaderManager;
	IRenderer&										m_oRenderer;
	IGeometryManager&								m_oGeometryManager;
	ICollisionManager*								m_pCollisionManager;
	IEntityManager*									m_pEntityManager;

	vector< string >								m_vErrorMessage;
	IRessource*										GetRessourceByExtension(std::string sRessourceFileName);
	IRessource*										CreateMaterial(ILoader::CMaterialInfos& mi, ITexture* pAlternative = NULL);
	IBone*											LoadSkeleton(ILoader::CAnimatableMeshData& oData);
	void											ComputeNormals(ILoader::CMeshInfos& mi, int slices, IHeightMap* pHeightMap);

	static IRessource*								CreateMesh(string sFileName, EEInterface& oInterface);
	static IRessource*								CreateCollisionMesh(string sFileName, EEInterface& oInterface);
	static IRessource*								CreateAnimation(string sFileName, EEInterface& oInterface);
	static IRessource*								CreateTexture(string sFileName, EEInterface& oInterface);
	static IRessource*								CreateLight(string sFileName, EEInterface& oInterface);
	static void										CollectMaterials(EEInterface& oInterface, const ILoader::CMaterialInfos& oMaterialInfos, IShader* pShader, std::map< int, CMaterial* >& vMaterials);
	static CMaterial*								CreateMaterial(EEInterface& oInterface, const ILoader::CMaterialInfos*, IShader* pShader);

	void											CreateTextureDesc(string sFileName, CTexture2D::CDesc& desc);
	std::map< std::string, TRessourceCreation >		m_mRessourceCreation;
	std::map< std::string, TTestRessourceCreation >	m_mTestRessourceCreation;
	int												m_nLightCount;
	IRenderer*										m_pCurrentRenderer;
	bool											m_bCatchException;

	IDrawTool*										m_pDrawTool;
};



extern "C" _declspec(dllexport) IRessourceManager* CreateRessourceManager(EEInterface& oInterface);
extern "C" _declspec(dllexport) CTimeManager* CreateTimeManager(EEInterface& oInterface);

#endif  //RESSOURCEMANAGER_H