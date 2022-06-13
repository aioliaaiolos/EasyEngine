#ifndef ICOLLISION_MANAGER_H
#define ICOLLISION_MANAGER_H

#include "EEPlugin.h"
#include "IRenderer.h"
#include "ILoader.h"
#include "../Utils2/Dimension.h"

// stl
#include <vector>

using namespace std;

class IMesh;
class IRenderer;
class ILoaderManager;
class CBox;
class ISphere;
class IGeometryManager;
class ISegment2D;
class ISegment;
class IEntity;
class IEntityManager;

class IHeightMap
{
public:
	virtual void	AdaptGroundMapToModel(const CMatrix& modelTM, const CVector modelDim, float groundAdaptationHeight) = 0;
	virtual void	Save(string sFileName) = 0;
	virtual void	GetFileName(string& fileName) = 0;
	virtual void	RestoreHeightMap(const CMatrix& modelTM, const CVector& modelDim, string originalHeightMap) = 0;
	virtual float	GetHeight(float xModel, float zModel) = 0;
	virtual void	ModelToMap(int xModel, int zModel, int& xMap, int& yMap)  = 0;
};

class ICollisionManager : public CPlugin
{
public:	
	ICollisionManager() : CPlugin( NULL, "" ){}
	virtual void			CreateHeightMap( IMesh* pMesh, ILoader::CTextureInfos& ti, IRenderer::TPixelFormat format = IRenderer::T_RGB ) = 0;	
	virtual void			CreateHeightMap( string sFileName) = 0;
	virtual void			LoadHeightMap( string sFileName, vector< vector< unsigned char > >& vPixels  ) = 0;
	virtual int				LoadHeightMap( string sFileName, IBox* pBox, bool forceReload = false) = 0;
	virtual float			GetMapHeight( int nHeightMapID, float xModel, float yModel ) = 0;
	virtual void			DisplayHeightMap( IMesh* pMesh ) = 0;
	virtual void			StopDisplayHeightMap() = 0;
	virtual	void			SetHeightMapPrecision( int nPrecision ) = 0;
	virtual bool			IsIntersection( const IBox& b, const ISphere& s ) = 0;
	virtual bool			IsIntersection( const ISegment& s, const IBox& b2 ) = 0;
	virtual bool			IsIntersection( const ISegment& s, const CVector& oCircleCenter, float fCircleRadius ) = 0;
	virtual void			Get2DLineIntersection( const CVector2D& oLine1First, const CVector2D& oLine1Last, const CVector2D& oLine2First, const CVector2D& oLine2Last, CVector2D& oIntersection ) = 0;
	virtual bool			IsSegmentRectIntersect( const ISegment2D& s, float fRectw, float fRecth, const CMatrix2X2& oSquareTM ) = 0;
	virtual bool			IsSegmentRectIntersect( const CVector2D& S1, const CVector2D& S2, float fRectw, float fRecth, const CMatrix2X2& oRectTM ) = 0;
	virtual void			SendCustomUniformValue(string name, float value) = 0;
	virtual IBox*			GetGroundBox(int nMapId) = 0;
	virtual void			SetGroundBoxHeight(int nMapId, float height) = 0;
	virtual void			SetGroundBoxMinPoint(int nMapId, float height) = 0;
	virtual IHeightMap*		GetHeightMap(int index) = 0;
	virtual void			ClearHeightMaps() = 0;
	
	// Collision map
	virtual void	DisplayCollisionMap() = 0;
	virtual void	StopDisplayCollisionMap() = 0;
	virtual void	CreateCollisionMap(string sFileName, IEntity* pRoot, int cellSize, float fBias) = 0;
	virtual void	CreateCollisionMapByRendering(ILoader::CTextureInfos& ti, vector<IEntity*> collides, IEntity* pScene, IRenderer::TPixelFormat format = IRenderer::T_RGB) = 0;
	virtual void	CreateCollisionArray(IEntity* pRoot, vector<vector<bool>>& vGrid, int nCellSize, float fBias) = 0;
	virtual void	CreateTextureFromCollisionArray(string sFileName, const vector<vector<bool>>& vGrid) = 0;
	virtual void	DisplayGrid(int nCellSize) = 0;
	virtual void	ComputeRowAndColumnCount(int& rowCount, int& columnCount, int nCellSize) = 0;
	virtual void	GetCellCoordFromPosition(float x, float y, int& cellx, int& celly, int nCellSize) = 0;
	virtual void	GetPositionFromCellCoord(int row, int column, float& x, float& y) = 0;
	virtual bool	TestCellObstacle(const ILoader::CTextureInfos& textureInfos, int x, int y) = 0;
	virtual void	AttachCollisionMapToScene(const ILoader::CTextureInfos& m_oCollisionMap, IEntity* pScene) = 0;
	virtual void	LoadCollisionMapComputedByRendering(string sFileName, IEntity* pScene, ILoader::CTextureInfos& collisionMap) = 0;

	virtual void	EnableHMHack(bool enable) = 0;
	virtual void	EnableHMHack2(bool enable) = 0;

	virtual void	SetEntityManager(IEntityManager* pEntityManager) = 0;
};

#endif // ICOLLISION_MANAGER_H