#ifndef ICAMERAMANAGER_H
#define ICAMERAMANAGER_H

#include "EEPlugin.h"

typedef int FreeCamID;
typedef int LinkedCamID;

class ICamera;
class IRenderer;
class IEntityManager;

class ICameraManager : public CPlugin
{
protected:
	ICameraManager() : CPlugin( nullptr, ""){}

public:
	struct Desc : public CPlugin::Desc
	{
		std::vector< IRenderer* >	m_vRenderer;
		Desc( std::vector< IRenderer* > vRenderer ) : CPlugin::Desc( NULL, "" )	
		{
			for ( unsigned int i = 0; i < vRenderer.size(); i++ )
				m_vRenderer.push_back( vRenderer[ i ] );
		}
	};

	enum TCameraType
	{ 
		T_NONE,
		T_FREE_CAMERA, 
		T_LINKED_CAMERA,
		T_MAP_CAMERA,
		T_GUI_MAP_CAMERA,
		T_MAP_ENTITY_CAMERA,
		T_CHARACTER_EDITOR
	};

	virtual ICamera*		CreateCamera( TCameraType, float fFov, IEntityManager& oEntityManager ) = 0;
	virtual void			SetActiveCamera( ICamera* ) = 0;
	virtual ICamera*		GetActiveCamera() = 0;
	virtual TCameraType		GetCameraType( ICamera* pCamera ) = 0;
	virtual ICamera*		GetCameraFromType( TCameraType type ) = 0;
	virtual void			UnlinkCameras() = 0;
};

#endif // ICAMERAMANAGER_H