#pragma once

#include <vector>

#include "Interface.h"
#include "ILoader.h"
#include "ICollisionManager.h"
#include "../Utils2/Dimension.h"
#include "../Utils/Math/Matrix.h"
#include "../Utils/Math/Vector.h"

using namespace std;

class IEntity;
class INode;

class CCollisionMap : public ICollisionMap
{
public:
	// Collision map
	CCollisionMap(EEInterface& oInterface, IEntity* pScene, int nCellSize);

	void SetFileName(string sFileName) override;

protected:

	void		GetCellCoordFromPosition(float x, float y, int& cellx, int& celly) override;
	void		GetPositionFromCellCoord(int row, int column, float& x, float& y) override;
	void		ComputeRowAndColumnCount(int& rowCount, int& columnCount, int nCellSize);
	bool		TestCellObstacle(int x, int y) override;
	void		ConvertLinearToCoord(int pixelNumber, int nTextureWidth, int& x, int& y);
	void		CreateCollisionArray(IEntity* pRoot, vector<vector<bool>>& vGrid, int nCelSize, float fBias);
	void		AddObjectToCollisionGrid(const CVector& rootDim, const CDimension& gridDimension, const CVector& objectDim, const CMatrix& modelTM, vector<vector<bool>>& vGrid, int nCelSize, bool isObstacle, float fBias);
	void		GetCollisionEntities(IEntity* pRoot, vector<IEntity*>& vCollisionEntities);
	void		SortObjectsByHeight(IEntity* pRoot, vector<pair<float, IEntity*>>& vSortedObjectHeight);
	float		GetFloors(const vector<pair<float, IEntity*>>& vSortedObjectHeight, vector<pair<float, IEntity*>>& floors, vector<pair<float, IEntity*>>& nonFloors);
	void		GetRoofs(float fFloorHeight, const vector<pair<float, IEntity*>>& nonFloors, vector<IEntity*>& roofs, vector<IEntity*>& nonRoofs);
	void		GetDoors(const vector<IEntity*>& vCollisionEntities, vector<IEntity*>& doors);
	void		CreateTextureFromCollisionArray(string sFileName, const vector<vector<bool>>& vGrid);
	void		ModelToGrid(int xMap, int zMap, const CDimension& mapDimension, const CDimension& gridDimension, int& xGrid, int& yGrid);
	void		GridToMap(int xGrid, int yGrid, const CDimension& mapDimension, const CDimension& gridDimension, float& xMap, float& zMap);
	unsigned	int GetWidth() override;
	unsigned	int GetHeight() override;
	void		Generate() override;
	void		Load() override;

private:
	float						m_fGroundMapWidth;
	float						m_fGroundMapHeight;
	float						m_fWorldToScreenScaleFactor;
	float						m_fGridHeight;
	char**						m_pCollisionGrid;
	vector<IEntity*>			m_vGridElements;
	float						m_fGroundWidth;
	float						m_fGroundHeight;
	int							m_nCellSize;
	EEInterface&				m_oInterface;
	ILoaderManager&				m_oLoaderManager;
	IEntityManager&				m_oEntityManager;
	IEntity*					m_pScene;
	ILoader::CTextureInfos		m_oCollisionTexture;
	IMesh*						m_pGround;
	string						m_sFileName;
};