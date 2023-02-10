#define CAMERAMANAGER_CPP
#include "Interface.h"
#include "CameraManager.h"
#include "FreeCamera.h"
#include "LinkedCamera.h"
#include "IRenderer.h"
#include "IEntity.h"

using namespace std;

CCameraManager::CCameraManager(EEInterface& oInterface) :
m_oInterface(oInterface),
m_pActiveCamera( NULL )
{
	IRenderer* pRenderer = static_cast<IRenderer*>(oInterface.GetPlugin("Renderer"));
	m_vRenderer.push_back(pRenderer);

	oInterface.HandlePluginCreation("EntityManager", [this](CPlugin* plugin)
	{
		m_pEntityManager = static_cast<IEntityManager*>(plugin);
	});
	
}

CCameraManager::~CCameraManager()
{
}

ICamera* CCameraManager::CreateCamera( TCameraType type, float fFov)
{
	ICamera* pCamera = NULL;
	string sCameraName;
	switch( type )
	{
	case ICameraManager::TFree:
		pCamera = new CFreeCamera(m_oInterface, fFov, "FreeCamera");
		sCameraName = "FreeCamera";
		/*if (!m_pActiveCamera)
			m_pActiveCamera = pCamera;*/
		break;
	case ICameraManager::TLinked:
		pCamera = new CLinkedCamera(m_oInterface, fFov);
		sCameraName = "LinkedCamera";
		break;
	case ICameraManager::TMap:
		pCamera = new CFreeCamera(m_oInterface, fFov, "MapCamera");
		sCameraName = "MapCamera";
		break;
	case ICameraManager::TGuiMap:
		pCamera = new CFreeCamera(m_oInterface, fFov, "GUIMapCamera");
		sCameraName = "GuiMapCamera";
		break;
	case ICameraManager::TEditor:
		pCamera = new CFreeCamera(m_oInterface, fFov, "EditorCamera");
		sCameraName = "CharacterEditorCamera";
		break;
	default:
		pCamera = new CFreeCamera(m_oInterface, fFov, "DefaultFreeCamera");
		sCameraName = "noNameCamera";
		break;
	}
	m_mCameraType[ pCamera ] = type;
	m_mCamera[ type ] = pCamera;
	m_pEntityManager->AddEntity( pCamera, sCameraName );
	return pCamera;
}

void CCameraManager::SetActiveCamera( ICamera* pCamera )
{
	m_pActiveCamera = pCamera;
	for (unsigned int i = 0; i < m_vRenderer.size(); i++) {
		if(pCamera)
			m_vRenderer.at(i)->SetFov(pCamera->GetFov());
	}
}

ICamera* CCameraManager::GetActiveCamera()
{
	return m_pActiveCamera;
}

ICameraManager::TCameraType	CCameraManager::GetCameraType( ICamera* pCamera ) 
{
	map< ICamera*, TCameraType >::iterator itCamera = m_mCameraType.find(pCamera);
	if (itCamera != m_mCameraType.end())
		return itCamera->second;
	return T_NONE;
}

ICamera* CCameraManager::GetCameraFromType( TCameraType type )
{
	map<ICameraManager::TCameraType, ICamera*>::iterator itCamera = m_mCamera.find(type);
	if (itCamera != m_mCamera.end())
		return itCamera->second;
	return nullptr;
}

string CCameraManager::GetName()
{
	return "CameraManager";
}

void CCameraManager::UnlinkCameras()
{
	for (map< TCameraType, ICamera* >::iterator it = m_mCamera.begin(); it != m_mCamera.end(); it++) {
		it->second->Unlink();
	}
	
}

extern "C" _declspec(dllexport) ICameraManager* CreateCameraManager(EEInterface& oInterface)
{
	return new CCameraManager(oInterface);
}