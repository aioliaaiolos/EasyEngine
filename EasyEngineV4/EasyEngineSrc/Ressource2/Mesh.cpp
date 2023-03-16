#define MESH_CPP

#include "mesh.h"
#include "ILoader.h"
#include "IRenderer.h"
#include "Texture.h"
#include "Material.h"
#include "Exception.h"
#include "IDrawTool.h"
#include "RessourceManager.h"
#include "Animation.h"
#include "IShader.h"
#include "define.h"
#include "../Utils2/RenderUtils.h"
#include "IGeometry.h"
#include <algorithm>

using namespace std;


CMesh::Desc::Desc( vector< float >& vVertexArray, vector<  unsigned int  >& vIndexArray,
				vector< float >& vUVVertexArray, vector< unsigned int >& vUVIndexArray, 
				vector< float >& vNormalVertexArray, IRenderer& oRenderer, IShader* pShader ):
m_vVertexArray( vVertexArray ),
m_vIndexArray( vIndexArray ),
m_vUVVertexArray( vUVVertexArray ),
m_vUVIndexArray( vUVIndexArray ),
m_vNormalVertexArray( vNormalVertexArray ),
m_bIndexed( false ),
m_pDrawTool( NULL ),
m_nParentBoneID( -1 ),
m_oRenderer(oRenderer)
{
}

CMesh::CMesh( const Desc& oDesc ):
m_bIndexedGeometry( oDesc.m_bIndexed ),
m_pBuffer( NULL ),
m_bSkinned( false ),
m_mMaterials( oDesc.m_mMaterials ),
m_bFirstUpdate( true ),
m_nReponse( -1 ),
m_nVertexWeightBufferID( -1 ),
m_nWeightedVertexIDBufferID( -1 ),
m_nFaceMaterialBufferID( -1 ),
m_pMaterialTexture( NULL ),
m_pBbox( oDesc.m_pBbox ),
m_bDrawBoundingBox( false ),
m_pShader( oDesc.m_pShader ),
m_nParentBoneID( oDesc.m_nParentBoneID ),
m_oOrgMaxPosition( oDesc.m_oOrgMaxPosition ),
m_eRenderType( IRenderer::eFill ),
m_mAnimationKeyBox( oDesc.m_mAnimationKeyBox ),
m_bDrawAnimationBoundingBox( false ),
m_pCurrentAnimationBoundingBox( NULL ),
m_eDrawStyle(IRenderer::T_TRIANGLES),
m_nEntityMatricesBufferID(-1),
m_oRenderer(oDesc.m_oRenderer)
{
	m_pShader->Enable( true );
	m_sName = oDesc.m_sName;
	
	if ( m_bIndexedGeometry )
	{
		if ( oDesc.m_vVertexWeight.size() > 0 )
		{
			m_nVertexWeightBufferID = m_oRenderer.CreateBuffer( (int)oDesc.m_vVertexWeight.size() );
			m_oRenderer.FillBuffer( oDesc.m_vVertexWeight, m_nVertexWeightBufferID );

			m_nWeightedVertexIDBufferID = m_oRenderer.CreateBuffer( (int)oDesc.m_vWeightedVertexID.size() );
			m_oRenderer.FillBuffer( oDesc.m_vWeightedVertexID, m_nWeightedVertexIDBufferID );
			m_bSkinned = true;
		}
		m_pBuffer = m_oRenderer.CreateIndexedGeometry( oDesc.m_vVertexArray, oDesc.m_vIndexArray, oDesc.m_vUVVertexArray,
			oDesc.m_vUVIndexArray, oDesc.m_vNormalVertexArray );
	}
	else
	{
		if ( oDesc.m_vVertexWeight.size() > 0 )
		{
			vector< float > vVertexWeight, vWeightedVertexID;
			CRenderUtils::CreateNonIndexedVertexArray( oDesc.m_vIndexArray, oDesc.m_vVertexWeight, 4, vVertexWeight );
			CRenderUtils::CreateNonIndexedVertexArray( oDesc.m_vIndexArray, oDesc.m_vWeightedVertexID, 4, vWeightedVertexID );

			m_nVertexWeightBufferID = m_oRenderer.CreateBuffer( (int)vVertexWeight.size() );
			m_oRenderer.FillBuffer( vVertexWeight, m_nVertexWeightBufferID );

			m_nWeightedVertexIDBufferID = m_oRenderer.CreateBuffer( (int)vWeightedVertexID.size() );
			m_oRenderer.FillBuffer( vWeightedVertexID, m_nWeightedVertexIDBufferID );

			m_bSkinned = true;
		}

		if (oDesc.m_vFaceMaterialID.size() > 0)
		{
			CMatrix m;
			int nUnitTexture = 3;
			int nMaterialIndex = 0;
			map<unsigned int, int> mMaterialIdToIndex;
			for (map< int, CMaterial* >::const_iterator itMat = oDesc.m_mMaterials.begin(); itMat != oDesc.m_mMaterials.end(); ++itMat) {
				CMaterial* pMaterial = itMat->second;
				pMaterial->GetMaterialMatrix(m);
				ITexture* pTexture = itMat->second->GetTexture();
				if (pTexture) {
					mMaterialIdToIndex[itMat->second->GetID()] = nMaterialIndex++;
					m.Get(m_vMaterialArray);
					m_vShininessArray.push_back(pMaterial->GetShininess());
					pTexture->SetUnitTexture(nUnitTexture++);
					m_vTextureArray.push_back(pTexture);
					m_vUnitTextures.push_back(pTexture->GetUnitTexture());
				}
			}

			vector<unsigned short> vFaceMaterialID = oDesc.m_vFaceMaterialID;
			for (int i = 0; i < vFaceMaterialID.size(); i++) {
				int id = vFaceMaterialID[i];
				vFaceMaterialID[i] = mMaterialIdToIndex[id];
			}

			vector<unsigned int> vNonIndexedFaceMaterialID;
			CreateNonIndexedMaterialVertexArray(vFaceMaterialID, oDesc.m_vIndexArray, vNonIndexedFaceMaterialID);
			m_nFaceMaterialBufferID = m_oRenderer.CreateBuffer((int)vNonIndexedFaceMaterialID.size());
			m_oRenderer.FillBuffer(vNonIndexedFaceMaterialID, m_nFaceMaterialBufferID);
		}
		m_pBuffer = m_oRenderer.CreateGeometry(oDesc.m_vVertexArray, oDesc.m_vIndexArray, oDesc.m_vUVVertexArray,
			oDesc.m_vUVIndexArray, oDesc.m_vNormalFaceArray, oDesc.m_vNormalVertexArray);
	}
	
}

