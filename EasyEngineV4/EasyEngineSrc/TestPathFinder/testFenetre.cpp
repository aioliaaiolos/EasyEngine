// testFenetre.cpp : Defines the entry point for the application.
//

#include "testFenetre.h"
#include "IPathFinder.h"
#include <map>
#include <string>
#include <commdlg.h>
#include "Interface.h"

using namespace std;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	OnDialogRowColumnEvent(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void				UpdateScrollRange(HWND hWnd);

enum TGameState
{
	eInitDepDest = 0,
	eInitObstacle,
	eStarted
};

// variables
IGrid* grid = NULL;
int g_nRowCount = 5;
int g_nColumnCount = 7;
const int CellSize = 50;
int leftMargin = 10;
int topMargin = 10;
TGameState g_currentState = eInitDepDest;
vector<IGrid::ICell*> g_vObstacles;
IPathFinder* g_pPathFinder = NULL;
COLORREF departColor = 0xffaaaa;
COLORREF destinationColor = 0xffaaaa;
COLORREF openColor = 0x00ff00;
COLORREF closeColor = 0xff;
COLORREF obstacleColor = 0x999999;
COLORREF pathColor = 0x0099ff;


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TESTFENETRE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TESTFENETRE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TESTFENETRE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TESTFENETRE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   EnableScrollBar(hWnd, SB_BOTH, ESB_ENABLE_BOTH);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void InitObstacles()
{
	g_vObstacles.clear();
	for (int row = 0; row < grid->RowCount(); row++) {
		for (int column = 0; column < grid->ColumnCount(); column++) {
			IGrid::ICell& cell = grid->GetCell(row, column);
			if (cell.GetCellType() & IGrid::ICell::eObstacle)
				g_vObstacles.push_back(&cell);
		}
	}
}

void GetRectFromBox(int column, int row, RECT& rect)
{
	rect.left = leftMargin + column * CellSize;
	rect.right = rect.left + CellSize;
	rect.top = topMargin + row * CellSize;
	rect.bottom = rect.top + CellSize;
}

void ColorizeBox(HDC& hdc, int row, int column, COLORREF color)
{
	RECT rect;
	GetRectFromBox(row, column, rect);
	rect.left += 1;
	rect.top += 1;
	HBRUSH hbr = CreateSolidBrush(color);
	SelectObject(hdc, hbr);
	FillRect(hdc, &rect, hbr);
	DeleteObject(hbr);
}

void LabelizeBox(HDC& hdc, char* text, int backgroundColor, int column, int row)
{
	RECT rect;
	GetRectFromBox(column, row, rect);
	SetBkColor(hdc, backgroundColor);
	TEXTMETRICA m;
	GetTextMetricsA(hdc, &m);
	
	HFONT hFont = CreateFontA(12, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "SYSTEM_FIXED_FONT");
	SelectObject(hdc, hFont);

	TextOutA(hdc, rect.left + CellSize / 2 - m.tmAveCharWidth / 2, rect.top + CellSize / 2 - m.tmHeight / 2, text, strlen(text));
	DeleteObject(hFont);
	
}

void LabelizeBoxCost(HDC& hdc, IGrid::ICell* cell, int backgroundColor, int column, int row)
{
	int margin = 3;
	RECT rect;
	GetRectFromBox(column, row, rect);
	SetBkColor(hdc, backgroundColor);
	TEXTMETRICA m;
	GetTextMetricsA(hdc, &m);
	cell->GetGCost();
	char f[5], g[5], h[5];
	_itoa_s(cell->GetFCost(), f, 10);
	_itoa_s(cell->GetGCost(), g, 10);
	_itoa_s(cell->GetHCost(), h, 10);

	int hSize = strlen(h);
	int totalHSize = CellSize - m.tmAveCharWidth * strlen(h) - margin;

	HFONT hFont = CreateFontA(12, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "SYSTEM_FIXED_FONT");
	SelectObject(hdc, hFont);

	TextOutA(hdc, rect.left + margin, rect.top + margin, g, strlen(g));
	TextOutA(hdc, rect.left + totalHSize, rect.top + margin, h, strlen(h));
	TextOutA(hdc, rect.left + CellSize / 2 - strlen(f) * m.tmAveCharWidth/2, rect.top + CellSize - m.tmHeight, f, strlen(f));

	DeleteObject(hFont);
}

void GetCellFromCoordinates(int x, int y, int& row, int& column)
{
	column = (x - leftMargin) / CellSize;
	row = (y - topMargin) / CellSize;
	if ((column >= g_nColumnCount) || row >= g_nRowCount) {
		row = column = -1;
	}
}



HBRUSH obstacleBrush;
POINT g_lastMouseClick;

#include <algorithm>

void ToggleObstacle(IGrid::ICell* cell)
{
	vector<IGrid::ICell*>::iterator it = std::find(g_vObstacles.begin(), g_vObstacles.end(), cell);
	int row, column;
	cell->GetCoordinates(row, column);
	if (it == g_vObstacles.end()) {
		g_vObstacles.push_back(cell);
		grid->AddObstacle(row, column);
	}
	else {
		g_vObstacles.erase(it);
		grid->RemoveObstacle(row, column);
	}
}

int MixColor(int color1, int color2, float mixFactor)
{
	int r1 = color1 & 0xff;
	int g1 = (color1 & 0xff00) >> 8;
	int b1 = (color1 & 0xff0000) >> 16;

	int r2 = color2 & 0xff;
	int g2 = (color2 & 0xff00) >> 8;
	int b2 = (color2 & 0xff0000) >> 16;

	int r = (int)(mixFactor * (float)r1 + (1.f - mixFactor) * (float)r2);
	int g = (int)(mixFactor * (float)g1 + (1.f - mixFactor) * (float)g2);
	int b = (int)(mixFactor * (float)b1 + (1.f - mixFactor) * (float)b2);
	int color = r + (g << 8) + (b << 16);
	return color;
}

void DisplayArrow(HDC& hdc, int rowDepart, int columnDepart, int rowDestination, int columnDestination)
{
	RECT rectDepart, rectDestination;
	GetRectFromBox(columnDepart, rowDepart, rectDepart);
	GetRectFromBox(columnDestination, rowDestination, rectDestination);

	int res = 10;
	int tipSize = 3 * res / 4;

	if ((rowDepart - rowDestination) > 0) {
		// haut
		if ((columnDepart - columnDestination) > 0) {
			// haut gauche
			MoveToEx(hdc, rectDepart.left + res, rectDepart.top + res, LPPOINT(NULL));
			LineTo(hdc, rectDestination.right - res, rectDestination.bottom - res);
			// tip
			MoveToEx(hdc, rectDestination.right - res, rectDestination.bottom - res, LPPOINT(NULL));
			LineTo(hdc, rectDestination.right - res, rectDestination.bottom - res + tipSize);
			MoveToEx(hdc, rectDestination.right - res, rectDestination.bottom - res, LPPOINT(NULL));
			LineTo(hdc, rectDestination.right - res + tipSize, rectDestination.bottom - res);
		}
		else if ((columnDepart - columnDestination) == 0) {
			// haut haut
			MoveToEx(hdc, rectDepart.left + CellSize / 2, rectDepart.top + res, LPPOINT(NULL));
			LineTo(hdc, rectDepart.left + CellSize / 2, rectDestination.bottom - res);
			// tip
			MoveToEx(hdc, rectDepart.left + CellSize / 2, rectDestination.bottom - res, LPPOINT(NULL));
			LineTo(hdc, rectDepart.left + CellSize / 2 - tipSize, rectDestination.bottom - res + tipSize);
			MoveToEx(hdc, rectDepart.left + CellSize / 2, rectDestination.bottom - res, LPPOINT(NULL));
			LineTo(hdc, rectDepart.left + CellSize / 2 + tipSize, rectDestination.bottom - res + tipSize);
		}
		else {
			// haut droite
			MoveToEx(hdc, rectDepart.right - res, rectDepart.top + res, LPPOINT(NULL));
			LineTo(hdc, rectDestination.left + res, rectDestination.bottom - res);
			// tip
			MoveToEx(hdc, rectDestination.left + res, rectDestination.bottom - res, LPPOINT(NULL));
			LineTo(hdc, rectDestination.left + res - tipSize, rectDestination.bottom - res);
			MoveToEx(hdc, rectDestination.left + res, rectDestination.bottom - res, LPPOINT(NULL));
			LineTo(hdc, rectDestination.left + res, rectDestination.bottom - res + tipSize);
		}
	}
	else if ((rowDepart - rowDestination) == 0) {
		if (columnDepart - columnDestination > 0) {
			// gauche gauche
			MoveToEx(hdc, rectDepart.left + res, rectDepart.top + CellSize / 2, LPPOINT(NULL));
			LineTo(hdc, rectDestination.right - res, rectDepart.top + CellSize / 2);

			// tip
			MoveToEx(hdc, rectDestination.right - res, rectDepart.top + CellSize / 2, LPPOINT(NULL));
			LineTo(hdc, rectDestination.right - res + tipSize/2, rectDepart.top + CellSize / 2 - tipSize/2);
			MoveToEx(hdc, rectDestination.right - res, rectDepart.top + CellSize / 2, LPPOINT(NULL));
			LineTo(hdc, rectDestination.right - res + tipSize/2, rectDepart.top + CellSize / 2 + tipSize/2);
		}
		else {
			// droite droite
			MoveToEx(hdc, rectDepart.right - res, rectDepart.top + CellSize / 2, LPPOINT(NULL));
			LineTo(hdc, rectDestination.left + res, rectDepart.top + CellSize / 2);

			// tip
			MoveToEx(hdc, rectDestination.left + res, rectDepart.top + CellSize / 2, LPPOINT(NULL));
			LineTo(hdc, rectDestination.left + res - tipSize/2, rectDepart.top + CellSize / 2 - tipSize / 2);
			MoveToEx(hdc, rectDestination.left + res, rectDepart.top + CellSize / 2, LPPOINT(NULL));
			LineTo(hdc, rectDestination.left + res - tipSize / 2, rectDepart.top + CellSize / 2 + tipSize / 2);
		}
	}
	else {
		// bas
		if ((columnDepart - columnDestination) > 0) {
			// bas gauche
			MoveToEx(hdc, rectDepart.left + res, rectDepart.bottom - res, LPPOINT(NULL));
			LineTo(hdc, rectDestination.right - res, rectDestination.top + res);
			// tip
			MoveToEx(hdc, rectDestination.right - res, rectDestination.top + res, LPPOINT(NULL));
			LineTo(hdc, rectDestination.right - res + tipSize, rectDestination.top + res);
			MoveToEx(hdc, rectDestination.right - res, rectDestination.top + res, LPPOINT(NULL));
			LineTo(hdc, rectDestination.right - res, rectDestination.top + res - tipSize);
		}
		else if ((columnDepart - columnDestination) == 0) {
			// bas bas
			MoveToEx(hdc, rectDepart.left + CellSize / 2, rectDepart.bottom - res, LPPOINT(NULL));
			LineTo(hdc, rectDepart.left + CellSize / 2, rectDestination.top + res);
			// tip
			MoveToEx(hdc, rectDepart.left + CellSize / 2, rectDestination.top + res, LPPOINT(NULL));
			LineTo(hdc, rectDepart.left + CellSize / 2 - tipSize / 2, rectDestination.top + res - tipSize);
			MoveToEx(hdc, rectDepart.left + CellSize / 2, rectDestination.top + res, LPPOINT(NULL));
			LineTo(hdc, rectDepart.left + CellSize / 2 + tipSize / 2, rectDestination.top + res - tipSize);
		}
		else {
			// bas droite
			MoveToEx(hdc, rectDepart.right - res, rectDepart.bottom - res, LPPOINT(NULL));
			LineTo(hdc, rectDestination.left + res, rectDestination.top + res);
			// tip
			MoveToEx(hdc, rectDestination.left + res, rectDestination.top + res, LPPOINT(NULL));
			LineTo(hdc, rectDestination.left + res, rectDestination.top + res - tipSize);
			MoveToEx(hdc, rectDestination.left + res, rectDestination.top + res, LPPOINT(NULL));
			LineTo(hdc, rectDestination.left + res - tipSize, rectDestination.top + res);
		}
	}
}

void DisplayGrid(HDC& hdc)
{
	// Draw grid
	for (int i = 0; i < grid->ColumnCount() + 1; i++) {
		MoveToEx(hdc, i * CellSize + leftMargin, topMargin, LPPOINT(NULL));
		LineTo(hdc, i * CellSize + leftMargin, topMargin + CellSize * grid->RowCount());
	}

	for (int i = 0; i < grid->RowCount() + 1; i++) {
		MoveToEx(hdc, leftMargin, topMargin + i * CellSize, LPPOINT(NULL));
		LineTo(hdc, leftMargin + grid->ColumnCount() * CellSize, topMargin + i * CellSize);
	}

	// Draw obstacles
	for (vector<IGrid::ICell*>::iterator it = g_vObstacles.begin(); it != g_vObstacles.end(); it++) {
		IGrid::ICell* cell = (*it);
		if (cell->GetCellType() & IGrid::ICell::eObstacle) {
			int row, column;
			cell->GetCoordinates(row, column);
			ColorizeBox(hdc, column, row, obstacleColor);
		}
	}

	// Draw open list
	vector<IGrid::ICell*> openList;
	grid->GetOpenList(openList);
	for (vector<IGrid::ICell*>::iterator it = openList.begin(); it != openList.end(); it++) {
		IGrid::ICell* cell = (*it);
		int row, column;
		cell->GetCoordinates(row, column);
		ColorizeBox(hdc, column, row, openColor);
		LabelizeBoxCost(hdc, cell, openColor, column, row);
	}

	// Draw close list
	vector<IGrid::ICell*> closeList;
	grid->GetCloseList(closeList);
	for (vector<IGrid::ICell*>::iterator it = closeList.begin(); it != closeList.end(); it++) {
		IGrid::ICell* cell = (*it);
		int row, column;
		cell->GetCoordinates(row, column);
		ColorizeBox(hdc, column, row, closeColor);
		LabelizeBoxCost(hdc, cell, closeColor, column, row);
	}

	// Draw path
	vector<IGrid::ICell*> vPath;
	grid->GetPath(vPath);
	for (vector<IGrid::ICell*>::iterator it = vPath.begin(); it != vPath.end(); it++) {
		IGrid::ICell* cell = (*it);
		int row, column;
		cell->GetCoordinates(row, column);
		ColorizeBox(hdc, column, row, pathColor);
		LabelizeBoxCost(hdc, cell, pathColor, column, row);
	}

	int row, column;
	// Marque Depart
	float mixFactor = 0.7f;
	IGrid::ICell* depart = grid->GetDepart();
	if (depart) {
		depart->GetCoordinates(row, column);
		int departColor2 = departColor;
		if (depart->GetCellType() & IGrid::ICell::eOpen)
			departColor2 = MixColor(departColor, openColor, mixFactor);
		else if (depart->GetCellType() & IGrid::ICell::eClose)
			departColor2 = MixColor(departColor, closeColor, mixFactor);
		ColorizeBox(hdc, column, row, departColor2);
		LabelizeBox(hdc, "D", departColor2, column, row);
		LabelizeBoxCost(hdc, depart, departColor2, column, row);
	}

	// Marque arrivee
	IGrid::ICell* destination = grid->GetDestination();
	if (destination) {
		destination->GetCoordinates(row, column);
		int destinationColor2 = destinationColor;
		if (destination->GetCellType() & IGrid::ICell::eOpen)
			destinationColor2 = MixColor(destinationColor, openColor, mixFactor);
		ColorizeBox(hdc, column, row, destinationColor2);
		LabelizeBox(hdc, "A", destinationColor2, column, row);
		LabelizeBoxCost(hdc, destination, destinationColor2, column, row);
	}
	

	// Draw arrows
	for (vector<IGrid::ICell*>::iterator it = openList.begin(); it != openList.end(); it++) {
		IGrid::ICell* cell = (*it);
		if (cell->GetParent()) {
			int r1, c1, r2, c2;
			cell->GetCoordinates(r1, c1);
			cell->GetParent()->GetCoordinates(r2, c2);
			DisplayArrow(hdc, r1, c1, r2, c2);
		}
	}
	for (vector<IGrid::ICell*>::iterator it = closeList.begin(); it != closeList.end(); it++) {
		IGrid::ICell* cell = (*it);
		if (cell->GetParent()) {
			int r1, c1, r2, c2;
			cell->GetCoordinates(r1, c1);
			cell->GetParent()->GetCoordinates(r2, c2);
			DisplayArrow(hdc, r1, c1, r2, c2);
		}
	}

}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//

void Reset(HWND hWnd, bool resetObstacles)
{
	if (resetObstacles) {
		grid->Reset();
		g_vObstacles.clear();
	}
	else
		grid->ResetAllExceptObstacles();
	g_currentState = eInitDepDest;
	InvalidateRect(hWnd, NULL, TRUE);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	EEInterface* pInterface = new EEInterface;
	CPlugin::SetEngineInterface(pInterface);

	int x, y, row, column, nextRow, nextColumn;
	switch (message)
	{
	case WM_CREATE:
	{
		string sDirectoryName;
#ifdef _DEBUG
		sDirectoryName = "..\\..\\EasyEngine\\Debug\\";
#else
		sDirectoryName = "..\\..\\EasyEngine\\release\\";
#endif
		IPathFinder::Desc desc(NULL, "PathFinder");
		g_pPathFinder = (IPathFinder*)CPlugin::Create(desc, sDirectoryName + "IA.dll", "CreatePathFinder");
		grid = g_pPathFinder->CreateGrid(g_nRowCount, g_nColumnCount);
		obstacleBrush = CreateSolidBrush(obstacleColor);
		UpdateScrollRange(hWnd);
		break;
	}
	case WM_VSCROLL:
	{
		int pos = 0, d = 0;
		if (LOWORD(wParam) == SB_THUMBTRACK) {
			pos = HIWORD(wParam);
			int lastPos = GetScrollPos(hWnd, SB_VERT);
			d = pos - lastPos;
		}
		else {
			int d = 0;
			int scrollPos;
			switch (wParam) {
			case SB_LINEUP:
				d = -10;
				break;
			case SB_LINEDOWN:
				d = 10;
				break;
			case SB_PAGEUP:
				d = -50;
				break;
			case SB_PAGEDOWN:
				d = 50;
				break;
			}

			scrollPos = GetScrollPos(hWnd, SB_VERT);
			int min, max;
			pos = scrollPos + d;
			GetScrollRange(hWnd, SB_VERT, &min, &max);
			if (pos < min) {
				pos = min;
				d = min - scrollPos;
			}
			if (pos > max) {
				pos = max;
				d = max - scrollPos;
			}
		}
		SetScrollPos(hWnd, SB_VERT, pos, TRUE);
		ScrollWindow(hWnd, 0, -d, NULL, NULL);
		InvalidateRect(hWnd, NULL, TRUE);
		topMargin -= d;		
		break;
	}
	case WM_HSCROLL: {
		int pos = 0, d = 0;
		if (LOWORD(wParam) == SB_THUMBTRACK) {
			pos = HIWORD(wParam);
			int lastPos = GetScrollPos(hWnd, SB_HORZ);
			d = pos - lastPos;
		}
		else {
			int d = 0;
			int scrollPos;
			switch (wParam) {
			case SB_LINELEFT:
				d = -10;
				break;
			case SB_LINERIGHT:
				d = 10;
				break;
			case SB_PAGELEFT:
				d = -50;
				break;
			case SB_PAGERIGHT:
				d = 50;
				break;
			}

			scrollPos = GetScrollPos(hWnd, SB_HORZ);
			int min, max;
			pos = scrollPos + d;
			GetScrollRange(hWnd, SB_HORZ, &min, &max);
			if (pos < min) {
				pos = min;
				d = min - scrollPos;
			}
			if (pos > max) {
				pos = max;
				d = max - scrollPos;
			}
		}
		SetScrollPos(hWnd, SB_HORZ, pos, TRUE);
		ScrollWindow(hWnd, 0, -d, NULL, NULL);
		InvalidateRect(hWnd, NULL, TRUE);
		leftMargin -= d;
		break;
	}
	case WM_LBUTTONDOWN:
		x = LOWORD(lParam);
		y = HIWORD(lParam);

		GetCellFromCoordinates(x, y, row, column);
		if (row != -1 && column != -1) {
			switch (g_currentState) {
			case eInitDepDest:
				if (!grid->GetDepart())
					grid->SetDepart(column, row);
				else {
					grid->SetDestination(column, row);
					g_currentState = eInitObstacle;
				}
				break;
			case eInitObstacle:
			{
				int r, c;
				grid->GetDepart()->GetCoordinates(r, c);
				if ((c == column) && (r == row)) {
					g_currentState = eStarted;
					g_pPathFinder->FindPath(grid);
				}
				else {
					ToggleObstacle(&grid->GetCell(row, column));
				}
				break;
			}
			case eStarted:
				if (grid->GetManualMode())					
					grid->ProcessNode(row, column, nextRow, nextColumn);
				break;
			}
			InvalidateRect(hWnd, NULL, TRUE);
		}

		ShowWindow(hWnd, 10);
		UpdateWindow(hWnd);
		break;

	case WM_RBUTTONDOWN:
		grid->ResetAllExceptObstacles();
		g_currentState = eInitDepDest;
		InvalidateRect(hWnd, NULL, TRUE);
		break;

	case WM_MOUSEWHEEL:
	{
		int deltav = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
		deltav *= 20;
		int scrollPos = GetScrollPos(hWnd, SB_VERT);
		int min, max;
		GetScrollRange(hWnd, SB_VERT, &min, &max);
		int pos = scrollPos - deltav;

		if (pos < min) {
			pos = min;
			deltav = scrollPos - pos;
		}
		if (pos > max) {
			pos = max;
			deltav = scrollPos - pos;
		}
		
		SetScrollPos(hWnd, SB_VERT, pos, TRUE);
		ScrollWindow(hWnd, 0, deltav, NULL, NULL);
		topMargin += deltav;
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	}		
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
			char szFile[256] = { 0 };
			OPENFILENAMEA ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lpstrFile = szFile;
			ofn.lStructSize = sizeof(ofn);
			ofn.nMaxFile = sizeof(szFile);
			ofn.hwndOwner = hWnd;
			ofn.lpstrInitialDir = "..\\Data\\";
			ofn.lpstrFilter = "*.bin";
            // Parse the menu selections:
            switch (wmId)
            {
			case ID_FILE_SAVE:				
				GetSaveFileNameA(&ofn);
				strcat_s(szFile, ".bin");
				grid->Save(szFile);
				break;
			case ID_FILE_LOAD:
			{
				GetOpenFileNameA(&ofn);
				if (strlen(szFile) > 0) {
					grid->Load(szFile);
					UpdateScrollRange(hWnd);
					InvalidateRect(hWnd, NULL, TRUE);
					g_currentState = eInitObstacle;
					g_nRowCount = grid->RowCount();
					g_nColumnCount = grid->ColumnCount();
					InitObstacles();
				}
				break;
			}
			case ID_GAME_RESTART:
				Reset(hWnd, true);
				break;
			case ID_GAME_RESTARTEXCEPTOBSTACLES:
				Reset(hWnd, false);
				break;
			case ID_OPTIONS_MANUAL: {
				char name[MAX_PATH];
				GetMenuStringA(GetMenu(hWnd), ID_OPTIONS_MANUAL, name, MAX_PATH, 0);
				grid->SetManualMode(!grid->GetManualMode());
				CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_MANUAL, grid->GetManualMode() ? 0x8 : 0);
				break;
			}
			case ID_OPTIONS_GRIDSIZE: {
				int ret = DialogBoxParamA(GetModuleHandleA(NULL), MAKEINTRESOURCEA(IDD_DIALOG_ROWCOLUMN), hWnd, OnDialogRowColumnEvent, 0);
				if (ret) {
					g_nRowCount = HIWORD(ret);
					g_nColumnCount = LOWORD(ret);
					delete grid;
					grid = g_pPathFinder->CreateGrid(g_nRowCount, g_nColumnCount);
					g_vObstacles.clear();

					UpdateScrollRange(hWnd);
					InvalidateRect(hWnd, NULL, TRUE);
					g_currentState = eInitDepDest;
				}
				break;
			}				
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
			DisplayGrid(hdc);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK OnDialogRowColumnEvent(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char sRowCount[16];
	char sColumnCount[16];
	int rowCount, columnCount, ret = 0;
	HWND hEditRowCount, hEditColumnCount;
	switch (msg) {
	case WM_INITDIALOG:
		SetFocus(GetDlgItem(hDlg, IDC_EDIT_ROWCOUNT));
		SetDlgItemTextA(hDlg, IDC_ROWCOUNT, "Row count");
		SetDlgItemTextA(hDlg, IDC_COLUMNCOUNT, "Column count");
		EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EDIT_ROWCOUNT:
		case IDC_EDIT_COLUMNCOUNT:
			GetDlgItemTextA(hDlg, IDC_EDIT_ROWCOUNT, sRowCount, 16);
			GetDlgItemTextA(hDlg, IDC_EDIT_COLUMNCOUNT, sColumnCount, 16);
			if(strlen(sRowCount) && strlen(sColumnCount))
				EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
			else
				EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
			break;
		case IDOK:
			GetDlgItemTextA(hDlg, IDC_EDIT_ROWCOUNT, sRowCount, 16);
			GetDlgItemTextA(hDlg, IDC_EDIT_COLUMNCOUNT, sColumnCount, 16);
			rowCount = atoi(sRowCount);
			columnCount = atoi(sColumnCount);
			ret = rowCount * 0x10000 + columnCount;
		case IDCANCEL:
			EndDialog(hDlg, ret);
			break;
		}
		break;
	}
	return 0;
}

void UpdateScrollRange(HWND hWnd)
{
	RECT rect;
	GetWindowRect(hWnd, &rect);
	int max = grid->RowCount() * CellSize + rect.bottom - rect.top;
	SetScrollRange(hWnd, SB_VERT, 0, max, TRUE);

	max = grid->ColumnCount() * CellSize + rect.right - rect.left;
	SetScrollRange(hWnd, SB_HORZ, 0, max, TRUE);
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
