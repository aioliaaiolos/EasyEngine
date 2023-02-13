#include "pathFinder.h"
#include "Interface.h"
#include "Exception.h"
#include "Utils2/TimeManager.h"

#include <string>
#include <vector>
#include <sstream>
#include <fstream>

using namespace std;


#define AVOID_DIAGONALE_OBSTACLE

CGrid::CCell::CCell()
{
	Reset();
}

void CGrid::CCell::AddNodeFlag(TCell type)
{
	int t = (int)(m_eCellType | type);
	m_eCellType = (TCell)t;
}

void CGrid::CCell::RemoveNodeFlag(TCell n)
{
	int nn = ~n;
	int t = m_eCellType & nn;
	m_eCellType = (TCell)t;
}

IGrid::ICell* CGrid::CCell::GetParent()
{
	return m_pParent;
}

int CGrid::CCell::ComputeCoast(int column, int row)
{
	int dc = abs(m_column - column);
	int dr = abs(m_row - row);
	int r = 14 * min(dc, dr) + 10 * (max(dc, dr) - min(dc, dr));
	return r;
}

int CGrid::CCell::ComputeCoastToParent()
{
	return m_pParent->ComputeCoast(m_column, m_row);
}

void CGrid::CCell::Update(CCell* parent, CCell* destination)
{
	m_pParent = parent;
	m_nGCost = parent ? parent->m_nGCost + parent->ComputeCoast(m_column, m_row) : 0;
	m_nHCost = ComputeCoast(destination->m_column, destination->m_row);
	m_nFCost = m_nGCost + m_nHCost;
}

void CGrid::CCell::SetParent(CCell* parent)
{
	m_pParent = parent;
}

bool CGrid::CCell::IsInTheSameCell(CCell& cell)
{
	return (m_column == cell.m_column) && (m_row == cell.m_row);
}

void CGrid::CCell::GetCoordinates(int& row, int& column) const
{
	column = m_column;
	row = m_row;
}

void CGrid::CCell::SetCoordinates(int row, int column)
{
	m_column = column;
	m_row = row;
}

IGrid::ICell::TCell CGrid::CCell::GetCellType() const
{
	return m_eCellType;
}

void CGrid::CCell::Init(CCell& cell)
{
	m_row = cell.m_row;
	m_column = cell.m_column;
	m_nFCost = cell.m_nFCost;
	m_nGCost = cell.m_nGCost;
	m_nHCost = cell.m_nHCost;
	m_pParent = cell.m_pParent;
	m_eCellType = cell.m_eCellType;
}

void CGrid::CCell::Reset()
{
	m_eCellType = eUninitialized;
	m_pParent = NULL;
	m_nGCost = 0;
	m_nHCost = 0;
	m_nFCost = 0;
}

void CGrid::CCell::ResetAllExceptObstacles()
{
	if (m_eCellType != eObstacle) {
		m_eCellType = eUninitialized;
		m_pParent = NULL;
		m_nGCost = 0;
		m_nHCost = 0;
		m_nFCost = 0;
	}
}

CGrid::CCell& CGrid::CCell::operator >> (ofstream& stream)
{
	int cellType = (m_eCellType & ~eOpen) & ~eClose;
	stream.write((char*)&cellType, sizeof(int));
	stream.write((char*)&m_nGCost, sizeof(int));
	stream.write((char*)&m_nHCost, sizeof(int));
	stream.write((char*)&m_nFCost, sizeof(int));
	return *this;
}

CGrid::CCell& CGrid::CCell::operator << (ifstream& stream)
{
	int cellType = eUninitialized;
	stream.read((char*)&cellType, sizeof(int));
	stream.read((char*)&m_nGCost, sizeof(int));
	stream.read((char*)&m_nHCost, sizeof(int));
	stream.read((char*)&m_nFCost, sizeof(int));
	m_eCellType = (TCell)cellType;
	return *this;
}

int CGrid::CCell::GetFCost() const
{
	return m_nFCost;
}

int CGrid::CCell::GetGCost() const
{
	return m_nGCost;
}

int CGrid::CCell::GetHCost() const
{
	return m_nHCost;
}

CGrid::CGrid(EEInterface& oInterface, int rowCount, int columnCount) :
m_nRowCount(rowCount),
m_nColumnCount(columnCount),
m_pDepart(NULL),
m_pDestination(NULL),
m_bManualMode(false),
m_oTimeManager(static_cast<CTimeManager&>(*oInterface.GetPlugin("TimeManager")))
{
	Init();
}

