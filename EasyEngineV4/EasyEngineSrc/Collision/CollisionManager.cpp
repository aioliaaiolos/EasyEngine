#include "CollisionManager.h"
#include "IRessource.h"
#include "IShader.h"
#include "ILoader.h"
#include "HeightMap.h"
#include "IGeometry.h"
#include "Utils2/RenderUtils.h"
#include "IEntity.h"
#include "Interface.h"
#include "IFileSystem.h"
#include <set>

IMesh* CCollisionManager::s_pMesh = NULL;

CCollisionManager* g_pCurrentCollisionManager = NULL;

CCollisionManager::CCollisionManager(EEInterface& oInterface) :
m_oInterface(oInterface),
m_oRenderer(static_cast<IRenderer&>(*m_oInterface.GetPlugin("Renderer"))),
m_oLoaderManager(static_cast<ILoaderManager&>(*m_oInterface.GetPlugin("LoaderManager"))),
m_pFileSystem(static_cast<IFileSystem*>(m_oInterface.GetPlugin("FileSystem"))),
m_oGeometryManager(static_cast<IGeometryManager&>(*m_oInterface.GetPlugin("GeometryManager"))),
m_oEntityManager(static_cast<IEntityManager&>(*m_oInterface.GetPlugin("EntityManager"))),
m_nHeightMapPrecision( 1 ),
m_pGround(NULL),
m_sCustomName("a"),
m_fCustomValue(0.f),
m_fGroundWidth(0),
m_fGroundHeight(0),
m_fGridHeight(800.f),
m_pCollisionGrid(NULL),
m_pScene(NULL),
m_fScreenRatio(m_fScreenRatio),
m_pEntityManager(NULL),
m_bEnableHMHack(false),
m_bEnableHMHack2(false)
{
	g_pCurrentCollisionManager = this;
	unsigned int nScreenWidth, nScreenHeight;
	m_oRenderer.GetResolution(nScreenWidth, nScreenHeight);
	m_fScreenRatio = (float)nScreenWidth / (float)nScreenHeight;
	
}

void CCollisionManager::DisplayHeightMap(IMesh* pMesh)
{
	m_oRenderer.GetBackgroundColor(m_oOriginBackgroundColor);
	ILoader::CTextureInfos ti;
	CreateHeightMap(pMesh, ti);
	s_pMesh = pMesh;
	m_oRenderer.AbonneToRenderEvent(OnRenderHeightMap);
}

void CCollisionManager::StopDisplayHeightMap()
{
	m_oRenderer.DesabonneToRenderEvent(OnRenderHeightMap);
	m_oRenderer.SetBackgroundColor(m_oOriginBackgroundColor.m_x, m_oOriginBackgroundColor.m_y, m_oOriginBackgroundColor.m_z);
}

void CCollisionManager::DisplayCollisionMap()
{
	m_oRenderer.AbonneToRenderEvent(OnRenderCollisionMapCallback);
}

void CCollisionManager::StopDisplayCollisionMap()
{
	m_oRenderer.DesabonneToRenderEvent(OnRenderCollisionMapCallback);
}

void CCollisionManager::OnRenderCollisionMapCallback(IRenderer*)
{
	g_pCurrentCollisionManager->OnRenderCollisionMap();
}

void CCollisionManager::RenderCollisionGeometry(IShader* pCollisionShader, const CMatrix& groundModel, const IBox* const pBox)
{
	pCollisionShader->SendUniformValues("h", pBox->GetDimension().m_y);
	pCollisionShader->SendUniformValues("zMin", pBox->GetMinPoint().m_y);
	pCollisionShader->SendUniformValues("scale", m_fWorldToScreenScaleFactor);
	pCollisionShader->SendUniformMatrix4("modelMatrix", groundModel, true);
	pCollisionShader->SendUniformValues("isGround", 1.f);
	pCollisionShader->SendUniformValues("isGrid", 0.f);
	pCollisionShader->SendUniformValues(m_sCustomName, m_fCustomValue);

	m_pGround->Update();

	pCollisionShader->SendUniformValues("isGround", 0.f);
	for (vector<IEntity*>::iterator it = m_vCollideObjects.begin(); it != m_vCollideObjects.end(); it++) {
		const CMatrix& model = (*it)->GetWorldMatrix();
		pCollisionShader->SendUniformMatrix4("modelMatrix", model, true);
		(*it)->Update();
	}

	pCollisionShader->SendUniformValues("isGround", 0.f);
	pCollisionShader->SendUniformValues("isGrid", 1.f);
	for (vector<IEntity*>::iterator it = m_vGridElements.begin(); it != m_vGridElements.end(); it++) {
		const CMatrix& model = (*it)->GetWorldMatrix();
		pCollisionShader->SendUniformMatrix4("modelMatrix", model, true);
		(*it)->Update();
	}
	
}

void CCollisionManager::OnRenderCollisionMap()
{
	IShader* pOrgShader = m_pGround->GetShader();
	IShader* pCollisionShader = m_oRenderer.GetShader("collision");
	pCollisionShader->Enable(true);
	m_pGround->SetShader(pCollisionShader);

	CMatrix model, oProj;
	unsigned int nWidth, nHeight;
	m_oRenderer.GetResolution(nWidth, nHeight);
	oProj.m_00 = (float)nHeight / (float)nWidth;

	vector<IShader*> vBackStaticObjectShader;
	vector<IShader*> vBackLineShader;

	GetOriginalShaders(m_vCollideObjects, vBackStaticObjectShader);
	SetCollisionShaders(m_vCollideObjects, pCollisionShader);

	GetOriginalShaders(m_vGridElements, vBackLineShader);
	SetCollisionShaders(m_vGridElements, pCollisionShader);

	CMatrix oBakProj;
	m_oRenderer.GetProjectionMatrix(oBakProj);
	m_oRenderer.SetProjectionMatrix(oProj);

	bool wasCullingEnabled = m_oRenderer.IsCullingEnabled();
	if (wasCullingEnabled)
		m_oRenderer.CullFace(false);
	m_oRenderer.EnableDepthTest(false);

	RenderCollisionGeometry(pCollisionShader, model, m_pGround->GetBBox());

	m_oRenderer.EnableDepthTest(true);

	if (wasCullingEnabled)
		m_oRenderer.CullFace(true);

	pCollisionShader->Enable(false);
	m_pGround->SetShader(pOrgShader);

	RestoreOriginalShaders(vBackStaticObjectShader, m_vCollideObjects);
	RestoreOriginalShaders(vBackLineShader, m_vGridElements);

	m_oRenderer.SetProjectionMatrix(oBakProj);
}

