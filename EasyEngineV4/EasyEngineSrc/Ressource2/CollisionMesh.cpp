#include "CollisionMesh.h"
#include "IGeometry.h"

CCollisionMesh::Desc::Desc(IRenderer& oRenderer, ILoader::CCollisionModelInfos& oCollisionModelInfos) :
	m_oCollisionModelInfos(oCollisionModelInfos)
{
}

CCollisionMesh::CCollisionMesh(CCollisionMesh::Desc& oDesc) :
	m_oCollisionModelInfos(oDesc.m_oCollisionModelInfos)
{
}

void CCollisionMesh::Update()
{

}

void CCollisionMesh::SetShader(IShader* pShader)
{

}

bool CCollisionMesh::IsCollide(IBox* pBox)
{
	for (int i = 0; i < m_oCollisionModelInfos.m_vPrimitives.size(); i++) {
		IGeometry* primitive = m_oCollisionModelInfos.m_vPrimitives[i];
		if (primitive->IsIntersect(*pBox))
			return true;
	}
	return false;
}

IGeometry* CCollisionMesh::GetGeometry(int index)
{
	if(index < m_oCollisionModelInfos.m_vPrimitives.size())
		return m_oCollisionModelInfos.m_vPrimitives[index];
	return NULL;
}

int CCollisionMesh::GetGeometryCount() const
{
	return (int)m_oCollisionModelInfos.m_vPrimitives.size();
}

IShader* CCollisionMesh::GetShader() const
{
	return NULL;
}