void CGrid::Init()
{
	m_grid.resize(m_nRowCount);
	for (int row = 0; row < m_nRowCount; row++)
		m_grid[row].resize(m_nColumnCount);

	for (int row = 0; row < m_nRowCount; row++)
		for (int column = 0; column < m_nColumnCount; column++)
			m_grid[row][column].SetCoordinates(row, column);
}

void CGrid::Reset()
{
	m_vOpenList.clear();
	m_vCloseList.clear();
	m_vPath.clear();

	for (int row = 0; row < m_nRowCount; row++)
		for (int column = 0; column < m_nColumnCount; column++)
			m_grid[row][column].Reset();

	m_pDepart = NULL;
	m_pDestination = NULL;
}

void CGrid::ResetAllExceptObstacles()
{
	int r = -1, c = -1;
	for (ICell* cell : m_vOpenList) {
		cell->GetCoordinates(r, c);
		m_grid[r][c].ResetAllExceptObstacles();
	}
	for (ICell* cell : m_vCloseList) {
		cell->GetCoordinates(r, c);
		m_grid[r][c].ResetAllExceptObstacles();
	}

	m_vOpenList.clear();
	m_vCloseList.clear();
	m_vPath.clear();

	m_pDepart = NULL;
	m_pDestination = NULL;
}


void CGrid::Save(string sFileName, int xMin, int yMin, int xMax, int yMax)
{
	/* format
	 row_count(int), 
	 column_count(int),
	 originRow(int),
	 originColumn,	 
	 destinationRow(int),
	 destinationColumn(int),
	 Foreach row{
		Foreach column{
			type(int),
			GCost(int),
			HCost(int),
			FCost(int)
		}
	 } */

	if(xMin >= 0 && yMin >= 0 && xMax >=0 && yMax)
		SavePart(sFileName, xMin, yMin, xMax, yMax);
	else {
		ofstream file;
		file.open(sFileName.c_str(), ios::out | ios::binary);
		int originRow, originColumn, destinationRow, destinationColumn;
		m_pDepart->GetCoordinates(originRow, originColumn);
		m_pDestination->GetCoordinates(destinationRow, destinationColumn);
		file.write((char*)&m_nRowCount, sizeof(int));
		file.write((char*)&m_nColumnCount, sizeof(int));
		file.write((char*)&originRow, sizeof(int));
		file.write((char*)&originColumn, sizeof(int));
		file.write((char*)&destinationRow, sizeof(int));
		file.write((char*)&destinationColumn, sizeof(int));
		for (int row = 0; row < m_nRowCount; row++)
			for (int column = 0; column < m_nColumnCount; column++)
				m_grid[row][column] >> file;
		file.close();
	}
}


void CGrid::SavePart(string sFileName, int xMin, int yMin, int xMax, int yMax)
{
	/* 
	format
	row_count(int),
	column_count(int),
	originRow(int),
	originColumn,
	destinationRow(int),
	destinationColumn(int),
	Foreach row{
		Foreach column{
			type(int),
			GCost(int),
			HCost(int),
			FCost(int)
		}
	} 
	*/

	ofstream file;
	file.open(sFileName.c_str(), ios::out | ios::binary);

	int originRow, originColumn, destinationRow, destinationColumn;
	
	m_pDepart->GetCoordinates(originRow, originColumn);
	m_pDestination->GetCoordinates(destinationRow, destinationColumn);
	int newRepereRow = originRow < destinationRow ? originRow : destinationRow;
	newRepereRow -= yMin;
	int newRepereColumn = originColumn < destinationColumn ? originColumn : destinationColumn;	
	newRepereColumn -= xMin;

	int newOriginRow = originRow - newRepereRow;
	int newOriginColumn = originColumn - newRepereColumn;
	int newDestinationRow = destinationRow - newRepereRow;
	int newDestinationColumn = destinationColumn - newRepereColumn;
	int newRowCount = newOriginRow > newDestinationRow ? newOriginRow + 1 : newDestinationRow + 1;
	newRowCount += yMax;
	if (newRowCount + newRepereRow > m_grid.size()) {
		newRowCount = m_grid.size() - newRepereRow;
	}
	int newColumnCount = newOriginColumn > newDestinationColumn ? newOriginColumn + 1 : newDestinationColumn + 1;
	newColumnCount += xMax;
	if (newColumnCount + newRepereColumn > m_grid[0].size()) {
		newColumnCount = m_grid.size() - newRepereColumn;
	}

	file.write((char*)&newRowCount, sizeof(int));
	file.write((char*)&newColumnCount, sizeof(int));
	file.write((char*)&newOriginRow, sizeof(int));
	file.write((char*)&newOriginColumn, sizeof(int));
	file.write((char*)&newDestinationRow, sizeof(int));
	file.write((char*)&newDestinationColumn, sizeof(int));
	for (int row = 0; row < newRowCount; row++) {
		for (int column = 0; column < newColumnCount; column++) {
			m_grid[newRepereRow + row][newRepereColumn + column] >> file;
		}
	}
	file.close();
}