void CCollisionManager::ComputeGroundMapDimensions(IMesh* pMesh, int nScreenWidth, int nScreenHeight, float& width, float& height, float& groundToScreenScaleFactor)
{
	IBox* pBox = pMesh->GetBBox();
	if (pBox->GetDimension().m_x <= pBox->GetDimension().m_z * m_fScreenRatio)
	{
		width = (pBox->GetDimension().m_x * (float)nScreenHeight) / pBox->GetDimension().m_z;
		height = (float)nScreenHeight;
		groundToScreenScaleFactor = pBox->GetDimension().m_z / 2.f;
	}
	else
	{
		width = (float)nScreenWidth;
		height = pBox->GetDimension().m_z * (float)nScreenWidth / pBox->GetDimension().m_x;
		groundToScreenScaleFactor = pBox->GetDimension().m_x / 2.f;
	}
	width = (float)(((int)width / 4) * 4);
	height = ((int)height / 4) * 4;
}

int CCollisionManager::ComputeOptimalCellSize(int nCellSize, const CDimension& modelDimension)
{
	int nMapWidth = modelDimension.GetWidth() / nCellSize;
	nMapWidth = nMapWidth % 4 ? (nMapWidth >> 2) << 2 : nMapWidth;
	int nOptimalCellSize = modelDimension.GetWidth() / nMapWidth;
	return nOptimalCellSize;
}

void CCollisionManager::CreateCollisionMap(string sFileName, IEntity* pRoot, int nCellSize, float fBias)
{
	vector<vector<bool>> vGrid;
	IBox* pBox = (IBox*)pRoot->GetBoundingGeometry();
	CreateCollisionArray(pRoot, vGrid, nCellSize, fBias);
	CreateTextureFromCollisionArray(sFileName, vGrid);
}

void CCollisionManager::CreateCollisionMapByRendering(ILoader::CTextureInfos& ti, vector<IEntity*> collides, IEntity* pScene, IRenderer::TPixelFormat format)
{
	m_pScene = pScene;

	m_pGround = dynamic_cast<IMesh*>(pScene->GetRessource());
	if (!m_pGround) {
		CEException e("Erreur, aucun mesh défini pour la scene");
		throw e;
	}
	
	IShader* pOrgShader = m_pGround->GetShader();
	IShader* pCollisionShader = m_oRenderer.GetShader("collision");
	pCollisionShader->Enable(true);
	m_pGround->SetShader(pCollisionShader);

	CMatrix model, oProj;
	oProj.m_00 = 1.f / m_fScreenRatio;
	const IBox* pBox = m_pGround->GetBBox();

	unsigned int nScreenWidth, nScreenHeight;
	m_oRenderer.GetResolution(nScreenWidth, nScreenHeight);
	ComputeGroundMapDimensions(m_pGround, nScreenWidth, nScreenHeight, m_fGroundMapWidth, m_fGroundMapHeight, m_fWorldToScreenScaleFactor);
	int subdivisionCount = 30;

	int cellMapSize = m_fGroundMapWidth / subdivisionCount;

	float fOriginMapX = ((float)nScreenWidth - m_fGroundMapWidth) / 2.f;
	float fOriginMapY = ((float)nScreenHeight - m_fGroundMapHeight) / 2.f;

	m_fGroundWidth = pBox->GetDimension().m_x;
	m_fGroundHeight = pBox->GetDimension().m_z;

	m_vCollideObjects = collides;

	vector<IShader*> vBackStaticObjectShader;
	vector<IShader*> vBackLineShader;

	GetOriginalShaders(m_vCollideObjects, vBackStaticObjectShader);
	SetCollisionShaders(m_vCollideObjects, pCollisionShader);

	GetOriginalShaders(m_vGridElements, vBackLineShader);
	SetCollisionShaders(m_vGridElements, pCollisionShader);

	CMatrix oBakProj;
	m_oRenderer.GetProjectionMatrix(oBakProj);
	m_oRenderer.SetProjectionMatrix(oProj);

	bool wasCullingEnabled = m_oRenderer.IsCullingEnabled();
	if (wasCullingEnabled)
		m_oRenderer.CullFace(false);
	m_oRenderer.EnableDepthTest(false);

	m_oRenderer.BeginRender();

	RenderCollisionGeometry(pCollisionShader, model, pBox);

	m_oRenderer.ReadPixels(fOriginMapX, fOriginMapY, m_fGroundMapWidth, m_fGroundMapHeight, ti.m_vTexels, format);
	ti.m_nWidth = (int)m_fGroundMapWidth;
	ti.m_nHeight = (int)m_fGroundMapHeight;

	m_oRenderer.EndRender();

	m_oRenderer.EnableDepthTest(true);

	if (wasCullingEnabled)
		m_oRenderer.CullFace(true);

	pCollisionShader->Enable(false);
	m_pGround->SetShader(pOrgShader);

	RestoreOriginalShaders(vBackStaticObjectShader, m_vCollideObjects);
	RestoreOriginalShaders(vBackLineShader, m_vGridElements);

	m_oRenderer.SetProjectionMatrix(oBakProj);
}

void CCollisionManager::LoadCollisionMapComputedByRendering(string sFileName, IEntity* pScene, ILoader::CTextureInfos& collisionMap)
{
	m_oLoaderManager.Load(sFileName, collisionMap);
	IMesh* pGround = dynamic_cast<IMesh*>(pScene->GetRessource());
	m_pGround = pGround;
	m_pScene = pScene;
	if (m_pGround) {
		unsigned int nScreenWidth, nScreenHeight;
		m_oRenderer.GetResolution(nScreenWidth, nScreenHeight);
		ComputeGroundMapDimensions(m_pGround, nScreenWidth, nScreenHeight, m_fGroundMapWidth, m_fGroundMapHeight, m_fWorldToScreenScaleFactor);
	}
}

