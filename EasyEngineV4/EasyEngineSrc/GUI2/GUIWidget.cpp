#define GUIWIDGET_CPP

#include "IRessource.h"
#include "Interface.h"
#include "IShader.h"
#include "GUIManager.h"
#include "GUIWidget.h"
#include "listener.h"
#include "Exception.h"
#include "Utils2/Rectangle.h"
#include "GUIWindow.h"

using namespace std;

int CGUIWidget::s_nScreenResWidth = 0;
int CGUIWidget::s_nScreenResHeight = 0;
IShader* CGUIWidget::s_pShader = NULL;


void CGUIWidget::Init( int nResX, int nResY, IShader* pShader )
{
	s_nScreenResWidth = nResX;
	s_nScreenResHeight = nResY;
	s_pShader = pShader;
}

CGUIWidget::CGUIWidget() :
	CGUIWidget(0, 0)
{

}

CGUIWidget::CGUIWidget( int nWidth, int nHeight ):
_pListener(NULL),
_bIsCursorInWidget( NULL ),
m_pMesh( NULL ),
m_pParent(NULL)
{
	if( s_pShader == NULL )
	{
		CWidgetNotInitialized e( "" );
		throw e;
	}
	m_oDimension.SetDimension( (float) nWidth, (float)nHeight);
	m_pShader = s_pShader;
}

CGUIWidget::CGUIWidget(EEInterface& oInterface, ITexture* pTexture, CRectangle& oSkin):
_pListener(NULL),
_bIsCursorInWidget(NULL),
m_pMesh(NULL),
m_pParent(NULL)
{
	InitManagers(oInterface);
	ILoader::CMeshInfos mi;
	CRectangle oFinalSkin;
	CreateQuadMeshInfosFromTexture(*m_pRenderer, pTexture, oSkin, mi, oFinalSkin);

	ILoader::CAnimatableMeshData oData;
	oData.m_vMeshes.push_back(mi);
	IRessource* pMaterial = m_pRessourceManager->CreateMaterial(mi.m_oMaterialInfos, pTexture);
	IAnimatableMesh* pARect = m_pRessourceManager->CreateMesh(oData, pMaterial);
	IMesh* pRect = pARect->GetMesh(0);
	
	if (s_pShader == NULL)
	{
		CWidgetNotInitialized e("");
		throw e;
	}
	m_oDimension.SetDimension((float)oFinalSkin.m_oDim.GetWidth(), (float)oFinalSkin.m_oDim.GetWidth());
	m_pShader = s_pShader;
	
	SetQuad(pRect);
}

CGUIWidget::CGUIWidget(
	EEInterface& oInterface,
	ITexture* pTexture, 
	CRectangle& oSkin, 
	ILoader::CMeshInfos& outMeshInfos, 
	IRessource*& pOutMaterial) :
_pListener(NULL),
_bIsCursorInWidget(NULL),
m_pMesh(NULL),
m_pParent(NULL)
{
	InitManagers(oInterface);

	CRectangle oFinalSkin;
	CreateQuadMeshInfosFromTexture(*m_pRenderer, pTexture, oSkin, outMeshInfos, oFinalSkin);

	ILoader::CAnimatableMeshData oData;
	oData.m_vMeshes.push_back(outMeshInfos);
	pOutMaterial = m_pRessourceManager->CreateMaterial(outMeshInfos.m_oMaterialInfos, pTexture);
	IAnimatableMesh* pARect = m_pRessourceManager->CreateMesh(oData, pOutMaterial);
	IMesh* pRect = pARect->GetMesh(0);

	if (s_pShader == NULL)
	{
		CWidgetNotInitialized e("");
		throw e;
	}
	m_oDimension.SetDimension((float)oFinalSkin.m_oDim.GetWidth(), (float)oFinalSkin.m_oDim.GetHeight());
	m_pShader = s_pShader;

	SetQuad(pRect);
}

CGUIWidget::CGUIWidget(EEInterface& oInterface, string sFileName, int width, int height):
CGUIWidget(width, height)
{
	InitManagers(oInterface);
	CRectangle oSkin;
	oSkin.SetDimension(width, height);
	IMesh* pQuad = CreateQuadFromFile(*m_pRenderer, *m_pRessourceManager, sFileName, oSkin, oSkin.m_oDim);
	SetQuad(pQuad);
}