void CGrid::Load(string sFileName)
{
	m_grid.resize(0);
	m_vOpenList.clear();
	m_vCloseList.clear();
	m_vPath.clear();
	m_pDepart = NULL;
	m_pDestination = NULL;

	ifstream file;
	file.open(sFileName.c_str(), ios::in | ios::binary);
	int originRow, originColumn, destinationRow, destinationColumn;
	file.read((char*)&m_nRowCount, sizeof(int));
	file.read((char*)&m_nColumnCount, sizeof(int));
	file.read((char*)&originRow, sizeof(int));
	file.read((char*)&originColumn, sizeof(int));
	file.read((char*)&destinationRow, sizeof(int));
	file.read((char*)&destinationColumn, sizeof(int));

	Init();

	for (int row = 0; row < m_nRowCount; row++)
		for (int column = 0; column < m_nColumnCount; column++)
			m_grid[row][column] << file;

	SetDepart(originColumn, originRow);
	SetDestination(destinationColumn, destinationRow);

	file.close();
}

void CGrid::SetManualMode(bool manual)
{
	m_bManualMode = manual;
}

bool CGrid::GetManualMode()
{
	return m_bManualMode;
}

void CGrid::AddObstacle(int row, int column)
{
	m_grid[row][column].AddNodeFlag(CCell::eObstacle);
}

void CGrid::RemoveObstacle(int row, int column)
{
	m_grid[row][column].RemoveNodeFlag(CCell::eObstacle);
}

IGrid::ICell& CGrid::FindClosestNonObstacle(int row, int column)
{
	int range = 2;
	for (int iRow = -range; iRow <= range; iRow++) {
		for (int iColumn = -range; iColumn <= range; iColumn++) {
			ICell& cell = m_grid[row + iRow][column + iColumn];
			if ( !(cell.GetCellType() & IGrid::ICell::eObstacle) ) {
				return cell;
			}
		}
	}
	CEException e("Error in CGrid::FindClosestNonObstacle() : no path found");
	throw e;
}

void CGrid::SetDepart(int column, int row)
{
	if(m_pDepart)
		m_pDepart->RemoveNodeFlag(CCell::eDepart);
	m_pDepart = &m_grid[row][column];
	m_pDepart->AddNodeFlag(CCell::eDepart);
	
}

void CGrid::SetDestination(int column, int row)
{
	if (column < 0 || row < 0 || column > m_nColumnCount || row > m_nRowCount) {
		ostringstream oss;
		oss << "Error CGrid::SetDestination : column = " << column << ", row = " << row << " and column count = " << m_nColumnCount << " and rowCount = " << m_nRowCount;
		CEException e(oss.str());
		throw e;
	}
	if(m_pDestination)
		m_pDestination->RemoveNodeFlag(CCell::eArrivee);
	m_pDestination = &m_grid[row][column];
	if (m_pDestination->GetCellType() & ICell::eObstacle) {
		m_pDestination = (CCell*)&FindClosestNonObstacle(row, column);
	}
	m_pDestination->AddNodeFlag(CCell::eArrivee);	
}

IGrid::ICell* CGrid::GetDepart()
{
	return m_pDepart;
}

IGrid::ICell* CGrid::GetDestination()
{
	return m_pDestination;
}

int CGrid::RowCount() const
{
	return m_nRowCount;
}

int CGrid::ColumnCount() const
{
	return m_nColumnCount;
}

