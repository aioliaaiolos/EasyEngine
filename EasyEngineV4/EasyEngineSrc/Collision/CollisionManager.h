// stl
#include <string>
#include <vector>

#include "ICollisionManager.h"

using namespace std;

class IMesh;
class IRenderer;
class ILoaderManager;
class CHeightMap;
class IFileSystem;
class CBox;
class ISphere;
class ISegment2D;
class IEntity;
class INode;

class CCollisionManager : public ICollisionManager
{
public:	

	CCollisionManager(EEInterface& oInterface);
	void			CreateHeightMap( IMesh* pGround, ILoader::CTextureInfos& ti , IRenderer::TPixelFormat format = IRenderer::T_RGB );
	void			CreateHeightMap(string sFileName);
	void			CreateHeightMapWithoutRender(string sFileName);
	int				LoadHeightMap( string sFileName, IBox* pBox, bool forceReload = false) override;
	void			LoadHeightMap( string sFileName, vector< vector< unsigned char > >& vPixels  );
	float			GetMapHeight( int nHeightMapID, float x, float z );
	void 			DisplayHeightMap( IMesh* pMesh );
	void 			StopDisplayHeightMap();
	void			SetHeightMapPrecision( int nPrecision );
	bool			IsIntersection( const IBox& b, const ISphere& s );
	bool			IsIntersection( const ISegment& s, const IBox& b2 );
	bool			IsIntersection( const ISegment& s, const CVector& oCircleCenter, float fCircleRadius );
	void			Get2DLineIntersection( const ISegment2D& pSeg1, const ISegment2D& pSeg2, CVector2D& oIntersection );
	void			Get2DLineIntersection( const CVector2D& oLine1First, const CVector2D& oLine1Last, const CVector2D& oLine2First, const CVector2D& oLine2Last, CVector2D& oIntersection );
	bool			IsSegmentRectIntersect( const ISegment2D& s, float fRectw, float fRecth, const CMatrix2X2& oRectTM );
	bool			IsSegmentRectIntersect( const CVector2D& S1, const CVector2D& S2, float fRectw, float fRecth, const CMatrix2X2& oRectTM );
	void			SetGroundBoxHeight(int nMapId, float height);
	void			SetGroundBoxMinPoint(int nMapId, float min);
	IBox*			GetGroundBox(int nMapId);
	string			GetName() override;
	IHeightMap*		GetHeightMap(int index) override;
	void			ClearHeightMaps() override;

	// Collision map
	void	CreateCollisionMap(string sFileName, IEntity* pRoot, int cellSize, float fBias) override;
	void	CreateCollisionMapByRendering(ILoader::CTextureInfos& ti, vector<IEntity*> collides, IEntity* pScene, IRenderer::TPixelFormat format = IRenderer::T_RGB);
	void	CreateTextureFromCollisionArray(string sFileName, const vector<vector<bool>>& vGrid) override;
	void	AttachCollisionMapToScene(const ILoader::CTextureInfos& m_oCollisionMap, IEntity* pScene);
	void	LoadCollisionMapComputedByRendering(string sFileName, IEntity* pScene, ILoader::CTextureInfos& collisionMap);
	void	SendCustomUniformValue(string name, float value);
	void	DisplayCollisionMap();
	void	StopDisplayCollisionMap();
	void	DisplayGrid(int nCellSize);

	// temp	
	void	SetEntityManager(IEntityManager* pEntityManager);


private:
	EEInterface&			m_oInterface;
	IRenderer&				m_oRenderer;
	ILoaderManager&			m_oLoaderManager;
	IEntityManager&			m_oEntityManager;
	IFileSystem*			m_pFileSystem;
	map< int, CHeightMap* >	m_mHeigtMap;
	CVector					m_oOriginBackgroundColor;
	int						m_nHeightMapPrecision;
	IGeometryManager&		m_oGeometryManager;
	IEntity*				m_pScene;
	IMesh*					m_pGround;
	vector<IEntity*>		m_vCollideObjects;
	string					m_sCustomName;
	float					m_fCustomValue;
	IEntity*				m_pSphere;
	float					m_fGroundWidth;
	float					m_fGroundHeight;
	float					m_fScreenRatio;
	float					m_fMaxLenght;

	// Collision map
	float						m_fGroundMapWidth;
	float						m_fGroundMapHeight;
	float						m_fWorldToScreenScaleFactor;
	float						m_fGridHeight;
	char**						m_pCollisionGrid;
	vector<IEntity*>			m_vGridElements;

	// Height map
	bool						m_bEnableHMHack;
	bool						m_bEnableHMHack2;
	map<string, int>			m_mMapFileToId;

	// temp
	IEntityManager*				m_pEntityManager;
	// fin temp


	static IMesh*			s_pMesh;

	static void		OnRenderHeightMap(IRenderer* pRenderer);

	static void OnRenderCollisionMapCallback(IRenderer*);
	void OnRenderCollisionMap();
	void GetOriginalShaders(const vector<IEntity*>& staticObjects, vector<IShader*>& vBackupStaticObjectShader);
	void SetCollisionShaders(const vector<IEntity*>& staticObjects, IShader* pCollisionShader);
	void RestoreOriginalShaders(const vector<IShader*>& vBackupStaticObjectShader, vector<IEntity*>& staticObjects);
	void RenderCollisionGeometry(IShader* pCollisionShader, const CMatrix& groundModel, const IBox* const pBox);
	bool IsSegmentInsideSegment(float fS1Center, float fS1Radius, float fS2Center, float fS2Radius);
	void ComputeGroundMapDimensions(IMesh* pMesh, int nScreenWidth, int nScreenHeight, float& width, float& height, float& groundToScreenScaleFactor);

	// Collision map
	void MarkBox(int row, int column, float r, float g, float b);
	void GetCellCoordFromPosition(float x, float y, int& cellx, int& celly, int nCellSize);
	void GetPositionFromCellCoord(int row, int column, float& x, float& y);
	void ComputeRowAndColumnCount(int& rowCount, int& columnCount, int nCellSize);
	bool TestCellObstacle(const ILoader::CTextureInfos& collisionMap, int x, int y);
	float WorldToPixel(float worldLenght, int nMapWidth, int nMapHeight);
	void ConvertLinearToCoord(int pixelNumber, int nTextureWidth, int& x, int& y);
	void CreateCollisionArray(IEntity* pRoot, vector<vector<bool>>& vGrid, int nCelSize, float fBias);
	void AddObjectToCollisionGrid(const CVector& rootDim, const CDimension& gridDimension, const CVector& objectDim, const CMatrix& modelTM, vector<vector<bool>>& vGrid, int nCelSize, float fBias);
	void GetCollisionEntities(IEntity* pRoot, vector<IEntity*>& vCollisionEntities);
	void SortObjectsByHeight(IEntity* pRoot, vector<pair<float, IEntity*>>& vSortedObjectHeight);
	float GetFloors(const vector<pair<float, IEntity*>>& vSortedObjectHeight, vector<pair<float, IEntity*>>& floors, vector<pair<float, IEntity*>>& nonFloors);
	void GetRoofs(float fFloorHeight, const vector<pair<float, IEntity*>>& nonFloors, vector<IEntity*>& roofs, vector<IEntity*>& nonRoofs);
	static int ComputeOptimalCellSize(int nCellSize, const CDimension& modelDimension);

	// Height map
	void EnableHMHack(bool enable);
	void EnableHMHack2(bool enable);
};

extern "C" _declspec(dllexport) CCollisionManager* CreateCollisionManager(EEInterface& oInterface);