CMesh::~CMesh(void)
{
	m_oRenderer.DeleteBuffer( m_pBuffer );
	m_oRenderer.DeleteBuffer( m_nVertexWeightBufferID );
	m_oRenderer.DeleteBuffer( m_nWeightedVertexIDBufferID );
	m_oRenderer.DeleteBuffer( m_nFaceMaterialBufferID );
}

void CMesh::GetOrgWorldPosition( CVector& v )
{
	v = m_oOrgMaxPosition;
}

void CMesh::DrawBoundingBox( bool bDraw )
{
	m_bDrawBoundingBox = bDraw;
}

void CMesh::DrawBoundingSphere( bool bDraw )
{

}

void CMesh::CreateNonIndexedMaterialVertexArray( const std::vector< unsigned short >& vMtlFace, const std::vector< unsigned int >& vIndexArray, std::vector<float>& vOut )
{
	for (unsigned int iFace = 0; iFace < vIndexArray.size() / 3; iFace++ )
		for (unsigned int iVertex = 0; iVertex < 3; iVertex++ )
			vOut.push_back( (float)vMtlFace[ iFace ]);
}

void CMesh::CreateNonIndexedMaterialVertexArray(const std::vector< unsigned short >& vMtlFace, const std::vector< unsigned int >& vIndexArray, std::vector<unsigned int>& vOut)
{
	for (unsigned int iFace = 0; iFace < vIndexArray.size() / 3; iFace++)
		for (unsigned int iVertex = 0; iVertex < 3; iVertex++)
			vOut.push_back(vMtlFace[iFace]);
}

void CMesh::SetDrawStyle(IRenderer::TDrawStyle style)
{
	m_eDrawStyle = style;
}

bool CMesh::IsSkinned()
{
	return m_nVertexWeightBufferID != -1;
}

IShader* CMesh::GetShader()
{
	return m_pShader;
}

