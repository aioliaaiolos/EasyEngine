// System
#include <stdio.h>

// OpenGL

// Engine
#include "textureloader.h"
#include "../Utils2/Dimension.h"
#include "Exception.h"
#include "IFileSystem.h"
#include "../Utils2/Chunk.h"

using namespace std;

//------------------------------------------------------------------------------
//									CBMPLoader
//------------------------------------------------------------------------------
CBMPLoader::CBMPLoader(void)
{
}

CBMPLoader::~CBMPLoader(void)
{
}

void CBMPLoader::ReadBMP(string sFileName, vector< unsigned char >& vData, unsigned int& nWidth, unsigned int& nHeight, int& nBitPerPixel)
{
	BITMAPFILEHEADER header;
	BITMAPINFOHEADER infoHeader;

	ZeroMemory(&header, sizeof(BITMAPFILEHEADER));
	ZeroMemory(&infoHeader, sizeof(BITMAPINFOHEADER));
	FILE* pFile = fopen(sFileName.c_str(), "rb");
	
	fread(&header, sizeof(BITMAPFILEHEADER), 1, pFile);
	fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, pFile);	
	
	rewind(pFile);
	fseek(pFile, header.bfOffBits, SEEK_SET);
	int sizeImage = infoHeader.biSizeImage ? infoHeader.biSizeImage : infoHeader.biWidth * infoHeader.biHeight * infoHeader.biBitCount / 8;
	vData.resize(sizeImage);
	fread(&vData[0], sizeImage, 1, pFile);
	nWidth = infoHeader.biWidth;
	nHeight = infoHeader.biHeight;
	nBitPerPixel = infoHeader.biBitCount;
	fclose(pFile);
}

void CBMPLoader::WriteBMP(string sFileName, const vector< unsigned char >& vData, unsigned int nWidth, int unsigned nHeight, int nBitPerPixel)
{
	BITMAPFILEHEADER header;
	BITMAPINFOHEADER infoHeader;

	ZeroMemory(&header, sizeof(BITMAPFILEHEADER));
	ZeroMemory(&infoHeader, sizeof(BITMAPINFOHEADER));

	header.bfType = 0x4d42;
	infoHeader.biBitCount = nBitPerPixel;
	infoHeader.biHeight = nHeight;
	infoHeader.biWidth = nWidth;
	unsigned int nPixelSize = nBitPerPixel / 8;
	infoHeader.biSizeImage = nWidth * nHeight * nPixelSize;
	header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	header.bfSize = header.bfOffBits + infoHeader.biSizeImage;
	infoHeader.biSize = sizeof(BITMAPINFOHEADER);
	infoHeader.biPlanes = 1;

	FILE* pFile = fopen(sFileName.c_str(), "wb");
	if (!pFile) {
		CFileNotFoundException e(sFileName);
		throw e;
	}

	fwrite(&header, sizeof(BITMAPFILEHEADER), 1, pFile);
	fwrite(&infoHeader, sizeof(BITMAPINFOHEADER), 1, pFile);
	fwrite(&vData[0], infoHeader.biSizeImage, 1, pFile);
	fclose(pFile);
}

void CBMPLoader::Load( string sFileName, IRessourceInfos& ri, IFileSystem& oFileSystem )
{
	CTextureInfos* pTI = static_cast< CTextureInfos* >( &ri );
	string sRootDirectory;
	FILE* pFile = oFileSystem.OpenFile( sFileName, "rb" );
	if ( pFile )
	{
		oFileSystem.GetLastDirectory( sRootDirectory );
		fclose( pFile );
		
		string sPath;
		if ( sRootDirectory.size() > 0 )
			sPath = sRootDirectory + "\\" + sFileName;
		else
			sPath = sFileName;
		vector< unsigned char > vData;
		int nBitPerPixel = 0;
		ReadBMP(sPath, vData, pTI->m_nWidth, pTI->m_nHeight, nBitPerPixel);
		size_t size = pTI->m_nWidth * pTI->m_nHeight * nBitPerPixel / 8;
		pTI->m_vTexels.resize( size );
		memcpy(&pTI->m_vTexels[0], &vData[0], size);
		switch(nBitPerPixel)
		{
		case 24:
			pTI->m_ePixelFormat = eBGR;
			break;
		case 32:
			pTI->m_ePixelFormat = eBGRA;
			break;
		default:
			pTI->m_ePixelFormat = ePixelFormatNone;
			break;
		}
		
		if( pTI->m_bFlip )
			FlipImage( *pTI );
	}
	else
	{
		string sMessage = string( "Erreur : fichier \"" ) + sFileName + "\" manquant";
		CFileNotFoundException e( sMessage );
		e.m_sFileName = sFileName;
		throw e;
	}
}