void CCollisionManager::AttachCollisionMapToScene(const ILoader::CTextureInfos& oCollisionMap, IEntity* pScene)
{
	IMesh* pGround = dynamic_cast<IMesh*>(pScene->GetRessource());
	m_pGround = pGround;
	m_pScene = pScene;
	if (m_pGround) {
		IBox* pBox = m_pGround->GetBBox();
		m_fGroundWidth = pBox->GetDimension().m_x;
		m_fGroundHeight = pBox->GetDimension().m_z;
	}
}

void CCollisionManager::SetHeightMapPrecision( int nPrecision )
{
	m_nHeightMapPrecision = nPrecision;
}

void CCollisionManager::CreateHeightMap( IMesh* pGround, ILoader::CTextureInfos& ti, IRenderer::TPixelFormat format )
{
	if (!m_pGround)
		m_pGround = pGround;
 	IShader* pOrgShader = pGround->GetShader();
	IShader* pHMShader = NULL;
	if(!m_bEnableHMHack && !m_bEnableHMHack2)
		pHMShader = m_oRenderer.GetShader("hm");
	else
		pHMShader = m_oRenderer.GetShader("hmHack");
	pHMShader->Enable(true);
	pGround->SetShader(pHMShader);

	CMatrix oModelView, oProj;
	unsigned int nWidth, nHeight;
	m_oRenderer.GetResolution(nWidth, nHeight);
	float fScreenRatio = (float)nWidth / (float)nHeight;
	oProj.m_00 = 1.f / fScreenRatio;
	const IBox* pBox = pGround->GetBBox();
	float maxLenght = pBox->GetDimension().m_x;
	if (maxLenght < pBox->GetDimension().m_z)
		maxLenght = pBox->GetDimension().m_z;
	
	unsigned int nScreenWidth, nScreenHeight;
	m_oRenderer.GetResolution(nScreenWidth, nScreenHeight);
	ComputeGroundMapDimensions(pGround, nScreenWidth, nScreenHeight, m_fGroundMapWidth, m_fGroundMapHeight, m_fWorldToScreenScaleFactor);
	if(m_bEnableHMHack)
		m_fWorldToScreenScaleFactor *= 1.70068f;
	else if (m_bEnableHMHack2) {
		m_fWorldToScreenScaleFactor *= 2.55103f;
	}
	float fOriginMapX = ((float)nWidth - m_fGroundMapWidth) / 2.f;
	float fOriginMapY = ((float)nHeight - m_fGroundMapHeight) / 2.f;

	pHMShader->SendUniformValues("h", pBox->GetDimension().m_y);
	float zmin = pBox->GetMinPoint().m_y;
	if (m_bEnableHMHack) {
		zmin += 210.f; // temporary hack
	}
	else if (m_bEnableHMHack2) {
		static float value = 210.f;
		zmin += value; // temporary hack
	}
	pHMShader->SendUniformValues("zMin", zmin);
	pHMShader->SendUniformValues("scale", m_fWorldToScreenScaleFactor);

	CMatrix oBakProj;
	m_oRenderer.GetProjectionMatrix(oBakProj);
	m_oRenderer.SetProjectionMatrix(oProj);

	m_oRenderer.CullFace(false);
	m_oRenderer.BeginRender();
	pGround->Update();

	m_oRenderer.ReadPixels(fOriginMapX, fOriginMapY, m_fGroundMapWidth, m_fGroundMapHeight, ti.m_vTexels, format);
	ti.m_nWidth = (int)m_fGroundMapWidth;
	ti.m_nHeight = (int)m_fGroundMapHeight;
	m_oRenderer.EndRender();
	m_oRenderer.CullFace(true);

	pHMShader->Enable(false);
	pGround->SetShader(pOrgShader);
	m_oRenderer.SetProjectionMatrix(oBakProj);
}

void CCollisionManager::CreateHeightMap(string sFileName)
{
	if (sFileName.find(".bme") == -1)
		sFileName += ".bme";
	IEntity* pEntity = NULL;
	try
	{
		pEntity = m_pEntityManager->CreateEntity(sFileName, "");
	}
	catch (CEException& e)
	{
		string sError;
		e.GetErrorMessage(sError);
		string s = string("Erreur : ") + sError;
		throw e;
	}
	if (pEntity)
	{
		IMesh* pMesh = dynamic_cast< IMesh* >(pEntity->GetRessource());
		if (pMesh)
		{
			IMesh* pGround = static_cast<IMesh*>(m_pScene->GetRessource());
			string sSceneFileName;
			pGround->GetFileName(sSceneFileName);
			if (sSceneFileName == sFileName)
				EnableHMHack(true);
			else
				EnableHMHack2(true);
			ILoader::CTextureInfos ti;
			CreateHeightMap(pMesh, ti, IRenderer::T_BGR);
			ti.m_ePixelFormat = ILoader::eBGR;
			string sTextureFileName = sFileName.substr(0, sFileName.find('.'));
			sTextureFileName = string("HM_") + sTextureFileName + ".bmp";
			m_oLoaderManager.Export(sTextureFileName, ti);
		}
	}
}

void CCollisionManager::CreateHeightMapWithoutRender(string sFileName)
{
	ILoader::CTextureInfos ti;
	ILoader::CAnimatableMeshData ami;
	m_oLoaderManager.Load(sFileName, ami);
	ILoader::CMeshInfos& mi = ami.m_vMeshes[0];
	CVector dim = mi.m_pBoundingBox->GetDimension();
	int nCellCount = dim.m_x * dim.m_z;
	int quadCount = mi.m_vIndex.size() / 6;
	set<float> setx;
	set<float> sety;
	set<float> setz;
	map<pair<float, float>, float> mxz;
	float ymin = mi.m_vVertex[1];
	float ymax = ymin;
	for (int i = 0; i < mi.m_vVertex.size() / 3; i++) {
		float x = mi.m_vVertex[3 * i];
		float y = mi.m_vVertex[3 * i + 1];
		float z = mi.m_vVertex[3 * i + 2];
		setx.insert(x);
		sety.insert(y);
		setz.insert(z);
		mxz[pair<float, float>(x, z)] = y;
		if (ymin > y)
			ymin = y;
		if (ymax < y)
			ymax = y;
	}
	float d = dim.m_x / setx.size();
	int width = 64; // setx.size() + 1;
	int height = 64; // setz.size() + 1;
	ti.m_vTexels.resize(3 * width * height);
	memset(&ti.m_vTexels[0], 0, ti.m_vTexels.size());

	
	for (map<pair<float, float>, float>::iterator it = mxz.begin(); it != mxz.end(); it++) {
		float x = it->first.first;
		float y = it->second;
		float z = it->first.second;
		x += dim.m_x / 2.f;
		z += dim.m_z / 2.f;
		int pixelx = x / d;
		int pixelz = z / d;
		int pixelIndex = pixelx + pixelz * setx.size();
		if (pixelIndex < ti.m_vTexels.size()/3) {
			ti.m_vTexels[3 * pixelIndex] = 255 * (0.5f + y / dim.m_y);
			ti.m_vTexels[3 * pixelIndex + 1] = 255 * (0.5f + y / dim.m_y);
			ti.m_vTexels[3 * pixelIndex + 2] = 255 * (0.5f + y / dim.m_y);
		}
	}
	ti.m_nWidth = width;
	ti.m_nHeight = height;
	ti.m_ePixelFormat = ILoader::eBGR;
	string groundName = sFileName.substr(0, sFileName.find('.'));
	string textureFileName = "HM_" + groundName + "_test.bmp";
	m_oLoaderManager.Export(textureFileName, ti);
}