CGUIWidget::CGUIWidget(EEInterface& oInterface, const CDimension& windowSize, const CRectangle& skin) :
	CGUIWidget(windowSize.GetWidth(), windowSize.GetHeight())
{
	InitManagers(oInterface);
	IMesh* pRect = CreateQuad(*m_pRenderer, *m_pRessourceManager, windowSize, skin);
	SetQuad(pRect);
}

CGUIWidget::~CGUIWidget(void)
{
	delete _pListener;
	_pListener = nullptr;
}

void CGUIWidget::InitManagers(EEInterface& oInterface)
{
	m_pRenderer = static_cast<IRenderer*>(oInterface.GetPlugin("Renderer"));
	m_pRessourceManager = static_cast<IRessourceManager*>(oInterface.GetPlugin("RessourceManager"));
}

CGUIWidget* CGUIWidget::GetParent()
{
	return m_pParent;
}

void CGUIWidget::SetParent(CGUIWidget* parent)
{
	m_pParent = parent;
}

deque<CGUIWidget*>::iterator CGUIWidget::Unlink()
{
	CGUIWindow* pParent = static_cast<CGUIWindow*>(m_pParent);
	return pParent->RemoveWidget(this);
}

bool CGUIWidget::operator==( const CGUIWidget& w )
{
	return *m_pMesh == *w.m_pMesh;
}

void CGUIWidget::SetQuad( IRessource* pMesh )
{
	m_pMesh = static_cast< IMesh* >( pMesh );
}

IMesh* CGUIWidget::GetQuad()
{
	return static_cast< IMesh* >(m_pMesh);
}

void CGUIWidget::Display()
{
	if (m_pMesh) {
		float fWidgetLogicalPosx, fWidgetLogicalPosy;
		GetLogicalPosition(fWidgetLogicalPosx, fWidgetLogicalPosy, s_nScreenResWidth, s_nScreenResHeight);
		vector< float > vPos;
		vPos.push_back(fWidgetLogicalPosx);
		vPos.push_back(fWidgetLogicalPosy);
		m_pShader->SendUniformVec2Array("vImagePosition", vPos);
		m_pMesh->Update();
	}
}


void CGUIWidget::SetPosition(float fPosX, float fPosY)
{
	m_oPosition.SetX(fPosX);
	m_oPosition.SetY(fPosY);
}

void CGUIWidget::SetRelativePosition( float fPosX, float fPosY )
{
	m_oRelativePosition.SetX(fPosX);
	m_oRelativePosition.SetY(fPosY);
	float parentPosX = 0.f;
	float parentPosY = 0.f;
	if (m_pParent) {
		parentPosX = m_pParent->GetPosition().GetX();
		parentPosY = m_pParent->GetPosition().GetY();
	}
	m_oPosition.SetX(parentPosX + m_oRelativePosition.GetX());
	m_oPosition.SetY(parentPosY + m_oRelativePosition.GetY());
}

void CGUIWidget::UpdatePosition()
{
	SetRelativePosition(m_oRelativePosition.GetX(), m_oRelativePosition.GetY());
}

void CGUIWidget::Translate(float dx, float dy)
{
	m_oPosition.SetX(m_oPosition.GetX() + dx);
	m_oPosition.SetY(m_oPosition.GetY() + dy);
}

void CGUIWidget::SetY( float fY )
{
	m_oPosition.SetY( fY );
}

CPosition CGUIWidget::GetPosition() const
{
	return m_oPosition;
}

CPosition CGUIWidget::GetRelativePosition() const
{
	return m_oRelativePosition;
}

void CGUIWidget::GetLogicalPosition( float& x, float& y, int nResWidth, int nResHeight ) const
{
	x = 2.f*(float)m_oPosition.GetX() / nResWidth;
	y = - 2.f*(float)m_oPosition.GetY() / nResHeight;
}

void CGUIWidget::GetLogicalDimension( float& x, float& y, int nResWidth, int nResHeight ) const
{
	x = 2.f*(float)m_oDimension.GetWidth() / nResWidth;
	y = 2.f*(float)m_oDimension.GetHeight() / nResHeight;
}

