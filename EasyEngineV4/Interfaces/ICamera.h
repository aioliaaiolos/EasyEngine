#ifndef ICAMERA_H
#define ICAMERA_H

#define ENTITYCAMERA


#ifdef ENTITYCAMERA
#include "IEntity.h"
#endif // ENTITYCAMERA

#pragma warning(disable:4250)



class ICamera : virtual public IEntity
{

public:
	virtual float	GetSpeed() = 0;
	virtual void	SetSpeed( float fSpeed ) = 0;
	virtual void	Freeze( bool bFreeze ) = 0;
	virtual void	Move( float fOffsetYaw, float fOffsetPitch, float fOffsetRoll, float fAvanceOffet, float fLeftOffset, float fUpOffset ) = 0;
	virtual float	GetFov() = 0;
	virtual void	Zoom(int value) = 0;
	virtual void	SetInventoryMode(bool bInventoryMode) = 0;
	virtual void	DisplayViewCone(bool display) = 0;
};


class ICameraManager : public CPlugin
{
protected:
	ICameraManager() : CPlugin(nullptr, "") {}

public:
	struct Desc : public CPlugin::Desc
	{
		std::vector< IRenderer* >	m_vRenderer;
		Desc(std::vector< IRenderer* > vRenderer) : CPlugin::Desc(NULL, "")
		{
			for (unsigned int i = 0; i < vRenderer.size(); i++)
				m_vRenderer.push_back(vRenderer[i]);
		}
	};

	enum TCameraType
	{
		T_NONE,
		TFree,
		TLinked,
		TMap,
		TGuiMap,
		TMapEntity,
		TEditor
	};

	virtual ICamera*		CreateCamera(TCameraType, float fFov) = 0;
	virtual void			SetActiveCamera(ICamera*) = 0;
	virtual ICamera*		GetActiveCamera() = 0;
	virtual TCameraType		GetCameraType(ICamera* pCamera) = 0;
	virtual ICamera*		GetCameraFromType(TCameraType type) = 0;
	virtual void			UnlinkCameras() = 0;
	virtual float			GetCameraBaseFov() = 0;
};


#endif // ICAMERA_H