void CCollisionManager::GetOriginalShaders(const vector<IEntity*>& staticObjects, vector<IShader*>& vBackupStaticObjectShader)
{
	for (vector<IEntity*>::const_iterator it = staticObjects.begin(); it != staticObjects.end(); it++) {
		IMesh* pMesh = dynamic_cast<IMesh*>((*it)->GetRessource());
		if (pMesh)
			vBackupStaticObjectShader.push_back(pMesh->GetShader());
	}
}

void CCollisionManager::SetCollisionShaders(const vector<IEntity*>& staticObjects, IShader* pCollisionShader)
{
	for (vector<IEntity*>::const_iterator it = staticObjects.begin(); it != staticObjects.end(); it++) {
		(*it)->SetShader(pCollisionShader);
	}
}

void CCollisionManager::RestoreOriginalShaders(const vector<IShader*>& vBackupStaticObjectShader, vector<IEntity*>& staticObjects)
{
	int i = 0;
	for (vector<IEntity*>::iterator it = staticObjects.begin(); it != staticObjects.end(); it++) {
		(*it)->SetShader(vBackupStaticObjectShader[i++]);
	}
}

void CCollisionManager::OnRenderHeightMap( IRenderer* pRenderer )
{
	pRenderer->SetBackgroundColor( 0, 0, 1 );

	IShader* pOrgShader = s_pMesh->GetShader();
	IShader* pHMShader = pRenderer->GetShader( "hm" );
	pHMShader->Enable( true );
	s_pMesh->SetShader( pHMShader );
	
	CMatrix oModelView, oProj;
	unsigned int nWidth, nHeight;
	pRenderer->GetResolution( nWidth, nHeight );
	float fScreenRatio = (float)nWidth / (float)nHeight;
	oProj.m_00 = 1.f / fScreenRatio;
	const IBox* pBox = s_pMesh->GetBBox();
	float maxLenght = pBox->GetDimension().m_x ;
	if( maxLenght < pBox->GetDimension().m_y )
		maxLenght = pBox->GetDimension().m_y;
	float scale = ( maxLenght / 2.f );
	pHMShader->SendUniformValues( "h", pBox->GetDimension().m_z );
	pHMShader->SendUniformValues( "zMin", pBox->GetMinPoint().m_z );
	pHMShader->SendUniformValues( "zMax", pBox->GetMinPoint().m_z + pBox->GetDimension().m_z );
	pHMShader->SendUniformValues( "scale", scale );

	pRenderer->SetModelViewMatrix( oModelView );
	CMatrix oBakProj;
	pRenderer->GetProjectionMatrix( oBakProj );
	pRenderer->SetProjectionMatrix( oProj );

	s_pMesh->Update();

	pHMShader->Enable( false );
	s_pMesh->SetShader( pOrgShader );
	pRenderer->SetProjectionMatrix( oBakProj );
}

int CCollisionManager::LoadHeightMap( string sFileName, IBox* pBox, bool forceReload)
{
	int nID = 0;
	map<string, int>::iterator it = m_mMapFileToId.find(sFileName);
	if (forceReload || (it == m_mMapFileToId.end()) ) {
		CHeightMap* pHeightMap = new CHeightMap(m_oInterface, sFileName, *pBox);
		pHeightMap->SetPrecision(m_nHeightMapPrecision);
		nID = (int)m_mHeigtMap.size();
		m_mHeigtMap[nID] = pHeightMap;
		m_mMapFileToId[sFileName] = nID;
	}
	else
		nID = it->second;
	return nID;
}

void CCollisionManager::LoadHeightMap(string sFileName, vector< vector< unsigned char > >& vPixels)
{
	ILoader::CTextureInfos ti;
	m_oLoaderManager.Load(sFileName, ti);
	vPixels.clear();
	vPixels.resize(ti.m_nWidth);
	for (int i = 0; i < ti.m_nWidth; i++)
		for (int j = 0; j < ti.m_nHeight; j++)
			vPixels[i].push_back(ti.m_vTexels[j * (ti.m_nWidth + 1)]);
}

float CCollisionManager::GetMapHeight( int nHeightMapID, float xModel, float zModel )
{
	float fInterpolate = 0.f;
	map< int, CHeightMap* >::iterator itMap = m_mHeigtMap.find( nHeightMapID );
	if (itMap != m_mHeigtMap.end())
		fInterpolate = itMap->second->GetHeight(xModel, zModel);
	return fInterpolate;
}

bool CCollisionManager::IsSegmentInsideSegment( float fS1Center, float fS1Radius, float fS2Center, float fS2Radius )
{
	return ( ( fS1Center + fS1Radius ) > ( fS2Center - fS2Radius ) ) && ( ( fS1Center - fS1Radius ) < ( fS2Center + fS2Radius ) );
}

