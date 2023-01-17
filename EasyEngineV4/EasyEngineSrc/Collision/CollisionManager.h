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
	bool			IsSegmentInsideSegment(float fS1Center, float fS1Radius, float fS2Center, float fS2Radius);

	// Collision map
	ICollisionMap*	CreateCollisionMap(IEntity* pScene, float fBias) override;

	// temp	
	void	SetEntityManager(IEntityManager* pEntityManager);


private:

	void			ComputeGroundMapDimensions(IMesh* pMesh, int nScreenWidth, int nScreenHeight, float& width, float& height, float& groundToScreenScaleFactor);

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
	float					m_fScreenRatio;
	float					m_fMaxLenght;

	// Height map
	bool						m_bEnableHMHack;
	bool						m_bEnableHMHack2;
	map<string, int>			m_mMapFileToId;

	// temp
	IEntityManager*				m_pEntityManager;
	// fin temp


	static IMesh*			s_pMesh;

	static void		OnRenderHeightMap(IRenderer* pRenderer);


	// Height map
	void EnableHMHack(bool enable);
	void EnableHMHack2(bool enable);
};

extern "C" _declspec(dllexport) CCollisionManager* CreateCollisionManager(EEInterface& oInterface);