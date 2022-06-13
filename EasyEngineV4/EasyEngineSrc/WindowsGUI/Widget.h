#ifndef WIDGET_H
#define WIDGET_H

#include <windows.h>
#include <vector>
#include "IWidget.h"
#include "EEPlugin.h"

class CMenu2;

class CWidget : public IWidget
{

	std::vector< IWidget* >		m_vChild;
	IWidget*					m_pParent;

protected:
	
	void						CreateWnd( DWORD dwExStyle, std::string sClassName, std::string sWindowName, 
											DWORD dwStyle, int nPosX, int nPosY, int nWidth, int nHeight, const CMenu2* pMenu );
	HWND						m_hWnd;
public:



								CWidget( const Desc& desc );
	virtual						~CWidget();
	
	void						GetPosition( int& x, int& y )const;
	void						GetClientPosition( int& x, int& y )const;
	void						GetDimension(unsigned int& nWidth, unsigned int& nHeight )const;
	void						GetClientDimension( int& nWidth, int& nHeight )const;
	int							GetChildCount()const;
	IWidget*					GetChild( int iIndex )const;
	HDC							GetDeviceContext()const;

	void						SetPosition( int x, int y );
	void						SetCaption( std::string sCaption );
	
	void						AddChild( IWidget* pChild );
	void						ForcePaint();
	void						SendMessage2( CallbackArgs );
	HWND						GetHandle()const;
};

#endif // WIDGET_H