CDimension CGUIWidget::GetDimension() const
{
	return m_oDimension;
}

void CGUIWidget::SetListener(CListener* pListener)
{
	_pListener = pListener;
}

void CGUIWidget::UpdateCallback(int nCursorXPos, int nCursorYPos, IInputManager::TMouseButtonState eButtonState)
{
	bool bIsCursorInWidget = false;
	if (_pListener)
	{
		_pListener->ExecuteCallBack(IGUIManager::EVENT_OUTSIDE, this, nCursorXPos, nCursorYPos);
		if (nCursorXPos > m_oPosition.GetX() && nCursorXPos < m_oPosition.GetX() + m_oDimension.GetWidth())
		{
			if (nCursorYPos > m_oPosition.GetY() && nCursorYPos < m_oPosition.GetY() + m_oDimension.GetHeight())
			{
				bIsCursorInWidget = true;
				if (!_bIsCursorInWidget)
				{
					_pListener->ExecuteCallBack( IGUIManager::EVENT_MOUSEENTERED, this, nCursorXPos, nCursorYPos);
					_bIsCursorInWidget = true;
					return;
				}
				if (eButtonState == IInputManager::eMouseButtonStateDown || eButtonState == IInputManager::eMouseButtonStateJustDown)
				{
					_pListener->ExecuteCallBack( IGUIManager::EVENT_LMOUSECLICK, this, nCursorXPos, nCursorYPos);
					return;
				}
				if (eButtonState == IInputManager::eMouseButtonStateJustUp)
				{
					_pListener->ExecuteCallBack( IGUIManager::EVENT_LMOUSERELEASED, this, nCursorXPos, nCursorYPos);
					return;
				}
				if ( m_oNextCursorPos.GetX() != nCursorXPos || m_oNextCursorPos.GetY() != nCursorYPos )
				{
					_pListener->ExecuteCallBack( IGUIManager::EVENT_MOUSEMOVE, this, nCursorXPos, nCursorYPos);
					m_oNextCursorPos.SetPosition(static_cast<float> (nCursorXPos), static_cast<float> (nCursorYPos) );
					return;
				}
				_pListener->ExecuteCallBack(IGUIManager::EVENT_NONE, this, nCursorXPos, nCursorYPos);
			}
		}	
		if ( !bIsCursorInWidget && _bIsCursorInWidget )
		{
			_pListener->ExecuteCallBack( IGUIManager::EVENT_MOUSEEXITED, this, nCursorXPos, nCursorYPos);
			_bIsCursorInWidget = false;
			return;
		}
		
	}
}


void CGUIWidget::SetSkinName(const string& szSkinName)
{
	_strSkinName = string (szSkinName);
}


string CGUIWidget::GetSkinName()
{
	return _strSkinName;
}

void CGUIWidget::SetPosition(CPosition p)
{
	m_oPosition = p;
}

