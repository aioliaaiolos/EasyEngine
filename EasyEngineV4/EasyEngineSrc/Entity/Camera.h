#ifndef CAMERA_H
#define CAMERA_H

#include "ICamera.h"
#include "Entity.h"

class CCamera : public ICamera, public CEntity
{
public:
	CCamera(EEInterface& oInterface, float fFov);
	virtual ~CCamera();
	void				Freeze( bool bFreeze);	
	float				GetFov();
	void				Update();
	float				GetWeight() { return 0.f; }
	float				GetBoundingSphereRadius() const { return 0.f; }
	void				DisplayViewCone(bool display);

protected:

	virtual void		DisplayViewCone();
	
	bool				m_bFreeze;
	float				m_fFov;
	bool				m_bDisplayViewCone;
	IRenderer&			m_oRenderer;
};

#endif // CAMERA_H