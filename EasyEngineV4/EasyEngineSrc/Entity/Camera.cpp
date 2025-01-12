#include "Camera.h"
#include "Interface.h"

CCamera::CCamera(EEInterface& oInterface, float fFov):
CEntity(oInterface),
m_bFreeze( false ),
m_fFov( fFov ),
m_oRenderer(static_cast<IRenderer&>(*oInterface.GetPlugin("Renderer"))),
m_bDisplayViewCone(false)
{
}


CCamera::~CCamera()
{

}

void CCamera::Freeze( bool bFreeze )
{
	m_bFreeze = bFreeze;
}

float CCamera::GetFov()
{
	return m_fFov;
}

void CCamera::DisplayViewCone(bool display)
{
	m_bDisplayViewCone = display;
}

void CCamera::Update()
{
	CEntity::Update();
	if (m_bDisplayViewCone) {
		DisplayViewCone();
	}
}

void CCamera::DisplayViewCone()
{
	m_oRenderer.CullFace(0);
	CMatrix backupModelMatrix;
	m_oRenderer.GetModelMatrix(backupModelMatrix);
	m_oRenderer.SetModelMatrix(m_oWorldMatrix);
	m_oRenderer.DrawCylinder(50, 1, 100, 10, 10);
	m_oRenderer.CullFace(1);
	m_oRenderer.SetModelMatrix(backupModelMatrix);
}