void CGrid::UpdateOpenList(CCell& cell)
{
	for (vector<ICell*>::iterator it = m_vOpenList.begin(); it != m_vOpenList.end(); it++) {
		CCell* c = (CCell*)(*it);
		if ( cell.IsInTheSameCell(*c)  && (cell.GetHCost() < c->GetHCost()) ) {
			c->Update((CCell*)cell.GetParent(), m_pDestination);
		}
	}
}

bool CGrid::ProcessGrid(int startRow, int startColumn)
{
	int currentRow = startRow, currentColumn = startColumn, nextRow = -1, nextColumn = -1;
	bool complete = false;
	int t1 = m_oTimeManager.GetCurrentTimeInMillisecond();
	while (!complete) {
		complete = ProcessNode(currentRow, currentColumn, nextRow, nextColumn);
		currentRow = nextRow;
		currentColumn = nextColumn;
		int t2 = m_oTimeManager.GetCurrentTimeInMillisecond();
		if (t2 - t1 > 0)
			return false;
	}
}

bool CGrid::ProcessNode(int currentRow, int currentColumn, int& nextRow, int& nextColumn)
{
	if (currentRow < 0 || currentColumn < 0) {
		CEException e("Error in CGrid::ProcessNode(), currentRow and currentColumn has to be positive or null");
		throw e;
	}
	CCell& current = m_grid[currentRow][currentColumn];
	if (current.GetCellType() & IGrid::ICell::eArrivee)
		return true;
	if (current.GetCellType() & ICell::eClose)
		return false;
	if (m_grid[currentRow][currentColumn].GetCellType() == IGrid::ICell::eDepart) {
		current.Update(NULL, m_pDestination);
	}
	current.RemoveNodeFlag(IGrid::ICell::eOpen);
	current.AddNodeFlag(IGrid::ICell::eClose);
	m_vCloseList.push_back(&current);
	int index = GetNodeInOpenList(currentRow, currentColumn);
	if(index != -1)
		m_vOpenList.erase(m_vOpenList.begin() + index);
	for (int iRow = -1; iRow <= 1; iRow++) {
		for (int iColumn = -1; iColumn <= 1; iColumn++) {
			if ( (iColumn == 0) && (iRow == 0) )
				continue;
			if ( (currentColumn + iColumn < 0) || ( (currentColumn + iColumn) >= m_nColumnCount ) )
				continue;
			if ( (currentRow + iRow < 0) || ((currentRow + iRow) >= m_nRowCount) )
				continue;
			CCell& cell = m_grid[currentRow + iRow][currentColumn + iColumn];
			if (cell.GetCellType() & CCell::eObstacle)
				continue;
			
#ifdef AVOID_DIAGONALE_OBSTACLE
			if (iRow != 0 && iColumn != 0) {
				if ( (m_grid[currentRow][currentColumn + iColumn].GetCellType() & CCell::eObstacle) ||
					(m_grid[currentRow + iRow][currentColumn].GetCellType() & CCell::eObstacle) ) {
					continue;
				}
			}
#endif // AVOID_DIAGONALE_OBSTACLE

			if ( (cell.GetCellType() == CCell::eUninitialized) || (cell.GetCellType() == CCell::eArrivee) ) {
				cell.Update(&current, (CCell*)m_pDestination);
				InsertToOpenV2(&cell);
			}
			else {
				CCell cell2;
				cell2.Init(cell);
				cell2.Update(&current, m_pDestination);
				if (cell2.GetFCost() < cell.GetFCost()) {
					cell.Init(cell2);
					UpdateOpenList(cell);
				}
			}
		}
	}

	if (m_bManualMode)
		return true;

	vector<ICell*>::iterator itBest = m_vOpenList.begin();
	if (itBest != m_vOpenList.end()) {
		ICell* cell = *itBest;
		m_vCloseList.push_back(cell);
		m_vOpenList.erase(itBest);
		int r, c;
		cell->GetCoordinates(r, c);
		nextRow = r;
		nextColumn = c;
	}
	return false;
}

void CGrid::BuildPath()
{
	ICell* cell = m_pDestination;
	while (cell) {
		m_vPath.insert(m_vPath.begin(), cell);
		cell = cell->GetParent();
	}
}

void CGrid::GetOpenList(vector<ICell*>& openList)
{
	openList = m_vOpenList;
}

void CGrid::GetCloseList(vector<ICell*>& closeList)
{
	closeList = m_vCloseList;
}

void CGrid::GetPath(vector<ICell*>& path)
{
	path = m_vPath;
}

