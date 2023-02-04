#ifndef WIDGET_H
#define WIDGET_H

#include <deque>

#include "IGUIManager.h"
#include "IInputManager.h"
#include "ILoader.h"
#include "../Utils2/Dimension.h"
#include "../Utils2/Position.h"

class IShaderManager;
class IRessourceManager;
class IRessource;
class IShader;
class IMesh;
class CRectangle;
class ITexture;
class IRenderer;

class CMesh;
class CListener;
class CGUIManager;

class CGUIWidget
{

public:
							CGUIWidget(EEInterface& oInterface);
							CGUIWidget(EEInterface& oInterface, int nWidth, int nHeight );
							CGUIWidget(EEInterface& oInterface, ITexture* pTexture, CRectangle& oSkin);
							CGUIWidget(EEInterface& oInterface, ITexture* pTexture, CRectangle& oSkin, ILoader::CMeshInfos& outMeshInfos, IRessource*& pOutMaterial);
							CGUIWidget(EEInterface& oInterface, string sFileNam);
							CGUIWidget(EEInterface& oInterface, string sFileName, int width, int height);
							CGUIWidget(EEInterface& oInterface, const CDimension& windowSize, const CRectangle& skin);
	virtual					~CGUIWidget(void);

	bool					operator==( const CGUIWidget& w );
	void					operator=(const CGUIWidget& w);

	void					SetQuad(IRessource* pMesh );
	IMesh*					GetQuad();
	virtual void			Display();
	
	CPosition				GetPosition()const;
	CPosition				GetRelativePosition() const;
	void					GetLogicalPosition( float& x, float& y, int nResWidth, int nResHeight ) const;
	void					GetLogicalDimension( float& x, float& y, int nResWidth, int nResHeight ) const;
	CDimension				GetDimension() const;

	virtual void			SetPosition(float fPosX, float fPosY);
	void					SetRelativePosition(const CPosition& oPosition);
	virtual void			SetRelativePosition(float fPosX, float fPosY);
	void					SetPosition(CPosition p);
	void					SetY(float fY);
	void					Translate(float fPosX, float fPosY);
	void					SetListener(CListener* pListener);
	virtual void			UpdateCallback(int nCursorXPos, int nCursorYPos, IInputManager::TMouseButtonState eButtonState);
	void					SetSkinName( const std::string& szSkinName );
	std::string				GetSkinName();
	CGUIWidget*				GetParent();
	virtual void			SetParent(CGUIWidget* parent);
	deque<CGUIWidget*>::iterator	Unlink(bool bDelete = true);
	void					UpdatePosition();
	void					SetVisibility(bool bVisible);

	string					m_sUserData;

	static void				Init( int nResX, int nResY, IShader* pShader );

protected:
	IMesh*					CreateQuadFromFile(IRenderer& oRenderer, IRessourceManager& oRessourceManager, string sTextureName, const CRectangle& skin) const;
	void					CreateQuadMeshInfosFromTexture(IRenderer& oRenderer, ITexture* pTexture, const CRectangle& oSkin, ILoader::CMeshInfos& mi, CRectangle& oFinalSkin) const;
	void					GetScreenCoordFromTexCoord(const CRectangle& oTexture, const CDimension& oScreenDim, CRectangle& oScreen) const;
	void					CreateQuadMeshInfos(IRenderer& oRenderer, const CDimension& dimQuad, const CRectangle& oSkin, ILoader::CMeshInfos& mi) const;
	IMesh*					CreateQuadFromTexture(IRenderer& oRenderer, IRessourceManager& oRessourceManager, ITexture* pTexture, const CRectangle& oSkin) const;
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
	EEInterface&			m_oInterface;
	bool					m_bVisible;
	static int				s_nScreenResWidth;
	static int				s_nScreenResHeight;
	static IShader*			s_pShader;

};

class CLink : public CGUIWidget
{
public:
	enum TState
	{
		eNormal = 0,
		eClick,
		eHover
	};

	typedef void(*TItemSelectedCallback)(CLink*);

	CLink(EEInterface& oInterface, string sText, int nMaxWidth = -1);
	virtual ~CLink();
	void SetText(string sText);
	void Display();
	void SetColorByState(TState s, IGUIManager::TFontColor color);
	void SetClickedCallback(TItemSelectedCallback callback);
	void GetText(string& sText) const;
	int GetLineCount() const;
	const string& GetText() const;

private:

	void	ChangeColor(IGUIManager::TFontColor color);
	static void OnLinkEvent(IGUIManager::ENUM_EVENT nEvent, CGUIWidget*, int, int);

	string									m_sText;
	CGUIManager&							m_oGUIManager;
	map<TState, IGUIManager::TFontColor>	m_mColorByState;
	TState									m_eCurrentState = eNormal;
	TItemSelectedCallback					m_pClickCallback = nullptr;
	CListener*								m_pListener = nullptr;
	int										m_nLineCount = 0;
};


#endif