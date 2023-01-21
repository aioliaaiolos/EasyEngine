#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include <vector>
#include <map>
#include "ICamera.h"

class CNode;
class IRenderer;
class ICamera;

using namespace std;

class CCameraManager : public ICameraManager
{
public:
	CCameraManager(EEInterface& oInterface);
	~CCameraManager();
	ICamera*		CreateCamera( TCameraType, float fFov);
	void			SetActiveCamera( ICamera* pCamera );
	ICamera*		GetActiveCamera();
	TCameraType		GetCameraType( ICamera* pCamera ) ;
	ICamera*		GetCameraFromType( TCameraType type );
	string			GetName() override;
	void			UnlinkCameras() override;

private:
	
	static void		HandleEntityManagerCreated(CPlugin* plugin, IObject* pData);

	map< TCameraType, ICamera* >	m_mCamera;
	map< ICamera*, TCameraType >	m_mCameraType;
	ICamera*						m_pActiveCamera;
	vector< IRenderer*	>			m_vRenderer;
	EEInterface&					m_oInterface;
	IEntityManager*					m_pEntityManager;
};
extern "C" _declspec(dllexport) ICameraManager* CreateCameraManager(EEInterface& oInterface);

#endif // CAMERAMANAGER_H