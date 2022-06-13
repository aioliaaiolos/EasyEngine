#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

// stl
#include <string>
#include <vector>
#include "ICollisionManager.h"


class ILoaderManager;
class IFileSystem;
class IBox;
class CVector;
class IGeometryManager;

using namespace std;

class CHeightMap : public IHeightMap
{
public:
	CHeightMap();
	CHeightMap(EEInterface& oInterface, string sFileName, const IBox& bbox);
	~CHeightMap();
	void	Load(string sFileName);
	void	Save(string sFileName) override;
	void	GetPixel( int x, int y, CVector& pixel );
	float	GetHeight( float xModel, float zModel ) override;
	void	ModelToMap(int xModel, int zModel, int& xMap, int& yMap) override;
	void	SetPrecision( int nPrecision );
	IBox*	GetModelBBox();
	void	AdaptGroundMapToModel(const CMatrix& modelTM, const CVector modelDim, float groundAdaptationHeight) override;
	void	RestoreHeightMap(const CMatrix& modelTM, const CVector& modelDim, string originalHeightMap) override;
	void	GetFileName(string& fileName) override;

private:
	float	GetHeight(const CVector& p);
	void	ModelToMap(int xModel, int zModel, float& xMap, float& yMap);
	void	MapToModel(int xMap, int yMap, float& xModel, float& zModel);
	void	SetPixelValue(int x, int y, float value);
	float	GetHeightFromPixelValue(float pixelValue);
	float	GetPixelValueFromHeight(float height);

	static int	GetPixelNumberFromCoord(int nWidth, int nRow, int nColumn, int bpp);
	static void GetPixel(const ILoader::CTextureInfos& ti, int x, int y, CVector& pixel);

	vector< unsigned char >	m_vPixels;
	int		m_nWidth;
	int		m_nHeight;
	IBox*	m_pModelBox; 
	int		m_nPrecision;
	string	m_sFileName;
	ILoaderManager* m_pLoaderManager;
	IGeometryManager* m_pGeometryManager;
};

#endif // HEIGHTMAP_H