void CBMPLoader::FlipImage( CTextureInfos& ti )
{
	vector< unsigned char > vPixels;
	int nBytePerPixel = 3;
	if( ti.m_ePixelFormat == ILoader::eRGBA || ti.m_ePixelFormat == ILoader::eBGRA )
		nBytePerPixel = 4;
	vPixels.resize( ti.m_nWidth * ti.m_nHeight * nBytePerPixel );

	for( int i = 0; i < ti.m_nHeight; i++ )
		for( int j = 0; j < ti.m_nWidth; j++ )
			for( int k = 0; k < nBytePerPixel; k++ )
			{
				int iOld = ( ti.m_nHeight - i - 1 ) * ti.m_nWidth * nBytePerPixel + nBytePerPixel * j + k;
				int iNew = i * ti.m_nWidth * nBytePerPixel + nBytePerPixel * j + k;
				vPixels[ iNew ] = ti.m_vTexels[ iOld ];
			}
	ti.m_vTexels.swap( vPixels );
}


void CBMPLoader::Export( string sFileName, ILoader::IRessourceInfos& ri )
{
	ILoader::CTextureInfos* pInfos = static_cast< ILoader::CTextureInfos* >( &ri );
	if (pInfos->m_bFlip)
		FlipImage(*pInfos);

	unsigned int nWidth = pInfos->m_nWidth;
	unsigned int nHeight = pInfos->m_nHeight;
	const vector< unsigned char >& vData = pInfos->m_vTexels;
	int nBitPerPixel = 0;
	switch( pInfos->m_ePixelFormat )
	{
	case eRGB:
	case eBGR:
		nBitPerPixel = 24;
		break;
	case eRGBA:
	case eBGRA:
		nBitPerPixel = 32;
	}


	WriteBMP(sFileName, vData, nWidth, nHeight, nBitPerPixel);

}

//------------------------------------------------------------------------------
//									CTGALoader
//------------------------------------------------------------------------------
CTGALoader::CTGALoader(void)
{
}

CTGALoader::~CTGALoader(void)
{
}

bool CTGALoader::Load( string sFileName, CChunk& chunk, IFileSystem& oFileSystem )
{
	//fopen_s( &pFile, sFileName.c_str(), "rb" );
	FILE* pFile = oFileSystem.OpenFile( sFileName, "rb" );
	if ( !pFile )
		return false;
		
	
	TGAHeader header;
	fread(&header, sizeof(TGAHeader), 1, pFile);	

	TGA tga;
	tga._nWidth = header._width;
	tga._nHeight = header._height;
	tga._nFormat = eRGBA ;
	tga._nInternalFormat = 4;

	CDimension* pDim = new CDimension((float)tga._nWidth , (float)tga._nHeight);

	unsigned int nPixelCount = tga._nWidth*tga._nHeight;	
	unsigned char * pTexels = new unsigned char[nPixelCount*tga._nInternalFormat];
	for (unsigned int i=0 ; i<nPixelCount ; i++)
	{
		unsigned char c = (unsigned char) fgetc( pFile );
		pTexels[4*i+2] = c;
		c = (unsigned char) fgetc( pFile );
		pTexels[4*i+1] = c;
		c = (unsigned char) fgetc( pFile );
		pTexels[4*i] = c;
		c = (unsigned char) fgetc( pFile );
		pTexels[4*i+3] = c;
	}
	fclose( pFile );
	TPixelFormat* pPixelFormat = new TPixelFormat;
	*pPixelFormat = (TPixelFormat)tga._nFormat;

	chunk.Add(pTexels, "Texels");
	chunk.Add(pDim, "Dimensions");
	chunk.Add(pPixelFormat, "Pixels format");
	return true;
}

