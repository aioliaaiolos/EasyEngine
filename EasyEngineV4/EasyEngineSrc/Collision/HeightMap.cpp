#include "HeightMap.h"
#include "ILoader.h"
#include "IFileSystem.h"
#include "IGeometry.h"
#include "Interface.h"

#include <algorithm>

extern IGeometryManager* m_pGeometryManager;

CHeightMap::CHeightMap():
m_nPrecision(3)
{
}

CHeightMap::CHeightMap(EEInterface& oInterface, string sFileName,  const IBox& bbox) :
	m_nPrecision(3),
	m_pLoaderManager(static_cast<ILoaderManager*>(oInterface.GetPlugin("LoaderManager"))),
	m_pGeometryManager(static_cast<IGeometryManager*>(oInterface.GetPlugin("GeometryManager")))
{
	Load(sFileName);
	m_pModelBox = m_pGeometryManager->CreateBox();
	*m_pModelBox = bbox;
	bbox.GetDimension();
}

CHeightMap::~CHeightMap()
{

}

void CHeightMap::SetPrecision( int nPrecision )
{
	m_nPrecision = nPrecision;
}

IBox* CHeightMap::GetModelBBox()
{
	return m_pModelBox;
}

void CHeightMap::AdaptGroundMapToModel(const CMatrix& modelTM, const CVector modelDim, float groundAdaptationHeight)
{
	float xMargin = 2 * modelDim.m_y;
	float zMargin = 2 * modelDim.m_y;
	
	float x0, z0, x1, z1;
	MapToModel(0, 0, x0, z0);
	MapToModel(1, 1, x1, z1);
	float modelUnit = abs(x1 - x0);
	float bias = modelUnit / 2.f;
	modelUnit -= bias;
	CVector modelPos = modelTM.GetPosition();
	float xModel = -modelDim.m_x / 2.f - xMargin - modelUnit;
	float zModel = 0.f;
	float hMin = 99999999.f;
	while (xModel < modelDim.m_x / 2.f + xMargin + modelUnit) {
		zModel = -modelDim.m_z / 2.f - zMargin - modelUnit;
		while (zModel < (modelDim.m_z / 2.f + zMargin + modelUnit) ) {
			CVector P((float)xModel, 0, (float)zModel);
			CVector PTransform = modelTM * P;
			float xMap = 0, yMap = 0;
			ModelToMap(PTransform.m_x, PTransform.m_z, xMap, yMap);
			if (xMap >= m_nWidth || xMap < 0 || yMap >= m_nHeight || yMap < 0) {
				zModel += modelUnit;
				continue;
			}			
			CVector pixel;
			GetPixel(xMap, yMap, pixel);
			float height = GetHeight(pixel);
			if (hMin > height)
				hMin = height;
			zModel += modelUnit;
		}
		xModel += modelUnit;
	}

	hMin += groundAdaptationHeight;
	if (hMin < 0.f) hMin = 0.f;
	if (hMin > 255.f) hMin = 255.f;

	xModel = -modelDim.m_x / 2.f - xMargin - modelUnit;
	while (xModel < (modelDim.m_x / 2.f + xMargin + modelUnit)) {
		zModel = -modelDim.m_z / 2.f - zMargin - modelUnit;
		while (zModel < (modelDim.m_z / 2.f + zMargin + modelUnit)) {
			CVector P((float)xModel, 0, (float)zModel);
			CVector PTransform = modelTM * P;
			float xMap = 0, yMap = 0;
			ModelToMap(PTransform.m_x, PTransform.m_z, xMap, yMap);
			if (xMap >= m_nWidth || xMap < 0 || yMap >= m_nHeight || yMap < 0) {
				zModel += modelUnit;
				continue;
			}
			SetPixelValue(xMap, yMap, hMin);
			zModel += modelUnit;
		}
		xModel += modelUnit;
	}
}

