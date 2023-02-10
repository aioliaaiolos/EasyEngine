#ifndef MATERIAL_CPP
#ifndef MESH_CPP
#ifndef TEXTURE_CPP
#ifndef RESSOURCEMANAGER_CPP
#error
#endif
#endif
#endif
#endif

#ifndef TEXTURE_H
#define TEXTURE_H

#include "IRessource.h"
#include "IRenderer.h"


class CTextureBase : public ITexture
{
public:
	struct CDesc
	{
		float*						m_pData;
		int							m_nUnitTexture;
		IRenderer::TPixelFormat		m_eFormat;
		int							m_nFrameBufferObjectId;
		string						m_sName;
		IShader*					m_pShader = nullptr;
		IRenderer&					m_oRenderer;
		CDesc(IRenderer& oRenderer, IShader* pShader, int nUnitTexture);
	};
	CTextureBase(const CDesc& oDesc);
	virtual void		Update() = 0;
	void				SetShader(IShader* pShader);
	unsigned int		GetFrameBufferObjectId();
	void				SetUnitTexture(int nUnitTexture);
	void				SetUnitName(string sUnitName);

protected:
	int							m_nID;
	int							m_nUnitTexture;
	IShader*					m_pShader;
	int							m_nFrameBufferObjectId;
	string						m_sUnitName;
	IRenderer&					m_oRenderer;
};

class CTexture1D : public CTextureBase
{
	int			m_nSize;
	bool		m_bFirstUpdate;
public:
	struct CDesc : public CTextureBase::CDesc
	{
		int		m_nSize;
		CDesc( IRenderer& oRenderer, IShader* pShader, int nUnitTexture );
	};

				CTexture1D( CDesc& oDesc );
	void		GetDimension( int& nWidth, int& nHeight );
	void		Update();
	IShader*	GetShader() const;
};


class CTexture2D : public CTextureBase
{
	
	int							m_nWidth;
	int							m_nHeight;
	int							m_nReponse;

public:

	struct CDesc : public CTextureBase::CDesc
	{		
		vector< unsigned char >		m_vTexels;
		int							m_nWidth;
		int							m_nHeight;
		bool						m_bGenerateMipmaps;
		bool						m_bRenderTexture;
		int							m_nTextureId;
		CDesc( IRenderer& oRenderer, IShader* pShader, int nUnitTexture );
	};

								CTexture2D( CDesc& oDesc );
	void						Update();
	void						GetDimension( int& nWidth, int& nHeight );
	IShader*					GetShader() const;
};

#endif	//TEXTURE_H