void CGUIWidget::CreateQuadMeshInfosFromTexture(IRenderer& oRenderer, ITexture* pTexture, const CRectangle& oSkin, ILoader::CMeshInfos& mi, CRectangle& oFinalSkin) const
{
	string sShaderName = "gui";
	IShader* pShader = oRenderer.GetShader(sShaderName);

	int nTexDimWidth = 0, nTexDimHeight = 0;
	pTexture->GetDimension(nTexDimWidth, nTexDimHeight);
	oFinalSkin = oSkin;
	if (oSkin.m_oDim.GetWidth() == 0 || oSkin.m_oDim.GetHeight() == 0)
		oFinalSkin.SetDimension(CDimension(nTexDimWidth, nTexDimHeight));

	pTexture->SetShader(pShader);

	CRectangle oScreenRect;
	unsigned int nResWidth, nResHeight;
	oRenderer.GetResolution(nResWidth, nResHeight);
	CDimension oScreenDim(nResWidth, nResHeight);
	GetScreenCoordFromTexCoord(oFinalSkin, oScreenDim, oScreenRect);

	float fScreenXmin = oScreenRect.m_oPos.GetX();
	float fScreenYmin = oScreenRect.m_oPos.GetY();
	float fScreenXmax = fScreenXmin + oScreenRect.m_oDim.GetWidth();
	float fScreenYmax = fScreenYmin + oScreenRect.m_oDim.GetHeight();

	mi.m_vVertex.push_back(fScreenXmin); mi.m_vVertex.push_back(fScreenYmin); mi.m_vVertex.push_back(0);
	mi.m_vVertex.push_back(fScreenXmax); mi.m_vVertex.push_back(fScreenYmin); mi.m_vVertex.push_back(0);
	mi.m_vVertex.push_back(fScreenXmin); mi.m_vVertex.push_back(fScreenYmax); mi.m_vVertex.push_back(0);
	mi.m_vVertex.push_back(fScreenXmax); mi.m_vVertex.push_back(fScreenYmax); mi.m_vVertex.push_back(0);

	mi.m_vIndex.push_back(2); mi.m_vIndex.push_back(0); mi.m_vIndex.push_back(3);
	mi.m_vIndex.push_back(1); mi.m_vIndex.push_back(3); mi.m_vIndex.push_back(0);

	CRectangle oTextureRect(0, 0, nTexDimWidth, nTexDimHeight);
	fScreenXmax = 1 - (float)oFinalSkin.m_oPos.GetX() / (float)oTextureRect.m_oDim.GetWidth();
	fScreenYmax = 1 - (float)oFinalSkin.m_oPos.GetY() / (float)oTextureRect.m_oDim.GetHeight();
	fScreenXmin = 1 - (float)(oFinalSkin.m_oPos.GetX() + oFinalSkin.m_oDim.GetWidth()) / (float)(oTextureRect.m_oDim.GetWidth());
	fScreenYmin = 1 - (float)(oFinalSkin.m_oPos.GetY() + oFinalSkin.m_oDim.GetHeight()) / (float)(oTextureRect.m_oDim.GetHeight());
	float pUVVertex[8] = { fScreenXmin, fScreenYmin, fScreenXmax, fScreenYmin, fScreenXmin, fScreenYmax, fScreenXmax, fScreenYmax };
	for (int i = 0; i < 8; i++)
		mi.m_vUVVertex.push_back(pUVVertex[i]);
	mi.m_vUVIndex.push_back(3); mi.m_vUVIndex.push_back(1); mi.m_vUVIndex.push_back(2);
	mi.m_vUVIndex.push_back(0); mi.m_vUVIndex.push_back(2); mi.m_vUVIndex.push_back(1);

	for (int i = 0; i < 6; i++)
		mi.m_vNormalFace.push_back(0.f);

	for (int i = 0; i < 18; i++)
		mi.m_vNormalVertex.push_back(0.f);

	mi.m_bCanBeIndexed = false;
	mi.m_oMaterialInfos.m_sShaderName = sShaderName;
	mi.m_sShaderName = sShaderName;
}


void CGUIWidget::GetScreenCoordFromTexCoord(const CRectangle& oTexture, const CDimension& oScreenDim, CRectangle& oScreen) const
{
	int nResWidth = oScreenDim.GetWidth(), nResHeight = oScreenDim.GetHeight();
	oScreen.m_oPos.SetX(-1);
	float fWidthRatio = 2 * (float)oTexture.m_oDim.GetWidth() / (float)nResWidth;
	float fHeightRatio = 2 * (float)oTexture.m_oDim.GetHeight() / (float)nResHeight;
	oScreen.m_oPos.SetY(1 - fHeightRatio);
	oScreen.m_oDim.SetWidth(fWidthRatio);
	oScreen.m_oDim.SetHeight(fHeightRatio);
}

