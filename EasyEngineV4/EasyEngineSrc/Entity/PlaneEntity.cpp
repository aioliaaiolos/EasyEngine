#include "PlaneEntity.h"
#include "IRenderer.h"
#include "Interface.h"
#include "EEPlugin.h"

CPlaneEntity::CPlaneEntity(IRenderer& oRenderer, IRessourceManager& oRessourceManager, int slices, int size, string heightTexture, string diffuseTexture) : //CEntity(IRessourceManager& oRessourceManager, IRenderer& oRenderer, IEntityManager& oEntityManager, IGeometryManager& oGeometryManager, ICollisionManager& oCollisionManager)
	CShape(oRenderer),
	m_pMesh(nullptr)
{
	m_pMesh  = oRessourceManager.CreatePlane(slices, size, diffuseTexture);
	m_sEntityType = "PlaneEntity";
}


CPlaneEntity::~CPlaneEntity()
{
}

void CPlaneEntity::Update()
{
	CNode::Update();
	m_oRenderer.SetModelMatrix(m_oWorldMatrix);	

	m_oRenderer.CullFace(0);

	m_pMesh->SetDrawStyle(IRenderer::TDrawStyle::T_TRIANGLES);
	m_pMesh->Update();

	
	m_oRenderer.CullFace(1);
}

const string& CPlaneEntity::GetIDStr() const
{
	return m_sName;
}

void CPlaneEntity::Colorize(float r, float g, float b, float a)
{

}

const string& CPlaneEntity::GetTypeName() const
{
	return m_sEntityType;
}