bool CCollisionManager::IsIntersection( const IBox& b, const ISphere& s )
{
	CMatrix invBoxWorldMatrix, oWorldMatrix;
	b.GetTM( oWorldMatrix );
	oWorldMatrix.GetInverse( invBoxWorldMatrix );
	CVector vBoxBaseSphereCenter = invBoxWorldMatrix * s.GetCenter();
	bool bInsideX = IsSegmentInsideSegment( 0, b.GetDimension().m_x / 2.f, vBoxBaseSphereCenter.m_x, s.GetRadius() ); //vBoxBaseSphereCenter.m_x - s.GetRadius() < b.GetDimension().m_x / 2.f && vBoxBaseSphereCenter.m_x + s.GetRadius() > - b.GetDimension().m_x / 2.f;
	bool bInsideY = IsSegmentInsideSegment( 0, b.GetDimension().m_y / 2.f, vBoxBaseSphereCenter.m_y, s.GetRadius() );
	bool bInsideZ = IsSegmentInsideSegment( 0, b.GetDimension().m_z / 2.f, vBoxBaseSphereCenter.m_z, s.GetRadius() );
	return ( bInsideX && bInsideY && bInsideZ );
}

float GetMinx( const vector< CVector >& vPoints )
{
	float fMin = vPoints[ 0 ].m_x;
	for( int i = 1; i < vPoints.size(); i++ )
	{
		if( fMin > vPoints[ i ].m_x )
			fMin = vPoints[ i ].m_x;
	}
	return fMin;
}

float GetMiny( const vector< CVector >& vPoints )
{
	float fMin = vPoints[ 0 ].m_y;
	for( int i = 1; i < vPoints.size(); i++ )
	{
		if( fMin > vPoints[ i ].m_y )
			fMin = vPoints[ i ].m_y;
	}
	return fMin;
}

float GetMinz( const vector< CVector >& vPoints )
{
	float fMin = vPoints[ 0 ].m_z;
	for( int i = 1; i < vPoints.size(); i++ )
	{
		if( fMin > vPoints[ i ].m_z )
			fMin = vPoints[ i ].m_z;
	}
	return fMin;
}

float GetMaxx( const vector< CVector >& vPoints )
{
	float fMax = vPoints[ 0 ].m_x;
	for( int i = 1; i < vPoints.size(); i++ )
	{
		if( fMax < vPoints[ i ].m_x )
			fMax = vPoints[ i ].m_x;
	}
	return fMax;
}

float GetMaxy( const vector< CVector >& vPoints )
{
	float fMax = vPoints[ 0 ].m_y;
	for( int i = 1; i < vPoints.size(); i++ )
	{
		if( fMax < vPoints[ i ].m_y )
			fMax = vPoints[ i ].m_y;
	}
	return fMax;
}

float GetMaxz( const vector< CVector >& vPoints )
{
	float fMax = vPoints[ 0 ].m_z;
	for( int i = 1; i < vPoints.size(); i++ )
	{
		if( fMax < vPoints[ i ].m_z )
			fMax = vPoints[ i ].m_z;
	}
	return fMax;
}

bool CCollisionManager::IsIntersection( const ISegment& s, const IBox& b2 )
{
	return true;
}

bool CCollisionManager::IsIntersection( const ISegment& s, const CVector& oCircleCenter, float fCircleRadius )
{
	CVector H;
	s.ComputeProjectedPointOnLine( oCircleCenter, H );

	return false;
}

void CCollisionManager::Get2DLineIntersection( const CVector2D& oLine1First, const CVector2D& oLine1Last, const CVector2D& oLine2First, const CVector2D& oLine2Last, CVector2D& oIntersection )
{
	ISegment2D* pL1 = m_oGeometryManager.CreateSegment2D( oLine1First, oLine1Last );
	ISegment2D* pL2 = m_oGeometryManager.CreateSegment2D( oLine2First, oLine2Last );
	Get2DLineIntersection( *pL1, *pL2, oIntersection );
}

void CCollisionManager::Get2DLineIntersection( const ISegment2D& oSeg1, const ISegment2D& oSeg2, CVector2D& oIntersection )
{
	float a1, b1, c1, a2, b2, c2;
	oSeg1.ComputeLineEquation( a1, b1, c1 );
	oSeg2.ComputeLineEquation( a2, b2, c2 );
	oIntersection.m_x = 1 / a1 * ( b1 * ( (a2 * c1 - a1 * c2 ) / ( a2 * b1 - a1 * b2 )  ) - c1 );
	oIntersection.m_y = ( a1 * c2 - a2 * c1 ) / ( a2 * b1 - a1 * b2 );
}

bool CCollisionManager::IsSegmentRectIntersect( const ISegment2D& s, float fRectw, float fRecth, const CMatrix2X2& oRectTM )
{
	CVector2D S1, S2;
	s.GetPoints( S1, S2 );
	return IsSegmentRectIntersect( S1, S2, fRectw, fRecth, oRectTM );

}