void CGUIWidget::CreateQuadMeshInfos(IRenderer& oRenderer, const CDimension& quadSize, const CRectangle& oSkin, ILoader::CMeshInfos& mi) const
{
	CRectangle oFinalSkin = oSkin;
	if (oSkin.m_oDim.GetWidth() == 0 || oSkin.m_oDim.GetHeight() == 0)
		oFinalSkin.SetDimension(CDimension(quadSize.GetWidth(), quadSize.GetHeight()));

	CRectangle oScreenRect;
	unsigned int nResWidth, nResHeight;
	oRenderer.GetResolution(nResWidth, nResHeight);
	CDimension oScreenDim(nResWidth, nResHeight);
	GetScreenCoordFromTexCoord(oFinalSkin, oScreenDim, oScreenRect);

	float fScreenXmin = oScreenRect.m_oPos.GetX();
	float fScreenYmin = oScreenRect.m_oPos.GetY();
	float fScreenXmax = fScreenXmin + oScreenRect.m_oDim.GetWidth();
	float fScreenYmax = fScreenYmin + oScreenRect.m_oDim.GetHeight();

	mi.m_vVertex.push_back(fScreenXmin); mi.m_vVertex.push_back(fScreenYmin); mi.m_vVertex.push_back(0);
	mi.m_vVertex.push_back(fScreenXmax); mi.m_vVertex.push_back(fScreenYmin); mi.m_vVertex.push_back(0);
	mi.m_vVertex.push_back(fScreenXmin); mi.m_vVertex.push_back(fScreenYmax); mi.m_vVertex.push_back(0);
	mi.m_vVertex.push_back(fScreenXmax); mi.m_vVertex.push_back(fScreenYmax); mi.m_vVertex.push_back(0);

	mi.m_vIndex.push_back(2); mi.m_vIndex.push_back(0); mi.m_vIndex.push_back(3);
	mi.m_vIndex.push_back(1); mi.m_vIndex.push_back(3); mi.m_vIndex.push_back(0);

	CRectangle oTextureRect(0, 0, quadSize.GetWidth(), quadSize.GetHeight());
	fScreenXmax = 1 - (float)oFinalSkin.m_oPos.GetX() / (float)oTextureRect.m_oDim.GetWidth();
	fScreenYmax = 1 - (float)oFinalSkin.m_oPos.GetY() / (float)oTextureRect.m_oDim.GetHeight();
	fScreenXmin = 1 - (float)(oFinalSkin.m_oPos.GetX() + oFinalSkin.m_oDim.GetWidth()) / (float)(oTextureRect.m_oDim.GetWidth());
	fScreenYmin = 1 - (float)(oFinalSkin.m_oPos.GetY() + oFinalSkin.m_oDim.GetHeight()) / (float)(oTextureRect.m_oDim.GetHeight());
	float pUVVertex[8] = { fScreenXmin, fScreenYmin, fScreenXmax, fScreenYmin, fScreenXmin, fScreenYmax, fScreenXmax, fScreenYmax };
	float pUVVertex2[8] = { fScreenXmax, fScreenYmin, fScreenXmin, fScreenYmin, fScreenXmax, fScreenYmax, fScreenXmin, fScreenYmax };
	for (int i = 0; i < 8; i++)
		mi.m_vUVVertex.push_back(pUVVertex2[i]);
	mi.m_vUVIndex.push_back(3); mi.m_vUVIndex.push_back(1); mi.m_vUVIndex.push_back(2);
	mi.m_vUVIndex.push_back(0); mi.m_vUVIndex.push_back(2); mi.m_vUVIndex.push_back(1);

	for (int i = 0; i < 6; i++)
		mi.m_vNormalFace.push_back(0.f);

	for (int i = 0; i < 18; i++)
		mi.m_vNormalVertex.push_back(0.f);

	mi.m_bCanBeIndexed = false;
}

IMesh* CGUIWidget::CreateQuadFromTexture(IRenderer& oRenderer, IRessourceManager& oRessourceManager, ITexture* pTexture, const CRectangle& oSkin, const CDimension& oImageSize) const
{
	CRectangle oFinalSkin;
	ILoader::CMeshInfos mi;
	CreateQuadMeshInfosFromTexture(oRenderer, pTexture, oSkin, mi, oFinalSkin);

	ILoader::CAnimatableMeshData oData;
	oData.m_vMeshes.push_back(mi);
	IRessource* pMaterial = oRessourceManager.CreateMaterial(mi.m_oMaterialInfos, pTexture);
	IAnimatableMesh* pARect = oRessourceManager.CreateMesh(oData, pMaterial);
	IMesh* pRect = pARect->GetMesh(0);
	return pRect;
}