IGrid::ICell& CGrid::GetCell(int row, int column)
{
	return m_grid[row][column];
}

int CGrid::GetNodeInOpenList(int row, int column)
{
	for (int i = 0; i < m_vOpenList.size(); i++) {
		int r, c;
		m_vOpenList[i]->GetCoordinates(r, c);
		if (r == row && c == column)
			return i;
	}
	return -1;
}

void CGrid::InsertToOpen(CCell* b)
{
	if ((b->GetCellType() == IGrid::ICell::eUninitialized) || (b->GetCellType() == IGrid::ICell::eArrivee))
		b->AddNodeFlag(IGrid::ICell::eOpen);
	for (vector<CGrid::ICell*>::iterator it = m_vOpenList.begin(); it != m_vOpenList.end(); it++) {
		CCell* pBox = static_cast<CCell*>(*it);
		if (b->GetFCost() < pBox->GetFCost()) {
			m_vOpenList.insert(it, b);
			return;
		}
	}
	m_vOpenList.push_back(b);
}

void CGrid::InsertToOpenV2(CCell* b)
{
	if ((b->GetCellType() == IGrid::ICell::eUninitialized) || (b->GetCellType() == IGrid::ICell::eArrivee))
		b->AddNodeFlag(IGrid::ICell::eOpen);
	for (vector<CGrid::ICell*>::iterator it = m_vOpenList.begin(); it != m_vOpenList.end(); it++) {
		CCell* pBox = static_cast<CCell*>(*it);
		if ( (b->GetFCost() < pBox->GetFCost() )  ||
			(b->GetFCost() == pBox->GetFCost()) && (b->GetHCost() < pBox->GetHCost()) ) {
			m_vOpenList.insert(it, b);
			return;
		}		
	}
	m_vOpenList.push_back(b);
}

void CGrid::InsertToClose(CCell* b)
{

}


CPathFinder::CPathFinder(EEInterface& oInterface) :
	m_bSaveAStarGrid(false),
	m_xMinMargin(-1),
	m_yMinMargin(-1),
	m_xMaxMargin(-1),
	m_yMaxMargin(-1),
	m_oInterface(oInterface)
{
}

IGrid* CPathFinder::CreateGrid(int rowCount, int columnCount)
{
	return new CGrid(m_oInterface, rowCount, columnCount);
}

bool CPathFinder::FindPath(IGrid* grid)
{
	if (m_bSaveAStarGrid)
		SaveAStarGrid(grid, m_xMinMargin, m_yMinMargin, m_xMaxMargin, m_yMaxMargin);
	int row, column;
	CGrid* pGrid = static_cast<CGrid*>(grid);
	pGrid->GetDepart()->GetCoordinates(row, column);
	if (!pGrid->ProcessGrid(row, column))
		return false;
	pGrid->BuildPath();
	return true;
}

string CPathFinder::GetName()
{
	return "PathFinder";
}

void CPathFinder::EnableSaveGrid(bool bEnable, int xMinMargin, int yMinMargin, int xMaxMargin, int yMaxMargin)
{
	m_bSaveAStarGrid = bEnable;
	m_xMinMargin = xMinMargin;
	m_yMinMargin = yMinMargin;
	m_xMaxMargin = xMaxMargin;
	m_yMaxMargin = yMaxMargin;
}

void CPathFinder::SaveAStarGrid(IGrid* pGrid, int xMin, int yMin, int xMax, int yMax)
{
	WIN32_FIND_DATAA fd;
	ZeroMemory(&fd, sizeof(fd));
	string fileName;
	HANDLE hFile = FindFirstFileA("..\\Data\\grid*.bin", &fd);
	int index = 0;
	do {
		fileName = fd.cFileName;
		if (!fileName.empty()) {
			int first = strlen("grid");
			int dotPos = fileName.find(".");
			int n = dotPos - first;
			string sIndex = fileName.substr(first, n);
			int i = atoi(sIndex.c_str());
			if (i > index)
				index = i;
		}
	} while (FindNextFileA(hFile, &fd));

	ostringstream oss;
	oss << "..\\Data\\grid" << index + 1 << ".bin";
	pGrid->Save(oss.str(), xMin, yMin, xMax, yMax);
}

extern "C" _declspec(dllexport) CPathFinder* CreatePathFinder(EEInterface& oInterface)
{
	return new CPathFinder(oInterface);
}