bool CCollisionManager::IsSegmentRectIntersect( const CVector2D& S1, const CVector2D& S2, float fRectw, float fRecth, const CMatrix2X2& oRectTM )
{
	CVector2D x( 1, 0 );

	CVector2D R0 = CVector2D( -fRectw / 2.f, -fRecth / 2.f );
	CVector2D R1 = CVector2D( fRectw / 2.f, -fRecth / 2.f );
	CVector2D R2 = CVector2D( fRectw / 2.f, fRecth / 2.f );
	CVector2D R3 = CVector2D( -fRectw / 2.f, fRecth / 2.f );
	CMatrix2X2 oRectInvTM;
	oRectTM.GetInverse( oRectInvTM );
	
	CVector2D S1Inv = oRectInvTM * S1;
	CVector2D S2Inv = oRectInvTM * S2;

	float fMinx = R0.m_x, fMaxx = R1.m_x, fMinz = R0.m_y, fMaxz = R2.m_y;
	float fSegMinx = S1Inv.m_x;
	if( fSegMinx > S2Inv.m_x ) fSegMinx = S2Inv.m_x;
	if( fSegMinx > fMaxx )
		return false;
	float fSegMaxx = S1Inv.m_x;
	if( fSegMaxx < S2Inv.m_x ) fSegMaxx = S2Inv.m_x;
	if( fSegMaxx < fMinx )
		return false;
	float fSegMinz = S1Inv.m_y;
	if( fSegMinz > S2Inv.m_y ) fSegMinz = S2Inv.m_y;
	if( fSegMinz > fMaxz )
		return false;
	float fSegMaxz = S1Inv.m_y;
	if( fSegMaxz < S2Inv.m_y ) fSegMaxz = S2Inv.m_y;
	if( fSegMaxz < fMinz )
		return false;

	CVector2D R0tm = oRectTM * CVector2D( -fRectw / 2.f, -fRecth / 2.f );
	CVector2D R1tm = oRectTM * CVector2D( fRectw / 2.f, -fRecth / 2.f );
	CVector2D R2tm = oRectTM * CVector2D( fRectw / 2.f, fRecth / 2.f );
	CVector2D R3tm = oRectTM * CVector2D( -fRectw / 2.f, fRecth / 2.f );

	float alpha = acosf( ( ( S2 - S1 ) * x ) / ( S2 - S1 ).Norm() ) * 180.f / 3.1415927f;
	CMatrix2X2 oSegTM = CMatrix2X2::GetRotation( alpha ), oSegTMInv;
	oSegTM.AddTranslation( S1 );
	oSegTM.GetInverse( oSegTMInv );

	CVector2D R0Inv = oSegTMInv * R0tm;
	CVector2D R1Inv = oSegTMInv * R1tm;
	CVector2D R2Inv = oSegTMInv * R2tm;
	CVector2D R3Inv = oSegTMInv * R3tm;

	CVector2D S3Inv = oSegTMInv * S2;

	fMinx = R0Inv.m_y;
	if( R1Inv.m_x < fMinx ) fMinx = R1Inv.m_x;
	if( R2Inv.m_x < fMinx ) fMinx = R2Inv.m_x;
	if( R3Inv.m_x < fMinx ) fMinx = R3Inv.m_x;
	if( fMinx > S3Inv.m_x )
		return false;

	fMaxx = R0Inv.m_y;
	if( R1Inv.m_x > fMaxx ) fMaxx = R1Inv.m_y;
	if( R2Inv.m_x > fMaxx ) fMaxx = R2Inv.m_y;
	if( R3Inv.m_x > fMaxx ) fMaxx = R3Inv.m_y;
	if( fMaxx < 0 )
		return false;

	fMinz = R0Inv.m_y;
	if( R1Inv.m_y < fMinz ) fMinz = R1Inv.m_y;
	if( R2Inv.m_y < fMinz ) fMinz = R2Inv.m_y;
	if( R3Inv.m_y < fMinz ) fMinz = R3Inv.m_y;
	if( fMinz > 0 )
		return false;

	fMaxz = R0Inv.m_y;
	if( R1Inv.m_y > fMaxz ) fMaxz = R1Inv.m_y;
	if( R2Inv.m_y > fMaxz ) fMaxz = R2Inv.m_y ;
	if( R3Inv.m_y > fMaxz ) fMaxz = R3Inv.m_y;
	if( fMaxz < 0 )
		return false;

	return true;
}

void CCollisionManager::SetGroundBoxHeight(int nMapId, float height)
{
	m_mHeigtMap[nMapId]->GetModelBBox()->SetY(height);
}

void CCollisionManager::SetGroundBoxMinPoint(int nMapId, float min)
{
	m_mHeigtMap[nMapId]->GetModelBBox()->SetY(min);
}

IBox* CCollisionManager::GetGroundBox(int nMapId)
{
	return m_mHeigtMap[nMapId]->GetModelBBox();
}

string CCollisionManager::GetName()
{
	return "CollisionManager";
}

IHeightMap*	CCollisionManager::GetHeightMap(int index)
{
	map< int, CHeightMap*>::iterator it = m_mHeigtMap.find(index);
	if(it != m_mHeigtMap.end())
		return it->second;
	return nullptr;
}

void CCollisionManager::ClearHeightMaps()
{
	m_mHeigtMap.clear();
	m_mMapFileToId.clear();
}

void CCollisionManager::SendCustomUniformValue(string name, float value)
{
	m_sCustomName = name;
	m_fCustomValue = value;
}

void CCollisionManager::DisplayGrid(int nCellSize)
{
	CVector first, last;
	int rowCount = m_fGroundHeight / (float)nCellSize + 1;
	int columnCount = m_fGroundWidth / (float)nCellSize + 1;
	int w = m_fGroundWidth;
	int h = m_fGroundHeight;
	int s = (float)nCellSize;
	if (w % s > 0)
		columnCount++;
	if (h % s > 0)
		rowCount++;

	float lineWidth = 15.f;
	float y = 1130.f;

	for (int i = 0; i < rowCount; i++) {
		first.m_x = -m_fGroundWidth / 2;
		first.m_z = -m_fGroundHeight / 2 + i * (float)nCellSize;

		first.m_y = y;

		IEntity* line = m_pEntityManager->CreateCylinder(lineWidth, m_fGroundWidth);
		line->Link(m_pScene);
		line->SetLocalPosition(first);
		line->Roll(-90.f);
		line->LocalTranslate(0, m_fGroundWidth / 2, 0);
		line->Colorize(1, 0, 0, 1);
		m_vGridElements.push_back(line);
	}

	for (int i = 0; i < columnCount; i++) {
		first.m_x = -m_fGroundWidth / 2 + i * (float)nCellSize;
		first.m_z = -m_fGroundHeight / 2;
		
		first.m_y = y;

		IEntity* line = m_pEntityManager->CreateCylinder(lineWidth, m_fGroundWidth);
		line->Link(m_pScene);
		line->SetLocalPosition(first);
		line->Pitch(90.f);
		line->LocalTranslate(0, m_fGroundWidth / 2, 0);
		m_vGridElements.push_back(line);
	}
}

void CCollisionManager::MarkBox(int row, int column, float r, float g, float b)
{
	float x, z;
	GetPositionFromCellCoord(row, column, x, z);
	IEntity* pSphere = m_oEntityManager.CreateSphere((float)m_pScene->GetCellSize() * 3. / 8.);
	IShader* pColorShader = m_oRenderer.GetShader("color");
	pSphere->SetShader(pColorShader);
	pSphere->Colorize(r, g, b, 0.5f);
	pSphere->SetLocalPosition(x + (float)m_pScene->GetCellSize() / 2.f, m_fGridHeight , z + (float)m_pScene->GetCellSize() / 2.f);
	pSphere->Link(m_pScene);
	m_vGridElements.push_back(pSphere);
}