void CMesh::Update()
{
	m_pShader->Enable( true );
	m_oRenderer.SetRenderType( m_eRenderType );
	
	m_pShader->SendUniformValues("nMaterialCount", (int)m_mMaterials.size());
	if(m_mMaterials.size() == 1 ) {
		map< int, CMaterial* >::iterator itMat = m_mMaterials.begin();
		itMat->second->Update();
	}
	else {
		int nMatID = m_pShader->EnableVertexAttribArray("nMatID");
		m_oRenderer.BindVertexBuffer(m_nFaceMaterialBufferID);
		m_pShader->VertexAttributePointerf(nMatID, 1, 0);
		if (!m_vTextureArray.empty()) {
			m_pShader->SendUniformVector4Array("MaterialArray", m_vMaterialArray);
			m_pShader->SendUniformVectorArray("ShininessArray", m_vShininessArray);
			for (ITexture* pTexture : m_vTextureArray)
				m_oRenderer.BindTexture(pTexture->GetID(), pTexture->GetUnitTexture(), IRenderer::T_2D);
			m_pShader->SendUniformVectorArray("baseMapArray", m_vUnitTextures);
			m_pShader->SendUniformValues("TextureCount", (int)m_vTextureArray.size());
		}
	}
		
	unsigned int nVertexWeightID = -1;
	unsigned int nWeightedVertexID = -1;
	string shaderName;
	m_pShader->GetName(shaderName);
	transform(shaderName.begin(), shaderName.end(), shaderName.begin(), tolower);
	
	if (shaderName=="skinning") {
		nVertexWeightID = m_pShader->EnableVertexAttribArray( "vVertexWeight" );
		m_oRenderer.BindVertexBuffer( m_nVertexWeightBufferID );
		m_pShader->VertexAttributePointerf( nVertexWeightID, 4, 0 );

		nWeightedVertexID = m_pShader->EnableVertexAttribArray( "vWeightedVertexID" );
		m_oRenderer.BindVertexBuffer( m_nWeightedVertexIDBufferID );
		m_pShader->VertexAttributePointerf( nWeightedVertexID, 4, 0 );
	}

	if ( m_bIndexedGeometry )
		m_oRenderer.DrawIndexedGeometry( m_pBuffer, m_eDrawStyle);
	else
		m_oRenderer.DrawGeometry( m_pBuffer );

	if (shaderName == "skinning") {
		m_pShader->DisableVertexAttribArray( nVertexWeightID );
		m_pShader->DisableVertexAttribArray( nWeightedVertexID );
	}
	if ( m_bFirstUpdate )
		m_bFirstUpdate = false;

	if( m_bDrawBoundingBox )
		CRenderUtils::DrawBox( m_pBbox->GetMinPoint(), m_pBbox->GetDimension(), m_oRenderer);
	if( m_bDrawAnimationBoundingBox && m_pCurrentAnimationBoundingBox )
		CRenderUtils::DrawBox( m_pCurrentAnimationBoundingBox->GetMinPoint(), m_pCurrentAnimationBoundingBox->GetDimension(), m_oRenderer);
	m_oRenderer.SetRenderType( IRenderer::eFill );
}

void CMesh::UpdateInstances(int instanceCount)
{
	m_pShader->Enable(true);
	m_oRenderer.SetRenderType(m_eRenderType);
	if (m_mMaterials.size() == 1)
	{
		map< int, CMaterial* >::iterator itMat = m_mMaterials.begin();
		itMat->second->Update();
	}
	else
	{
		m_pMaterialTexture->Update();
	}

	unsigned int nVertexWeightID = -1;
	unsigned int nWeightedVertexID = -1;
	string shaderName;
	m_pShader->GetName(shaderName);
	transform(shaderName.begin(), shaderName.end(), shaderName.begin(), tolower);

	if (shaderName.find("skinning") != -1)
	{
		nVertexWeightID = m_pShader->EnableVertexAttribArray("vVertexWeight");
		m_oRenderer.BindVertexBuffer(m_nVertexWeightBufferID);
		m_pShader->VertexAttributePointerf(nVertexWeightID, 4, 0);

		nWeightedVertexID = m_pShader->EnableVertexAttribArray("vWeightedVertexID");
		m_oRenderer.BindVertexBuffer(m_nWeightedVertexIDBufferID);
		m_pShader->VertexAttributePointerf(nWeightedVertexID, 4, 0);
	}
	
#if 0
	unsigned int nWorldMatrices = m_pShader->EnableVertexAttribArray("vWorldMatrices");
	if (m_nEntityMatricesBufferID == -1) {
		m_nEntityMatricesBufferID = GetRenderer().CreateBuffer((int)matrices.size());
	}
		
	const unsigned int matrixSize = 16;
	int nEntityMatrix = m_pShader->EnableVertexAttribArray("vEntityMatrix");
	GetRenderer().BindVertexBuffer(m_nEntityMatricesBufferID);
	GetRenderer().FillBuffer(matrices, m_nEntityMatricesBufferID, 0);
	m_pShader->VertexAttributePointerf(nEntityMatrix, matrixSize, 0);
	m_pShader->AttribDivisor(nEntityMatrix, matrixSize);
#endif // 0

	if (m_bIndexedGeometry) {
		m_oRenderer.DrawIndexedGeometryInstanced(m_pBuffer, m_eDrawStyle, instanceCount);
	}
	else
	{
		try {
			m_pShader->SendUniformValues("nMaterialCount", (float)m_mMaterials.size());
		}
		catch (exception&) {
			if (m_bFirstUpdate) {
				string s = string("Attribut \"nMatID\" et/ou variable uniform \"nMaterialCount\" non défini(s) dans \"") + m_sShaderName + "\"";
				MessageBox(NULL, s.c_str(), "CMesh::Update()", MB_ICONEXCLAMATION);
			}
		}
		if (m_mMaterials.size() > 1) {
			int nMatID = m_pShader->EnableVertexAttribArray("nMatID");
			m_oRenderer.BindVertexBuffer(m_nFaceMaterialBufferID);
			m_pShader->VertexAttributePointerf(nMatID, 1, 0);
		}
		m_oRenderer.DrawGeometryInstanced(m_pBuffer, instanceCount);
	}

	if (shaderName == "skinning")
	{
		m_pShader->DisableVertexAttribArray(nVertexWeightID);
		m_pShader->DisableVertexAttribArray(nWeightedVertexID);
	}
	if (m_bFirstUpdate)
		m_bFirstUpdate = false;

	if (m_bDrawBoundingBox)
		CRenderUtils::DrawBox(m_pBbox->GetMinPoint(), m_pBbox->GetDimension(), m_oRenderer);
	if (m_bDrawAnimationBoundingBox && m_pCurrentAnimationBoundingBox)
		CRenderUtils::DrawBox(m_pCurrentAnimationBoundingBox->GetMinPoint(), m_pCurrentAnimationBoundingBox->GetDimension(), m_oRenderer);
	m_oRenderer.SetRenderType(IRenderer::eFill);
}

