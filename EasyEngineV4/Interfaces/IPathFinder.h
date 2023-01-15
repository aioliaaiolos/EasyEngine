#ifndef IIAMANAGER_H
#define IIAMANAGER_H

#include "EEPlugin.h"

#include <vector>

using namespace std;

class IGrid
{
public:
	class ICell
	{
	public:
		enum TCell
		{
			eUninitialized = 0,	
			eEmpty = 1,
			eObstacle = 2,
			eDepart = 4,
			eArrivee = 8,
			eOpen = 16,
			eClose = 32
		};

		virtual TCell GetCellType() const = 0;
		virtual void GetCoordinates(int& row, int& column) const = 0;
		virtual int GetFCost() const = 0;
		virtual int GetGCost() const = 0;
		virtual int GetHCost() const = 0;
		virtual ICell* GetParent() = 0;
	};

	virtual int RowCount() const = 0;
	virtual int ColumnCount() const = 0;
	virtual void AddObstacle(int row, int column) = 0;
	virtual void RemoveObstacle(int row, int column) = 0;
	virtual void SetDepart(int x, int y) = 0;
	virtual void SetDestination(int x, int y) = 0;
	virtual IGrid::ICell* GetDepart() = 0;
	virtual IGrid::ICell* GetDestination() = 0;
	virtual void GetOpenList(vector<ICell*>& openList) = 0;
	virtual void GetCloseList(vector<ICell*>& closeList) = 0;
	virtual void GetPath(vector<ICell*>& path) = 0;
	virtual ICell& GetCell(int row, int column) = 0;
	virtual bool ProcessNode(int row, int column, int& nextRow, int& nextColumn) = 0;
	virtual void Reset() = 0;
	virtual void ResetAllExceptObstacles() = 0;
	virtual void Save(string sFileName, int xMin = -1, int yMin = -1, int xMax = -1, int yMax = -1) = 0;
	virtual void Load(string sFileName) = 0;
	virtual bool GetManualMode() = 0;
	virtual void SetManualMode(bool manual) = 0;
};

class IPathFinder : public CPlugin
{
public:

	IPathFinder() : CPlugin(nullptr, "") {}

	virtual IGrid* CreateGrid(int rowCount, int columnCount) = 0;
	virtual bool FindPath(IGrid* grid) = 0;
	virtual void EnableSaveGrid(bool bEnable, int xMinMargin, int yMinMargin, int xMaxMargin, int yMaxMargin) = 0;

protected:
	IPathFinder(const Desc& oDesc) : CPlugin(oDesc.m_pParent, oDesc.m_sName) {}
	
};


#endif // IIAMANAGER_H