IMesh* CGUIWidget::CreateQuad(IRenderer& oRenderer, IRessourceManager& oRessourceManager, const CDimension& oQuadSize, const CRectangle& skin) const
{
	CRectangle oFinalSkin;
	ILoader::CMeshInfos mi;
	CreateQuadMeshInfos(oRenderer, oQuadSize, skin, mi);
	ILoader::CAnimatableMeshData oData;
	oData.m_vMeshes.push_back(mi);

	IRessource* pMaterial = oRessourceManager.CreateMaterial(mi.m_oMaterialInfos, NULL);

	IAnimatableMesh* pARect = oRessourceManager.CreateMesh(oData, pMaterial);
	IMesh* pRect = pARect->GetMesh(0);
	pRect->SetShader(oRenderer.GetShader("gui"));
	return pRect;
}


IMesh* CGUIWidget::CreateQuadFromFile(IRenderer& oRenderer, IRessourceManager& oRessourceManager, string sTextureName, const CRectangle& skin, const CDimension& oImageSize) const
{
	ITexture* pTexture = static_cast< ITexture* > (oRessourceManager.GetRessource(sTextureName));
	return CreateQuadFromTexture(oRenderer, oRessourceManager, pTexture, skin, oImageSize);
}

CLink::CLink(EEInterface& oInterface, string sText) :
	CGUIWidget(0, 0),
	m_oGUIManager(static_cast<CGUIManager&>(*oInterface.GetPlugin("GUIManager")))
{
	m_sText = sText;
	int nWidth = 0;
	for (char& c : m_sText) {
		nWidth += m_oGUIManager.GetLetterEspacementX(c) + 1;
	}
	
	m_oDimension.SetWidth(nWidth);
	m_oDimension.SetHeight(m_oGUIManager.GetCurrentFontEspacementY());
	IAnimatableMesh* pARect = m_oGUIManager.CreateTextMeshes(m_sText, IGUIManager::TFontColor::eRed);
	SetQuad(pARect->GetMesh(0));

	m_pListener = new CListener;
	this->SetListener(m_pListener);
	m_pListener->SetEventCallBack(OnLinkEvent);
	m_mColorByState[TState::eNormal] = IGUIManager::TFontColor::eRed;	
	m_mColorByState[TState::eHover] = IGUIManager::TFontColor::eTurquoise;
	m_mColorByState[TState::eClick] = IGUIManager::TFontColor::eYellow;
}

CLink::~CLink()
{
}

void CLink::Display()
{
	CGUIWidget::Display();
}

void CLink::ChangeColor(IGUIManager::TFontColor color)
{
	IRessource* pMaterial = m_oGUIManager.GetFontMaterial(color);
	m_pMesh->SetMaterial(pMaterial);
}

void CLink::OnLinkEvent(IGUIManager::ENUM_EVENT eEvent, CGUIWidget* pWidget, int x, int y)
{
	CLink* pThisLink = static_cast<CLink*>(pWidget);
	switch (eEvent) {
	case IGUIManager::ENUM_EVENT::EVENT_LMOUSECLICK:
		pThisLink->ChangeColor(pThisLink->m_mColorByState[TState::eClick]);
		break;
	case IGUIManager::ENUM_EVENT::EVENT_LMOUSERELEASED:
		pThisLink->ChangeColor(pThisLink->m_mColorByState[TState::eHover]);
		pThisLink->m_pClickCallback(pThisLink);
		break;
	case IGUIManager::ENUM_EVENT::EVENT_MOUSEENTERED:
		pThisLink->ChangeColor(pThisLink->m_mColorByState[TState::eHover]);
		break;
	case IGUIManager::ENUM_EVENT::EVENT_MOUSEEXITED:
		pThisLink->ChangeColor(pThisLink->m_mColorByState[TState::eNormal]);
		break;
	}
}

void CLink::SetText(string sText)
{
	m_sText = sText;
}

void CLink::SetColorByState(CLink::TState s, IGUIManager::TFontColor color)
{
	m_mColorByState[s] = color;
	if (s == CLink::TState::eNormal)
		ChangeColor(color);
}

void CLink::SetClickedCallback(TItemSelectedCallback callback)
{
	m_pClickCallback = callback;
}

void CLink::GetText(string& sText) const
{
	sText = m_sText;
}

const string& CLink::GetText() const
{
	return m_sText;
}