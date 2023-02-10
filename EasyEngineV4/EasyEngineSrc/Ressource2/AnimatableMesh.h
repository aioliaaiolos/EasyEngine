#include "Mesh.h"

class CAnimatableMesh : public IAnimatableMesh
{
	IBone*					m_pSkeleton;
	vector< CMesh* >		m_vMeshes;

public:

	~CAnimatableMesh();
	void 			Update();
	void			SetShader( IShader* pShader ) ;
	IShader*		GetShader() const;
	void			AddMesh( CMesh* pMesh );
	void			SetSkeleton( IBone* pBone );
	IMesh*			GetMesh( int nIndex );
	IBone*			GetSkeleton();
	unsigned int	GetMeshCount();
	

};