void CMesh::DisplaySkeletonInfo( INode* pRoot, bool bRecurse )
{
	CMatrix m;
	pRoot->GetWorldMatrix( m );
	m_oRenderer.DrawBase( m, 20 );
	for ( unsigned int i = 0; i < pRoot->GetChildCount(); i++ )
		DisplaySkeletonInfo( pRoot->GetChild( i ) );
}

void CMesh::CreateMaterialTexture( const std::map< int, CMaterial* >& )
{
	throw 1;
}

bool CMesh::operator==( const IMesh& w )
{
	const CMesh* pMesh = static_cast< const CMesh* >( &w );
	return m_pBuffer == pMesh->m_pBuffer;
}

void CMesh::SetShader( IShader* pShader )
{
	m_pShader = pShader;
	if( m_mMaterials.size() == 1 )
	{
		map< int, CMaterial* >::iterator itMat = m_mMaterials.begin();
		itMat->second->SetShader( pShader );
	}
}

IBox* CMesh::GetBBox()
{
	return m_pBbox;
}

int CMesh::GetParentBoneID() const
{
	return m_nParentBoneID;
}

void CMesh::SetRenderingType( IRenderer::TRenderType t )
{
	m_eRenderType = t;
}

IBox* CMesh::GetAnimationBBox( string sAnimation )
{
	map< string, IBox* >::const_iterator itAnimation = m_mAnimationKeyBox.find( sAnimation );
	if( itAnimation != m_mAnimationKeyBox.end() )
		return itAnimation->second;
	itAnimation = m_mAnimationKeyBox.begin();
	if( itAnimation != m_mAnimationKeyBox.end() )
		return itAnimation->second;
	return m_pBbox;
}

void CMesh::DrawAnimationBoundingBox( bool bDraw )
{
	m_bDrawAnimationBoundingBox = bDraw;
}

void CMesh::SetCurrentAnimationBoundingBox( string AnimationName )
{
	map< string, IBox* >::const_iterator itBox = m_mAnimationKeyBox.find( AnimationName );
	if( itBox != m_mAnimationKeyBox.end() )
		m_pCurrentAnimationBoundingBox = itBox->second;
}

CVector& CMesh::GetOrgMaxPosition()
{
	return m_oOrgMaxPosition;
}

void CMesh::Colorize(float r, float g, float b, float a)
{
	if (m_mMaterials.size() == 1)
	{
		map< int, CMaterial* >::iterator itMat = m_mMaterials.begin();
		itMat->second->SetAdditionalColor(r, g, b, 1.f);
	}
}

ITexture* CMesh::GetTexture(int nMaterialIndex)
{
	return m_mMaterials[0]->GetTexture();
}

void CMesh::SetTexture(ITexture* pTexture)
{
	m_mMaterials[0]->SetTexture(pTexture);
}

void CMesh::SetMaterial(IRessource* pMaterial)
{
	m_mMaterials[0] = static_cast<CMaterial*>(pMaterial);
}

int CMesh::GetMaterialCount()
{
	return m_mMaterials.size();
}

IMaterial* CMesh::GetMaterial(int index)
{
	std::map< int, CMaterial* >::iterator itMat = m_mMaterials.find(index);
	if(itMat != m_mMaterials.end()) {
		return itMat->second;
	}
	return nullptr;
}