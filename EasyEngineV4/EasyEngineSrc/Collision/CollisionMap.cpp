#include "CollisionMap.h"
#include "IGeometry.h"
#include "IEntity.h"
#include "IFileSystem.h"

#include "Utils2/StringUtils.h"

CCollisionMap::CCollisionMap(EEInterface& oInterface, IEntity* pScene, int nCellSize) :
	m_pScene(pScene),
	m_oInterface(oInterface),
	m_oLoaderManager(*dynamic_cast<ILoaderManager*>(m_oInterface.GetPlugin("LoaderManager"))),
	m_oEntityManager(*dynamic_cast<IEntityManager*>(m_oInterface.GetPlugin("EntityManager"))),
	m_pGround(nullptr)
{
	m_nCellSize = nCellSize;	
	
	m_pGround = dynamic_cast<IMesh*>(m_pScene->GetRessource());
	if (m_pGround) {
		IBox* pBox = m_pGround->GetBBox();
		m_fGroundWidth = pBox->GetDimension().m_x;
		m_fGroundHeight = pBox->GetDimension().m_z;
		string sFileName, sFolderPath, sMeshFileName, sSimpleFileName;
		m_pGround->GetFileName(sMeshFileName);
		int idx = sMeshFileName.find_last_of('/');
		sFolderPath = sMeshFileName.substr(0, idx);
		sSimpleFileName = sMeshFileName.substr(idx + 1);
		string sFileNameWithoutExtension;
		CStringUtils::GetFileNameWithoutExtension(sSimpleFileName, sFileNameWithoutExtension);
		m_sFileName = sFolderPath + "/" + sFileNameWithoutExtension + "-collision.bmp";
	}
}

void CCollisionMap::SetFileName(string sFileName)
{
	m_sFileName = sFileName;
}

void CCollisionMap::Generate()
{
	vector<vector<bool>> vGrid;

	int nCellSize = m_nCellSize;
	float lenght = max(m_pScene->GetBoundingGeometry()->GetBBoxDimension().m_x, m_pScene->GetBoundingGeometry()->GetBBoxDimension().m_z);
	if (lenght > 500000)
		nCellSize = 200;
	else
		nCellSize = 64;
	CreateCollisionArray(m_pScene, vGrid, nCellSize, 0);
	CreateTextureFromCollisionArray(m_sFileName, vGrid);
}

void CCollisionMap::Load()
{
	m_oLoaderManager.LoadTexture(m_sFileName, m_oCollisionTexture);
}

void CCollisionMap::SortObjectsByHeight(IEntity* pRoot, vector<pair<float, IEntity*>>& vSortedObjectHeight)
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

bool IsWallDimensions(const CVector& oBoxDimension)
{
	float scaleFactor = 2.f;
	return (oBoxDimension.m_y > 200 && (scaleFactor * oBoxDimension.m_x < oBoxDimension.m_y || scaleFactor * oBoxDimension.m_z < oBoxDimension.m_y));
}

float CCollisionMap::GetFloors(const vector<pair<float, IEntity*>>& vSortedObjectHeight, vector<pair<float, IEntity*>>& floors, vector<pair<float, IEntity*>>& nonFloors)
{
	float fGroundHeight = 0;
	for (vector<pair<float, IEntity*>>::const_iterator it = vSortedObjectHeight.begin(); it != vSortedObjectHeight.end(); it++) {
		if ( (it->first < fGroundHeight + 80.f) && !IsWallDimensions(it->second->GetBoundingGeometry()->GetBBoxDimension()) )
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

void CCollisionMap::GetRoofs(float fFloorHeight, const vector<pair<float, IEntity*>>& nonFloors, vector<IEntity*>& roofs, vector<IEntity*>& nonRoofs)
{
	for (const pair<float, IEntity*>& r : nonFloors) {
		if (r.first > fFloorHeight + 200.f)
			roofs.push_back(r.second);
		else
			nonRoofs.push_back(r.second);
	}
}

void CCollisionMap::GetDoors(const vector<IEntity*>& vCollisionEntities, vector<IEntity*>& doors)
{
	for (const IEntity* pEntity : vCollisionEntities)
	{
		if (pEntity->GetChildCount() > 0) {
			for (int i = 0; i < pEntity->GetChildCount(); i++) {
				INode* pChild = pEntity->GetChild(i);
				if (pChild->GetName().find("Door") != -1) {
					IEntity* pChildEntity = dynamic_cast<IEntity*>(pChild);
					doors.push_back(pChildEntity);
				}
			}
		}
	}
}

void CCollisionMap::GetCollisionEntities(IEntity* pRoot, vector<IEntity*>& vCollisionEntities)
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

		vector<IEntity*> doors;
		GetDoors(vCollisionEntities, doors);
		vCollisionEntities.insert(vCollisionEntities.end(), doors.begin(), doors.end());
	}
}


void CCollisionMap::AddObjectToCollisionGrid(const CVector& rootDim, const CDimension& gridDimension, const CVector& objectDim, const CMatrix& modelTM, vector<vector<bool>>& vGrid, int nCellSize, bool isObstacle, float fBias)
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
			vGrid[xGrid][yGrid] = isObstacle;
			zModel += modelUnit;
		}
		xModel += modelUnit;
	}
}

bool CCollisionMap::TestCellObstacle(int x, int y)
{
	unsigned char free[3] = { 0, 0, 0 };
	unsigned char rgb[3];
	memcpy(rgb, &m_oCollisionTexture.m_vTexels[3 * (x + y * m_oCollisionTexture.m_nWidth)], 3);
	return !memcmp(free, rgb, 3) == 0;
}

