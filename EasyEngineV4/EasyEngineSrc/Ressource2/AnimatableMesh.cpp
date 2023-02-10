#include "AnimatableMesh.h"

CAnimatableMesh::~CAnimatableMesh()
{
	for( int i = 0; i < GetMeshCount(); i++ )
		delete GetMesh( i );
}

void CAnimatableMesh::Update()
{
}

void CAnimatableMesh::SetShader( IShader* pShader ) 
{
}

IShader* CAnimatableMesh::GetShader() const
{
	return NULL;
}

void CAnimatableMesh::AddMesh( CMesh* pMesh )
{
	m_vMeshes.push_back( pMesh );
}

void CAnimatableMesh::SetSkeleton( IBone* pBone )
{
	m_pSkeleton = pBone;
}

IMesh* CAnimatableMesh::GetMesh( int nIndex )
{
	return m_vMeshes[ nIndex ];
}

IBone* CAnimatableMesh::GetSkeleton()
{
	return m_pSkeleton;
}

unsigned int CAnimatableMesh::GetMeshCount()
{
	return m_vMeshes.size();
}