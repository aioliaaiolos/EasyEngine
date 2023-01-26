#include "FreeCamera.h"
#include "IRenderer.h"
#include "Interface.h"

CFreeCamera::CFreeCamera(EEInterface& oInterface, float fFov, string sCamName):
CCamera(oInterface, fFov),
m_fYaw( 0.f ),
m_fPitch( 0.f ),
m_fSpeed(1.f),
m_bDisplayViewCone(false)
{
	IEntityManager* pEntityManager = static_cast<IEntityManager*>(oInterface.GetPlugin("EntityManager"));	
	m_sEntityID = sCamName;
	m_sName = sCamName;
	pEntityManager->AddEntity(this, sCamName);
}

CFreeCamera::~CFreeCamera()
{

}

float CFreeCamera::GetSpeed()
{
	return m_fSpeed;
}

void CFreeCamera::SetSpeed(float fSpeed)
{
	m_fSpeed = fSpeed;
}

void CFreeCamera::LocalTranslate(float x, float y, float z)
{
	CNode::LocalTranslate(m_fSpeed * x, m_fSpeed * y, m_fSpeed * z);
}

void CFreeCamera::Move( float fOffsetYaw, float fOffsetPitch, float fOffsetRoll, float fAvanceOffet, float fLeftOffset, float fUpOffset )
{
	if ( !m_bFreeze )
	{
		m_fYaw += fOffsetYaw;
		m_fPitch += fOffsetPitch;
		LocalTranslate( -fLeftOffset, fUpOffset, -fAvanceOffet );

		CVector vAvance = m_oLocalMatrix * CVector( 0,0,0,1 );
		m_oLocalMatrix = CMatrix::GetyRotation( m_fYaw ) * CMatrix::GetxRotation( m_fPitch );
		m_oLocalMatrix.SetPosition( vAvance.m_x, vAvance.m_y, vAvance.m_z );
	}
}

void CFreeCamera::GetEntityName(string& sName)
{
	sName = m_sEntityID;
}

void CFreeCamera::SetEntityName(string sName)
{
	m_sEntityID = sName;
}

void CFreeCamera::Zoom(int value)
{

}