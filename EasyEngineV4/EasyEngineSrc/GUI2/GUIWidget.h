#ifndef WIDGET_H
#define WIDGET_H

#include <deque>

#include "Container.h"
#include "IInputManager.h"
#include "ILoader.h"
#include "../Utils2/Dimension.h"
#include "../Utils2/Position.h"

class CMesh;
class CListener;
class IShaderManager;
class IRessourceManager;
class IRessource;
class IShader;
class IMesh;
class CRectangle;
class ITexture;
class IRenderer;

class CGUIWidget
{

public:

							CGUIWidget( int nWidth, int nHeight );
							CGUIWidget(EEInterface& oInterface, ITexture* pTexture, CRectangle& oSkin);
							CGUIWidget(EEInterface& oInterface, ITexture* pTexture, CRectangle& oSkin, ILoader::CMeshInfos& outMeshInfos, IRessource*& pOutMaterial);
							CGUIWidget(EEInterface& oInterface, string sFileName, int width, int height);
							CGUIWidget(EEInterface& oInterface, const CDimension& windowSize, const CRectangle& skin);
	virtual					~CGUIWidget(void);

	bool					operator==( const CGUIWidget& w );

	void					SetQuad(IRessource* pMesh );
	IMesh*					GetQuad();
	virtual void			Display();
	
	CPosition				GetPosition()const;
	CPosition				GetRelativePosition() const;
	void					GetLogicalPosition( float& x, float& y, int nResWidth, int nResHeight ) const;
	void					GetLogicalDimension( float& x, float& y, int nResWidth, int nResHeight ) const;
	CDimension				GetDimension() const;

	virtual void			SetPosition(float fPosX, float fPosY);
	void					SetRelativePosition(float fPosX, float fPosY);
	void					SetPosition(CPosition p);
	void					SetY(float fY);
	void					Translate(float fPosX, float fPosY);
	void					SetListener(CListener* pListener);
	virtual void			UpdateCallback(int nCursorXPos, int nCursorYPos, IInputManager::TMouseButtonState eButtonState);
	void					SetSkinName( const std::string& szSkinName );
	std::string				GetSkinName();
	CGUIWidget*				GetParent();
	virtual void			SetParent(CGUIWidget* parent);
	deque<CGUIWidget*>::iterator	Unlink();

	static void				Init( int nResX, int nResY, IShader* pShader );

protected:
	IMesh*					CreateQuadFromFile(IRenderer& oRenderer, IRessourceManager& oRessourceManager, string sTextureName, const CRectangle& skin, const CDimension& oImageSize) const;
	void					CreateQuadMeshInfosFromTexture(IRenderer& oRenderer, ITexture* pTexture, const CRectangle& oSkin, ILoader::CMeshInfos& mi, CRectangle& oFinalSkin) const;
	void					GetScreenCoordFromTexCoord(const CRectangle& oTexture, const CDimension& oScreenDim, CRectangle& oScreen) const;
	void					CreateQuadMeshInfos(IRenderer& oRenderer, const CDimension& dimQuad, const CRectangle& oSkin, ILoader::CMeshInfos& mi) const;
	IMesh*					CreateQuadFromTexture(IRenderer& oRenderer, IRessourceManager& oRessourceManager, ITexture* pTexture, const CRectangle& oSkin, const CDimension& oImageSize) const;
	IMesh*					CreateQuad(IRenderer& oRenderer, IRessourceManager& oRessourceManager, const CDimension& quadSize, const CRectangle& skin) const;
	void					InitManagers(EEInterface& oInterface);

	CPosition				m_oNextCursorPos;
	CGUIWidget*				m_pParent;
	IShader*				m_pShader;
	IMesh*					m_pMesh;
	CDimension				m_oDimension;
	CPosition				m_oPosition;
	CPosition				m_oRelativePosition;
	CListener*				_pListener;
	bool					_bIsCursorInWidget;
	std::string				_strSkinName;
	IRenderer*				m_pRenderer;
	IRessourceManager*		m_pRessourceManager;
	static int				s_nScreenResWidth;
	static int				s_nScreenResHeight;
	static IShader*			s_pShader;

};

class CLink : public CGUIWidget
{
public:
	CLink(EEInterface& oInterface, string sText);
	void SetText(string sText);


private:
	string	m_sText;
};


#endif