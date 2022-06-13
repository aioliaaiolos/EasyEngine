#ifndef IWIDGET_H
#define IWIDGET_H

#include "EEPlugin.h"

class IWidget : public CPlugin
{
protected:
	IWidget():CPlugin(NULL, ""){}
public:
	struct Desc : public CPlugin::Desc
	{
		int 			m_nWidth;
		int 			m_nHeight;
		int 			m_nPosX;
		int 			m_nPosY;
		IWidget*		m_pParent;
		Desc( int nWidth, int nHeight, IWidget* pParent ) :
			CPlugin::Desc( NULL, "" ),
			m_nPosX( 0 ),
			m_nPosY( 0 ),
			m_nWidth( nWidth ),
			m_nHeight( nHeight ),
			m_pParent( pParent )
			{
			}
	};

	
	struct CallbackArgs
	{
		IWidget* m_pParent;
		UINT m_msg;
		WPARAM m_wParam;
		LPARAM m_lParam;
		CallbackArgs();
		CallbackArgs( IWidget* pParent, UINT msg, WPARAM wParam, LPARAM lParam ):
			m_pParent( pParent ),
			m_msg( msg ),
			m_wParam( wParam ),
			m_lParam( lParam )
			{
			}
	};

	virtual HWND	GetHandle() = 0;
	virtual void	GetDimension(unsigned int& nWidth, unsigned int& nHeight )const = 0;
	virtual void	AddChild( IWidget* pChild ) = 0;
	virtual void	CallCallback( CallbackArgs& args ) = 0;
};


#endif // IWIDGET_H