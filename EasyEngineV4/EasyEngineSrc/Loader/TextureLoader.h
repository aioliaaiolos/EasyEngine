#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include "loader.h"
#include "../Utils2/Singleton.h"


#pragma pack(push, 1)

struct TGAHeader
{
	unsigned char		_length;
	unsigned char		_colormap_type;
	unsigned char		_image_type;

	short				_cm_firstEntry;
	short				_cm_length;
	unsigned char		_cm_size;

	short				_x_origin;
	short				_y_origin;

	short				_width;
	short				_height;

	unsigned char		_PixelDepth;
	unsigned char		_ImageDescriptor;
};

#pragma pack(pop)

struct TGA
{
	unsigned int	_nWidth;
	unsigned int	_nHeight;

	unsigned int	_nFormat;
	int				_nInternalFormat;

	unsigned char*	_pTexels;

};


class ITextureLoader : public ILoader
{
public:
};


class CBMPLoader :	public CSingleton<CBMPLoader> , public ITextureLoader
{
	friend class CSingleton<CBMPLoader>;
	void			FlipImage(CTextureInfos& ti);

public:
					CBMPLoader(void);								
	virtual			~CBMPLoader(void);
	bool			Load( const std::string& sFileName, CChunk& chunk, IFileSystem& oFileSystem );
	void			Load( string sFileName, IRessourceInfos& ti, IFileSystem& oFileSystem );
	void			Load( std::string sFileName, IRessourceInfos& mi ){}
	void			Export( string sFileName, ILoader::IRessourceInfos& ri );
	void			ReadBMP(string sFileName, vector< unsigned char >& vData, unsigned int& nWidth, unsigned int& nHeight, int& nBitPerPixel);
	void			WriteBMP(string sFileName, const vector< unsigned char >& vData, unsigned int nWidth, unsigned int nHeight, int nBitPerPixel);
};



class CTGALoader : public ITextureLoader 
{
								
public:
					CTGALoader(void);
	virtual			~CTGALoader(void);
	bool			Load( string sFileName, CChunk& chunk, IFileSystem& oFileSystem );
	void			Load( string sFileName, CTextureInfos& ti, IFileSystem& oFileSystem );
	void			Load( string sFileName, IRessourceInfos& ti, IFileSystem& oFileSystem );
	void			Load( string sFileName, IRessourceInfos& mi ){}
	void			Export( string sFileName, ILoader::IRessourceInfos& ri ){throw 1;}
};



#endif //TEXTURE_LOADER_H