void CHeightMap::RestoreHeightMap(const CMatrix& modelTM, const CVector& modelDim, string originalHeightMap)
{
	ILoader::CTextureInfos ti;
	ti.m_bFlip = false;
	m_pLoaderManager->Load(originalHeightMap, ti);
	ti.m_vTexels;
	
	float xMargin = modelDim.m_y;
	float zMargin = modelDim.m_y;
	float x0, z0, x1, z1;
	MapToModel(0, 0, x0, z0);
	MapToModel(1, 1, x1, z1);
	float modelUnit = abs(x1 - x0);
	float bias = modelUnit / 10.f;
	modelUnit -= bias;
	CVector modelPos = modelTM.GetPosition();
	
	int xModel = -modelDim.m_x / 2.f - xMargin;
	int zModel = 0;
	float hMin = 99999999.f;
	while (xModel < modelDim.m_x / 2.f + xMargin) {
		zModel = -modelDim.m_z / 2.f - zMargin;
		while (zModel < (modelDim.m_z / 2.f + zMargin)) {
			CVector P((float)xModel, 0, (float)zModel);
			CVector PTransform = modelTM * P;
			float xMap = 0, yMap = 0;
			ModelToMap(PTransform.m_x, PTransform.m_z, xMap, yMap);
			CVector pixel;
			if (xMap >= m_nWidth || xMap < 0 || yMap >= m_nHeight || yMap < 0) {
				zModel += modelUnit;
				continue;
			}
			GetPixel(ti, xMap, yMap, pixel);
			float height = GetHeight(pixel);
			SetPixelValue(xMap, yMap, height);
			zModel += modelUnit;
		}
		xModel += modelUnit;
	}
}

void CHeightMap::GetFileName(string& fileName)
{
	fileName = m_sFileName;
}

void CHeightMap::SetSliceCount(int sliceCount)
{
	m_nSliceCount = sliceCount;
}

void CHeightMap::Load(string sFileName)
{
	ILoader::CTextureInfos ti;
	ti.m_bFlip = false;
	m_pLoaderManager->Load( sFileName, ti );

	double l = log2(ti.m_nWidth);
	int il = (int)l;
	if (l - il != 0) {
		CEException e("Erreur : La texture utilisee pour la height map doit avoir une taille en puissance de 2");
		throw e;
	}

	m_nWidth = ti.m_nWidth;
	m_nHeight = ti.m_nHeight;
	m_vPixels.swap( ti.m_vTexels );
	m_sFileName = sFileName;
}

void CHeightMap::Save(string sFileName)
{
	ILoader::CTextureInfos ti;
	ti.m_bFlip = false;
	ti.m_nWidth = m_nWidth;
	ti.m_nHeight = m_nHeight;
	ti.m_vTexels = m_vPixels;
	ti.m_ePixelFormat = ILoader::eRGB;
	m_pLoaderManager->Export(sFileName, ti);
}

void CHeightMap::GetPixel( int x, int y, CVector& pixel )
{
	int nPixelIndice = 3 * x + y * m_nWidth * 3;
	pixel.m_x = m_vPixels[ nPixelIndice ];
	pixel.m_y = m_vPixels[ nPixelIndice + 1 ];
	pixel.m_z = m_vPixels[ nPixelIndice + 2 ];
}

void CHeightMap::GetPixel(const ILoader::CTextureInfos& ti, int x, int y, CVector& pixel)
{
	int nPixelIndice = 3 * x + y * ti.m_nWidth * 3;
	pixel.m_x = ti.m_vTexels[nPixelIndice];
	pixel.m_y = ti.m_vTexels[nPixelIndice + 1];
	pixel.m_z = ti.m_vTexels[nPixelIndice + 2];
}

void CHeightMap::SetPixelValue(int x, int y, float value)
{
	int nPixelIndice = 3 * x + y * m_nWidth * 3;
	m_vPixels[nPixelIndice] = value;
	m_vPixels[nPixelIndice + 1] = value;
	m_vPixels[nPixelIndice + 2] = value;
}

float CHeightMap::GetHeight( const CVector& p )
{
	float ret = 0.f;
	if( m_nPrecision == 1 )
		ret = (float)p.m_x;
	else if( m_nPrecision == 3 )
	{
		int nColor = p.m_z * pow( 2., 16 ) + p.m_y * pow(2., 8.) + p.m_x;
		ret = (float)nColor / pow( 2., 16. );
	}
	return ret;
}