void CTGALoader::Load( string sFileName, IRessourceInfos& ri, IFileSystem& oFileSystem )
{
	CTextureInfos* pTI = static_cast< CTextureInfos* >( &ri );
	FILE* pFile = oFileSystem.OpenFile( sFileName, "rb" );
	if ( !pFile )
	{
		string sMessage = string( "Erreur dans CTGALoader::Load() : \"" ) + sFileName + "\" manquant.";
		CFileNotFoundException e( sMessage );
		e.m_sFileName = sFileName;
		throw e;
	}
	
	TGAHeader header;
	fread(&header, sizeof(TGAHeader), 1, pFile);	

	TGA tga;
	tga._nWidth = header._width;
	tga._nHeight = header._height;
	tga._nFormat = eRGBA ;
	tga._nInternalFormat = 4;

	pTI->m_nWidth = tga._nWidth;
	pTI->m_nHeight = tga._nHeight;
	
	unsigned int nPixelCount = tga._nWidth*tga._nHeight;
	pTI->m_vTexels.resize( nPixelCount * tga._nInternalFormat );
	for (unsigned int i=0 ; i<nPixelCount ; i++)
	{
		unsigned char c = (unsigned char) fgetc( pFile );
		pTI->m_vTexels[ 4 * i + 2 ] = c;
		c = (unsigned char) fgetc( pFile );
		pTI->m_vTexels[ 4 * i + 1 ] = c;
		c = (unsigned char) fgetc( pFile );
		pTI->m_vTexels[ 4 * i ] = c;
		c = (unsigned char) fgetc( pFile );
		pTI->m_vTexels[ 4 * i + 3 ] = c;
	}
	fclose( pFile );
	pTI->m_ePixelFormat = (TPixelFormat)tga._nFormat;
}


void CTGALoader::Load( string sFileName, CTextureInfos& ti, IFileSystem& oFileSystem )
{
	FILE* pFile = oFileSystem.OpenFile( sFileName, "rb" );
	if ( !pFile )
	{
		string sMessage = string( "Erreur dans CTGALoader::Load() : \"" ) + sFileName + "\" manquant.";
		CFileNotFoundException e( sMessage );
		e.m_sFileName = sFileName;
		throw e;
	}	
	
	TGAHeader header;
	fread(&header, sizeof(TGAHeader), 1, pFile);	

	TGA tga;
	tga._nWidth = header._width;
	tga._nHeight = header._height;
	tga._nFormat = eRGBA ;
	tga._nInternalFormat = 4;

	ti.m_nWidth = tga._nWidth;
	ti.m_nHeight = tga._nHeight;
	
	unsigned int nPixelCount = tga._nWidth*tga._nHeight;
	ti.m_vTexels.resize( nPixelCount * tga._nInternalFormat );
	for (unsigned int i=0 ; i<nPixelCount ; i++)
	{
		unsigned char c = (unsigned char) fgetc( pFile );
		ti.m_vTexels[ 4 * i + 2 ] = c;
		c = (unsigned char) fgetc( pFile );
		ti.m_vTexels[ 4 * i + 1 ] = c;
		c = (unsigned char) fgetc( pFile );
		ti.m_vTexels[ 4 * i ] = c;
		c = (unsigned char) fgetc( pFile );
		ti.m_vTexels[ 4 * i + 3 ] = c;
	}
	fclose( pFile );
	ti.m_ePixelFormat = (TPixelFormat)tga._nFormat;
}
