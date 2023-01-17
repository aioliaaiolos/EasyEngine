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
#include "CollisionMap.h"

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

ICollisionMap* CCollisionManager::CreateCollisionMap(IEntity* pScene, float fBias)
{
	CCollisionMap* pCollisionMap = new CCollisionMap(m_oInterface, pScene);
	return pCollisionMap;
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
	float fGroundMapWidth, fGroundMapHeight, fWorldToScreenScaleFactor;
	ComputeGroundMapDimensions(pGround, nScreenWidth, nScreenHeight, fGroundMapWidth, fGroundMapHeight, fWorldToScreenScaleFactor);
	if(m_bEnableHMHack)
		fWorldToScreenScaleFactor *= 1.70068f;
	else if (m_bEnableHMHack2) {
		fWorldToScreenScaleFactor *= 2.55103f;
	}
	float fOriginMapX = ((float)nWidth - fGroundMapWidth) / 2.f;
	float fOriginMapY = ((float)nHeight - fGroundMapHeight) / 2.f;

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
	pHMShader->SendUniformValues("scale", fWorldToScreenScaleFactor);

	CMatrix oBakProj;
	m_oRenderer.GetProjectionMatrix(oBakProj);
	m_oRenderer.SetProjectionMatrix(oProj);

	m_oRenderer.CullFace(false);
	m_oRenderer.BeginRender();
	pGround->Update();

	m_oRenderer.ReadPixels(fOriginMapX, fOriginMapY, fGroundMapWidth, fGroundMapHeight, ti.m_vTexels, format);
	ti.m_nWidth = (int)fGroundMapWidth;
	ti.m_nHeight = (int)fGroundMapHeight;
	m_oRenderer.EndRender();
	m_oRenderer.CullFace(true);

	pHMShader->Enable(false);
	pGround->SetShader(pOrgShader);
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
	int nCellCount = (int)(dim.m_x * dim.m_z);
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

void CCollisionManager::EnableHMHack(bool enable)
{
	m_bEnableHMHack = enable;
}

void CCollisionManager::EnableHMHack2(bool enable)
{
	m_bEnableHMHack2 = enable;
}

void CCollisionManager::SetEntityManager(IEntityManager* pEntityManager)
{
	m_pEntityManager = pEntityManager;
}

extern "C" _declspec(dllexport) CCollisionManager* CreateCollisionManager(EEInterface& oInterface)
{
	return new CCollisionManager(oInterface);
}