// Helper : reproduit exactement texture2D(heightMap, uv) avec GL_LINEAR
float CHeightMap::SampleHeightmapBilinear(float xModel, float zModel)
{
	float xMap, yMap;
	ModelToMap(xModel, zModel, xMap, yMap);

	int ix = (int)floor(xMap);
	int iy = (int)floor(yMap);
	float fx = xMap - ix;
	float fy = yMap - iy;

	auto clampX = [&](int i) { return (i < 0) ? 0 : (i > m_nWidth - 1 ? m_nWidth - 1 : i); };
	auto clampY = [&](int i) { return (i < 0) ? 0 : (i > m_nHeight - 1 ? m_nHeight - 1 : i); };
	int x0 = clampX(ix), x1 = clampX(ix + 1);
	int y0 = clampY(iy), y1 = clampY(iy + 1);

	CVector p00, p10, p01, p11;
	GetPixel(x0, y0, p00);
	GetPixel(x1, y0, p10);
	GetPixel(x0, y1, p01);
	GetPixel(x1, y1, p11);

	float h00 = GetHeight(p00);
	float h10 = GetHeight(p10);
	float h01 = GetHeight(p01);
	float h11 = GetHeight(p11);

	float h = (1 - fx) * (1 - fy) * h00
		+ fx       * (1 - fy) * h10
		+ (1 - fx) * fy       * h01
		+ fx       * fy       * h11;

	return GetHeightFromPixelValue(h);
}

void CHeightMap::MapToModel(float xMap, float yMap, float& xModel, float& zModel)
{
	float dimx = m_pModelBox->GetDimension().m_x;
	float dimz = m_pModelBox->GetDimension().m_z;

	// Inverse de ModelToMap
	float u = (xMap + 0.5f) / m_nWidth;
	float v = (yMap + 0.5f) / m_nHeight;

	xModel = u * dimx - dimx / 2.f;
	zModel = v * dimz - dimz / 2.f;
}

void CHeightMap::ModelToMap(float xModel, float zModel, float& xMap, float& yMap)
{
	float dimx = m_pModelBox->GetDimension().m_x;
	float dimz = m_pModelBox->GetDimension().m_z;

	// UV dans [0, 1]
	float u = (xModel + dimx / 2.f) / dimx;
	float v = (zModel + dimz / 2.f) / dimz;

	// Pixel-center : px = u * W - 0.5 (cohérent avec OpenGL GL_LINEAR)
	xMap = u * m_nWidth - 0.5f;
	yMap = v * m_nHeight - 0.5f;
}

// Niveau mesh : reproduit la rasterisation des triangles
float CHeightMap::GetHeight(float xModel, float zModel)
{
	float dimx = m_pModelBox->GetDimension().m_x;
	float dimz = m_pModelBox->GetDimension().m_z;

	float quadSizeX = dimx / m_nSliceCount;
	float quadSizeZ = dimz / m_nSliceCount;

	// Position dans la grille MESH
	float fColumn = (xModel + dimx / 2.f) / quadSizeX;
	float fLine = (zModel + dimz / 2.f) / quadSizeZ;

	if (fColumn < 0 || fColumn >= m_nSliceCount || fLine < 0 || fLine >= m_nSliceCount)
		return -1e8f;

	int column = (int)fColumn;
	int line = (int)fLine;
	float dx = fColumn - column;
	float dy = fLine - line;

	// Positions monde des 4 vertices du quad (correspond exactement à CreatePlane2)
	float x0 = column       * quadSizeX - dimx / 2.f;
	float x1 = (column + 1) * quadSizeX - dimx / 2.f;
	float z0 = line       * quadSizeZ - dimz / 2.f;
	float z1 = (line + 1) * quadSizeZ - dimz / 2.f;

	// Hauteur de chaque vertex = ce que le GPU calcule via texture2D bilinéaire
	float h00 = SampleHeightmapBilinear(x0, z0);
	float h10 = SampleHeightmapBilinear(x1, z0);
	float h01 = SampleHeightmapBilinear(x0, z1);
	float h11 = SampleHeightmapBilinear(x1, z1);

	// Sélection du triangle (diagonale "/" — partagée entre v(column+1,line) et v(column,line+1))
	if (dx + dy <= 1.0f)
		return h00 + dx * (h10 - h00) + dy * (h01 - h00);
	else
		return h11 + (1.f - dx) * (h01 - h11) + (1.f - dy) * (h10 - h11);
}

float CHeightMap::GetHeightFromPixelValue(float pixelValue)
{
	return pixelValue * m_pModelBox->GetDimension().m_y / 255.f + m_pModelBox->GetMinPoint().m_y;
}

float CHeightMap::GetPixelValueFromHeight(float height)
{
	return 255.f * (height - m_pModelBox->GetMinPoint().m_y ) / m_pModelBox->GetDimension().m_y;
}

int	CHeightMap::GetPixelNumberFromCoord( int nWidth, int nRow, int nColumn, int bpp )
{
	return bpp * ( nRow * nWidth + nColumn );
}