void CCollisionManager::GetPositionFromCellCoord(int row, int column, float& x, float& y)
{
	int rowCount, columnCount;
	int nCellSize = m_pScene->GetCellSize();
	ComputeRowAndColumnCount(rowCount, columnCount, nCellSize);
	x = (0.5f + (float)column - (float)columnCount / 2.f) * (float)nCellSize;
	y = (-(float)row + (float)rowCount / 2.f ) * (float)nCellSize;
}

void CCollisionManager::GetCellCoordFromPosition(float x, float y, int& cellx, int& celly, int nCellSize)
{
	cellx = (int)( (x + m_fGroundWidth / 2) / (float)nCellSize);
	celly = (int)( (-y + m_fGroundHeight / 2) / (float)nCellSize);
}

void CCollisionManager::ComputeRowAndColumnCount(int& rowCount, int& columnCount, int nCellSize)
{
	rowCount = m_fGroundHeight / (float)nCellSize;
	columnCount = m_fGroundWidth / (float)nCellSize;
}

void CCollisionManager::ConvertLinearToCoord(int pixelNumber, int nTextureWidth, int& x, int& y)
{
	x = pixelNumber / 3 - pixelNumber / (3 * nTextureWidth) * nTextureWidth;
	y = pixelNumber / (3 * nTextureWidth);
}

void GridToMap(int xGrid, int yGrid, const CDimension& mapDimension, const CDimension& gridDimension, float& xMap, float& zMap)
{
	xMap = mapDimension.GetWidth() * ((float)xGrid / gridDimension.GetWidth() - 0.5f);
	zMap = mapDimension.GetHeight() * (0.5f - (float)yGrid / gridDimension.GetHeight());
}

void ModelToGrid(int xMap, int zMap, const CDimension& mapDimension, const CDimension& gridDimension, int& xGrid, int& yGrid)
{
	xGrid = xMap * gridDimension.GetWidth() / mapDimension.GetWidth() + gridDimension.GetWidth() / 2.f;
	yGrid = gridDimension.GetHeight() * (0.5f - zMap / mapDimension.GetHeight());
}

void CCollisionManager::CreateCollisionArray(IEntity* pRoot, vector<vector<bool>>& vGrid, int nCellSize, float fBias)
{
	IBox* pRootBBox = dynamic_cast<IBox*>(pRoot->GetBoundingGeometry());
	if (!pRootBBox)
		throw CEException("CreateCollisionArray() : entity root has no bounding box");
	int nGridWidth = pRootBBox->GetDimension().m_x / nCellSize;
	int nGridHeight = pRootBBox->GetDimension().m_z / nCellSize;
	nGridWidth = nGridWidth % 4 ? ((nGridWidth) >> 2) << 2 : nGridWidth;
	nGridHeight = nGridHeight % 2 ? (nGridHeight >> 1) << 1  : nGridHeight;

	vGrid.resize(nGridWidth);
	for (int i = 0; i < vGrid.size(); i++)
		vGrid[i].resize(nGridHeight);
	
	vector<IEntity*> collisionEntities;
	GetCollisionEntities(pRoot, collisionEntities);

	for(IEntity* pChild : collisionEntities) {
		IGeometry* pChildGeometry = pChild->GetBoundingGeometry();
		if (pChildGeometry) {
			CVector boxDim;
			pChildGeometry->GetBBoxDimension(boxDim);
			AddObjectToCollisionGrid(pRootBBox->GetDimension(), CDimension(nGridWidth, nGridHeight), boxDim, pChild->GetLocalMatrix(), vGrid, nCellSize, fBias);
		}
	}
}

void CCollisionManager::CreateTextureFromCollisionArray(string sFileName, const vector<vector<bool>>& vGrid)
{
	ILoader::CTextureInfos ti;
	ti.m_ePixelFormat = ILoader::TPixelFormat::eRGB;
	ti.m_nWidth = vGrid.size();
	ti.m_nHeight = vGrid[0].size();	

	ti.m_nWidth = ti.m_nWidth % 4 ? (ti.m_nWidth >> 2) << 2 : ti.m_nWidth;
	ti.m_nHeight = ti.m_nHeight % 2 ? (ti.m_nHeight >> 1) << 1 : ti.m_nHeight;

	ti.m_sFileName = sFileName;
	ti.m_vTexels.resize(ti.m_nWidth * ti.m_nHeight * 3);

	for (int i = 0; i < ti.m_nWidth; i++) {
		for (int j = 0; j < ti.m_nHeight; j++) {
			unsigned char color = vGrid[i][j] ? 255 : 0;
			for (int k = 0; k < 3; k++)
				ti.m_vTexels[3 * (i + j * ti.m_nWidth) + k] = color;
		}
	}
	m_oLoaderManager.Export(sFileName, ti);
}

void CCollisionManager::SortObjectsByHeight(IEntity* pRoot, vector<pair<float, IEntity*>>& vSortedObjectHeight)
{
	IBox* pRootBBox = dynamic_cast<IBox*>(pRoot->GetBoundingGeometry());
	for (int i = 0; i < pRoot->GetChildCount(); i++) {
		IEntity* pObject = dynamic_cast<IEntity*>(pRoot->GetChild(i));
		IGeometry* pBBox = pObject->GetBoundingGeometry();
		if (pBBox) {
			CVector childDim;
			pBBox->GetBBoxDimension(childDim);
			CMatrix childTM = pObject->GetLocalMatrix();
			CVector rootDim = pRootBBox->GetDimension();

			float objectMaxHeight = (childTM.m_13 + childDim.m_y / 2.f);

			bool inserted = false;
			for (vector<pair<float, IEntity*>>::iterator it = vSortedObjectHeight.begin(); it != vSortedObjectHeight.end();) {
				if (objectMaxHeight < it->first) {
					IBox* pLastBBox = (IBox*)it->second->GetBoundingGeometry();
					IEntity* pLastEntity = it->second;
					it = vSortedObjectHeight.insert(it, pair<float, IEntity*>(objectMaxHeight, pObject));
					inserted = true;
					break;
				}
				else
					it++;
			}
			if (vSortedObjectHeight.empty() || !inserted)
				vSortedObjectHeight.push_back(pair<float, IEntity*>(objectMaxHeight, pObject));
		}
	}
}