void CCollisionMap::CreateCollisionArray(IEntity* pRoot, vector<vector<bool>>& vGrid, int nCellSize, float fBias)
{
	IBox* pRootBBox = dynamic_cast<IBox*>(pRoot->GetBoundingGeometry());
	if (!pRootBBox)
		throw CEException("CreateCollisionArray() : entity root has no bounding box");
	int nGridWidth = pRootBBox->GetDimension().m_x / nCellSize;
	int nGridHeight = pRootBBox->GetDimension().m_z / nCellSize;
	nGridWidth = nGridWidth % 4 ? ((nGridWidth) >> 2) << 2 : nGridWidth;
	nGridHeight = nGridHeight % 2 ? (nGridHeight >> 1) << 1 : nGridHeight;

	vGrid.resize(nGridWidth);
	for (int i = 0; i < vGrid.size(); i++)
		vGrid[i].resize(nGridHeight);

	vector<IEntity*> collisionEntities;
	vector<INode*> doors;
	GetCollisionEntities(pRoot, collisionEntities);

	for (IEntity* pChild : collisionEntities) {
		IGeometry* pChildGeometry = pChild->GetBoundingGeometry();
		if (pChildGeometry) {
			CVector boxDim;
			pChildGeometry->GetBBoxDimension(boxDim);
			AddObjectToCollisionGrid(pRootBBox->GetDimension(), CDimension(nGridWidth, nGridHeight), boxDim, pChild->GetLocalMatrix(), vGrid, nCellSize, pChild->GetName().find("Door") == -1, fBias);
		}
	}
}

void CCollisionMap::GetPositionFromCellCoord(int row, int column, float& x, float& y)
{
	int rowCount, columnCount;
	int nCellSize = m_pScene->GetCellSize();
	ComputeRowAndColumnCount(rowCount, columnCount, nCellSize);
	x = (0.5f + (float)column - (float)columnCount / 2.f) * (float)nCellSize;
	y = (-(float)row + (float)rowCount / 2.f) * (float)nCellSize;
}

void CCollisionMap::GetCellCoordFromPosition(float x, float y, int& cellx, int& celly)
{
	cellx = (int)((x + m_fGroundWidth / 2) / (float)m_nCellSize);
	celly = (int)((-y + m_fGroundHeight / 2) / (float)m_nCellSize);
}

void CCollisionMap::ComputeRowAndColumnCount(int& rowCount, int& columnCount, int nCellSize)
{
	rowCount = m_fGroundHeight / (float)nCellSize;
	columnCount = m_fGroundWidth / (float)nCellSize;
}

void CCollisionMap::ConvertLinearToCoord(int pixelNumber, int nTextureWidth, int& x, int& y)
{
	x = pixelNumber / 3 - pixelNumber / (3 * nTextureWidth) * nTextureWidth;
	y = pixelNumber / (3 * nTextureWidth);
}

void CCollisionMap::GridToMap(int xGrid, int yGrid, const CDimension& mapDimension, const CDimension& gridDimension, float& xMap, float& zMap)
{
	xMap = mapDimension.GetWidth() * ((float)xGrid / gridDimension.GetWidth() - 0.5f);
	zMap = mapDimension.GetHeight() * (0.5f - (float)yGrid / gridDimension.GetHeight());
}

unsigned int CCollisionMap::GetWidth()
{
	return m_oCollisionTexture.m_nWidth;
}

unsigned int CCollisionMap::GetHeight()
{
	return m_oCollisionTexture.m_nHeight;
}

void CCollisionMap::ModelToGrid(int xMap, int zMap, const CDimension& mapDimension, const CDimension& gridDimension, int& xGrid, int& yGrid)
{
	xGrid = xMap * gridDimension.GetWidth() / mapDimension.GetWidth() + gridDimension.GetWidth() / 2.f;
	yGrid = gridDimension.GetHeight() * (0.5f - zMap / mapDimension.GetHeight());
}

void CCollisionMap::CreateTextureFromCollisionArray(string sFileName, const vector<vector<bool>>& vGrid)
{
	m_oCollisionTexture.m_ePixelFormat = ILoader::TPixelFormat::eRGB;
	m_oCollisionTexture.m_nWidth = vGrid.size();
	m_oCollisionTexture.m_nHeight = vGrid[0].size();

	m_oCollisionTexture.m_nWidth = m_oCollisionTexture.m_nWidth % 4 ? (m_oCollisionTexture.m_nWidth >> 2) << 2 : m_oCollisionTexture.m_nWidth;
	m_oCollisionTexture.m_nHeight = m_oCollisionTexture.m_nHeight % 2 ? (m_oCollisionTexture.m_nHeight >> 1) << 1 : m_oCollisionTexture.m_nHeight;

	m_oCollisionTexture.m_sFileName = sFileName;
	m_oCollisionTexture.m_vTexels.resize(m_oCollisionTexture.m_nWidth * m_oCollisionTexture.m_nHeight * 3);

	for (int i = 0; i < m_oCollisionTexture.m_nWidth; i++) {
		for (int j = 0; j < m_oCollisionTexture.m_nHeight; j++) {
			unsigned char color = vGrid[i][j] ? 255 : 0;
			for (int k = 0; k < 3; k++)
				m_oCollisionTexture.m_vTexels[3 * (i + j * m_oCollisionTexture.m_nWidth) + k] = color;
		}
	}

	ILoaderManager* pLoaderManager = dynamic_cast<ILoaderManager*>(m_oInterface.GetPlugin("LoaderManager"));
	pLoaderManager->Export(sFileName, m_oCollisionTexture);
}