float CCollisionManager::GetFloors(const vector<pair<float, IEntity*>>& vSortedObjectHeight, vector<pair<float,IEntity*>>& floors, vector<pair<float, IEntity*>>& nonFloors)
{
	float fGroundHeight = 0;
	for (vector<pair<float, IEntity*>>::const_iterator it = vSortedObjectHeight.begin(); it != vSortedObjectHeight.end(); it++) {
		if (it->first < fGroundHeight + 80.f)
			floors.push_back(*it);
		else {
			IEntity* pNonFloor = it->second;
			CMatrix nonFloorTM = pNonFloor->GetLocalMatrix();
			IGeometry* pNonFloorBBox = pNonFloor->GetBoundingGeometry();
			CVector nonFloorDim;
			pNonFloorBBox->GetBBoxDimension(nonFloorDim);
			float nonFloorMinHeight = (nonFloorTM.m_13 - nonFloorDim.m_y / 2.f);
			nonFloors.push_back(pair<float, IEntity*>(nonFloorMinHeight, pNonFloor));
		}
	}
	
	// exclude furnitures
	float hMargin = 10.f;
	for (vector<pair<float, IEntity*>>::iterator itFloor = floors.begin(); itFloor != floors.end();) {
		bool bErase = false;
		for (pair<float, IEntity*>& nonFloor : nonFloors) {
			if (itFloor->first - hMargin > nonFloor.first) {
				itFloor = floors.erase(itFloor);
				bErase = true;
				break;
			}
		}
		if (!bErase)
			itFloor++;
	}

	// Get ground height
	fGroundHeight = -9999999999.f;
	for (pair<float, IEntity*>& floor : floors) {
		if (fGroundHeight < floor.first)
			fGroundHeight = floor.first;
	}

	return fGroundHeight;
}

void CCollisionManager::GetRoofs(float fFloorHeight, const vector<pair<float, IEntity*>>& nonFloors, vector<IEntity*>& roofs, vector<IEntity*>& nonRoofs)
{
	for (const pair<float, IEntity*>& r : nonFloors) {
		if (r.first > fFloorHeight + 200.f)
			roofs.push_back(r.second);
		else
			nonRoofs.push_back(r.second);
	}
}

void CCollisionManager::GetCollisionEntities(IEntity* pRoot, vector<IEntity*>& vCollisionEntities)
{
	ISceneManager* pSceneManager = static_cast<ISceneManager*>(m_oInterface.GetPlugin("SceneManager"));
	IScene* pScene = pSceneManager->GetScene("Game");
	if (pRoot == pScene) {
		for (int i = 0; i < pScene->GetChildCount(); i++) {
			IEntity* pEntity = dynamic_cast<IEntity*>(pScene->GetChild(i));
			if (pEntity)
				vCollisionEntities.push_back(pEntity);
		}
	}
	else {
		vector<pair<float, IEntity*>> vSortedObjectHeight;
		IBox* pRootBBox = dynamic_cast<IBox*>(pRoot->GetBoundingGeometry());
		SortObjectsByHeight(pRoot, vSortedObjectHeight);

		vector<pair<float, IEntity*>> floors, nonFloors;
		float fFloorHeight = GetFloors(vSortedObjectHeight, floors, nonFloors);

		vector<IEntity*> roofs;
		GetRoofs(fFloorHeight, nonFloors, roofs, vCollisionEntities);
	}
}


void CCollisionManager::AddObjectToCollisionGrid(const CVector& rootDim, const CDimension& gridDimension, const CVector& objectDim, const CMatrix& modelTM, vector<vector<bool>>& vGrid, int nCellSize, float fBias)
{
	float x0, z0, x1, z1;
	CDimension mapDimension(rootDim.m_x, rootDim.m_z);
	GridToMap(0, 0, mapDimension, gridDimension, x0, z0);
	GridToMap(1, 1, mapDimension, gridDimension, x1, z1);
	float modelUnit = abs(x1 - x0);
	modelUnit -= modelUnit * fBias;
	CVector modelPos = modelTM.GetPosition();
	int xModel = -objectDim.m_x / 2.f;
	int zModel = 0;
	while (xModel < objectDim.m_x / 2.f) {
		zModel = -objectDim.m_z / 2.f;
		while (zModel < (objectDim.m_z / 2.f)) {
			CVector P((float)xModel, 0, (float)zModel);
			CVector PTransform = modelTM * P;
			int xGrid = 0, yGrid = 0;
			ModelToGrid(PTransform.m_x, PTransform.m_z, mapDimension, gridDimension, xGrid, yGrid);
			xGrid = xGrid < 0.f ? 0 : (xGrid >= gridDimension.GetWidth() ? gridDimension.GetWidth() - 1 : xGrid);
			yGrid = yGrid < 0.f ? 0 : (yGrid >= gridDimension.GetHeight() ? gridDimension.GetHeight() - 1 : yGrid);
			vGrid[xGrid][yGrid] = true;
			zModel += modelUnit;
		}
		xModel += modelUnit;
	}
}

void CCollisionManager::EnableHMHack(bool enable)
{
	m_bEnableHMHack = enable;
}

void CCollisionManager::EnableHMHack2(bool enable)
{
	m_bEnableHMHack2 = enable;
}


bool CCollisionManager::TestCellObstacle(const ILoader::CTextureInfos& collisionMap, int x, int y)
{
	unsigned char free[3] = { 0, 0, 0};
	unsigned char rgb[3];
	memcpy(rgb, &collisionMap.m_vTexels[3 * (x + y * collisionMap.m_nWidth)], 3);
	return !memcmp(free, rgb, 3) == 0;
}

float CCollisionManager::WorldToPixel(float worldLenght, int nMapWidth, int nMapHeight)
{
	float ret;
	IBox* pBox = m_pGround->GetBBox();
	if (pBox->GetDimension().m_x <= pBox->GetDimension().m_z * m_fScreenRatio)
		ret = (worldLenght * (float)nMapHeight) / pBox->GetDimension().m_z;
	else
		ret = worldLenght * (float)nMapWidth / pBox->GetDimension().m_x;
	return ret;
}

void CCollisionManager::SetEntityManager(IEntityManager* pEntityManager)
{
	m_pEntityManager = pEntityManager;
}

extern "C" _declspec(dllexport) CCollisionManager* CreateCollisionManager(EEInterface& oInterface)
{
	return new CCollisionManager(oInterface);
}