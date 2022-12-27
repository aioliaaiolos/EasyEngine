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
IRessource::Desc( oRenderer, pShader ),
m_vVertexArray( vVertexArray ),
m_vIndexArray( vIndexArray ),
m_vUVVertexArray( vUVVertexArray ),
m_vUVIndexArray( vUVIndexArray ),
m_vNormalVertexArray( vNormalVertexArray ),
m_bIndexed( false ),
m_pDrawTool( NULL ),
m_nParentBoneID( -1 )
{
}

CMesh::CMesh( const Desc& oDesc ):
IMesh( oDesc ),
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
m_nEntityMatricesBufferID(-1)
{
	m_pShader->Enable( true );
	
	if ( m_bIndexedGeometry )
	{
		if ( oDesc.m_vVertexWeight.size() > 0 )
		{
			m_nVertexWeightBufferID = GetRenderer().CreateBuffer( (int)oDesc.m_vVertexWeight.size() );
			GetRenderer().FillBuffer( oDesc.m_vVertexWeight, m_nVertexWeightBufferID );

			m_nWeightedVertexIDBufferID = GetRenderer().CreateBuffer( (int)oDesc.m_vWeightedVertexID.size() );
			GetRenderer().FillBuffer( oDesc.m_vWeightedVertexID, m_nWeightedVertexIDBufferID );
			m_bSkinned = true;
		}
		m_pBuffer = GetRenderer().CreateIndexedGeometry( oDesc.m_vVertexArray, oDesc.m_vIndexArray, oDesc.m_vUVVertexArray, 
			oDesc.m_vUVIndexArray, oDesc.m_vNormalVertexArray );
	}
	else
	{
		if ( oDesc.m_vVertexWeight.size() > 0 )
		{
			vector< float > vVertexWeight, vWeightedVertexID;
			CRenderUtils::CreateNonIndexedVertexArray( oDesc.m_vIndexArray, oDesc.m_vVertexWeight, 4, vVertexWeight );
			CRenderUtils::CreateNonIndexedVertexArray( oDesc.m_vIndexArray, oDesc.m_vWeightedVertexID, 4, vWeightedVertexID );

			m_nVertexWeightBufferID = GetRenderer().CreateBuffer( (int)vVertexWeight.size() );
			GetRenderer().FillBuffer( vVertexWeight, m_nVertexWeightBufferID );

			m_nWeightedVertexIDBufferID = GetRenderer().CreateBuffer( (int)vWeightedVertexID.size() );
			GetRenderer().FillBuffer( vWeightedVertexID, m_nWeightedVertexIDBufferID );

			m_bSkinned = true;
		}
		if( oDesc.m_vFaceMaterialID.size() > 0 )
		{
			vector< float > vNonIndexedFaceMaterialID;
			CreateNonIndexedMaterialVertexArray( oDesc.m_vFaceMaterialID, oDesc.m_vIndexArray, vNonIndexedFaceMaterialID );
			m_nFaceMaterialBufferID = GetRenderer().CreateBuffer( (int)vNonIndexedFaceMaterialID.size() );
			GetRenderer().FillBuffer( vNonIndexedFaceMaterialID, m_nFaceMaterialBufferID );

			vector< float > vTextureMaterialData;
			CMatrix m;
			for( map< int, CMaterial* >::const_iterator itMat = oDesc.m_mMaterials.begin(); itMat != oDesc.m_mMaterials.end(); ++itMat )
			{
				itMat->second->GetMaterialMatrix( m );
				m.Get( vTextureMaterialData );
			}
			float k = 0.f;
			while ( pow(2.f, k) < vTextureMaterialData.size()) k++;
			vTextureMaterialData.resize(pow(2.f, k));
			CTexture1D::CDesc oTextureDesc( GetRenderer(), m_pShader, 0 );
			oTextureDesc.m_eFormat = IRenderer::T_RGBA;
			oTextureDesc.m_nSize = (int)vTextureMaterialData.size();
			oTextureDesc.m_pData = &vTextureMaterialData[ 0 ];
			oTextureDesc.m_sName = "Material texture";
			m_pMaterialTexture = new CTexture1D( oTextureDesc );
		}
		m_pBuffer = GetRenderer().CreateGeometry( oDesc.m_vVertexArray, oDesc.m_vIndexArray, oDesc.m_vUVVertexArray, 
													oDesc.m_vUVIndexArray, oDesc.m_vNormalFaceArray, oDesc.m_vNormalVertexArray );
	}	
}

CMesh::~CMesh(void)
{
	GetRenderer().DeleteBuffer( m_pBuffer );
	GetRenderer().DeleteBuffer( m_nVertexWeightBufferID );
	GetRenderer().DeleteBuffer( m_nWeightedVertexIDBufferID );
	GetRenderer().DeleteBuffer( m_nFaceMaterialBufferID );
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

void CMesh::CreateNonIndexedMaterialVertexArray( const std::vector< unsigned short >& vMtlFace, const std::vector< unsigned int >& vIndexArray, std::vector< float >& vOut )
{
	for (unsigned int iFace = 0; iFace < vIndexArray.size() / 3; iFace++ )
		for (unsigned int iVertex = 0; iVertex < 3; iVertex++ )
			vOut.push_back( (float)vMtlFace[ iFace ] + 1 );
}

void CMesh::SetDrawStyle(IRenderer::TDrawStyle style)
{
	m_eDrawStyle = style;
}

bool CMesh::IsSkinned()
{
	return m_nVertexWeightBufferID != -1;
}

void CMesh::Update()
{
	m_pShader->Enable( true );
	GetRenderer().SetRenderType( m_eRenderType );
	if( m_mMaterials.size() == 1 )
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
	
	if (shaderName=="skinning")
	{
		nVertexWeightID = m_pShader->EnableVertexAttribArray( "vVertexWeight" );
		GetRenderer().BindVertexBuffer( m_nVertexWeightBufferID );
		m_pShader->VertexAttributePointerf( nVertexWeightID, 4, 0 );

		nWeightedVertexID = m_pShader->EnableVertexAttribArray( "vWeightedVertexID" );
		GetRenderer().BindVertexBuffer( m_nWeightedVertexIDBufferID );
		m_pShader->VertexAttributePointerf( nWeightedVertexID, 4, 0 );
	}

	if ( m_bIndexedGeometry )
		GetRenderer().DrawIndexedGeometry( m_pBuffer, m_eDrawStyle);
	else
	{
		if ( m_mMaterials.size() > 1 )
		{
			try
			{
				int nMatID = m_pShader->EnableVertexAttribArray( "nMatID" );
				GetRenderer().BindVertexBuffer( m_nFaceMaterialBufferID );
				m_pShader->VertexAttributePointerf( nMatID, 1, 0 );
				m_pShader->SendUniformValues( "fMultimaterial", 1.f );
			}
			catch( exception& )
			{
				if( m_bFirstUpdate )
				{
					string s = string( "Attribut \"nMatID\" et/ou variable uniform \"fMultimaterial\" non défini(s) dans \"" ) + m_sShaderName + "\"";
					MessageBox( NULL, s.c_str(), "CMesh::Update()", MB_ICONEXCLAMATION);
				}
			}
		}
		else
		{
			try
			{
				m_pShader->SendUniformValues( "fMultimaterial", 0.f );
			}
			catch( exception& e )
			{
				if( m_nReponse != 6 )
				{
					string sMessage = e.what();
					sMessage += "\nVoulez vous continuer et ignorer tous les avertissements de ce type ?";
					m_nReponse = MessageBox( NULL, sMessage.c_str(), "", MB_YESNO );
				}
			}
		}
		GetRenderer().DrawGeometry( m_pBuffer );
	}

	if (shaderName == "skinning")
	{
		m_pShader->DisableVertexAttribArray( nVertexWeightID );
		m_pShader->DisableVertexAttribArray( nWeightedVertexID );
	}
	if ( m_bFirstUpdate )
		m_bFirstUpdate = false;

	if( m_bDrawBoundingBox )
		CRenderUtils::DrawBox( m_pBbox->GetMinPoint(), m_pBbox->GetDimension(), GetRenderer() );	
	if( m_bDrawAnimationBoundingBox && m_pCurrentAnimationBoundingBox )
		CRenderUtils::DrawBox( m_pCurrentAnimationBoundingBox->GetMinPoint(), m_pCurrentAnimationBoundingBox->GetDimension(), GetRenderer() );
	GetRenderer().SetRenderType( IRenderer::eFill );
}

void CMesh::UpdateInstances(int instanceCount)
{
	m_pShader->Enable(true);
	GetRenderer().SetRenderType(m_eRenderType);
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
		GetRenderer().BindVertexBuffer(m_nVertexWeightBufferID);
		m_pShader->VertexAttributePointerf(nVertexWeightID, 4, 0);

		nWeightedVertexID = m_pShader->EnableVertexAttribArray("vWeightedVertexID");
		GetRenderer().BindVertexBuffer(m_nWeightedVertexIDBufferID);
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
		GetRenderer().DrawIndexedGeometryInstanced(m_pBuffer, m_eDrawStyle, instanceCount);
	}
	else
	{
		if (m_mMaterials.size() > 1)
		{
			try
			{
				int nMatID = m_pShader->EnableVertexAttribArray("nMatID");
				GetRenderer().BindVertexBuffer(m_nFaceMaterialBufferID);
				m_pShader->VertexAttributePointerf(nMatID, 1, 0);
				m_pShader->SendUniformValues("fMultimaterial", 1.f);
			}
			catch (exception&)
			{
				if (m_bFirstUpdate)
				{
					string s = string("Attribut \"nMatID\" et/ou variable uniform \"fMultimaterial\" non défini(s) dans \"") + m_sShaderName + "\"";
					MessageBox(NULL, s.c_str(), "CMesh::Update()", MB_ICONEXCLAMATION);
				}
			}
		}
		else
		{
			try
			{
				m_pShader->SendUniformValues("fMultimaterial", 0.f);
			}
			catch (exception& e)
			{
				if (m_nReponse != 6)
				{
					string sMessage = e.what();
					sMessage += "\nVoulez vous continuer et ignorer tous les avertissements de ce type ?";
					m_nReponse = MessageBox(NULL, sMessage.c_str(), "", MB_YESNO);
				}
			}
		}
		GetRenderer().DrawGeometryInstanced(m_pBuffer, instanceCount);
	}

	if (shaderName == "skinning")
	{
		m_pShader->DisableVertexAttribArray(nVertexWeightID);
		m_pShader->DisableVertexAttribArray(nWeightedVertexID);
	}
	if (m_bFirstUpdate)
		m_bFirstUpdate = false;

	if (m_bDrawBoundingBox)
		CRenderUtils::DrawBox(m_pBbox->GetMinPoint(), m_pBbox->GetDimension(), GetRenderer());
	if (m_bDrawAnimationBoundingBox && m_pCurrentAnimationBoundingBox)
		CRenderUtils::DrawBox(m_pCurrentAnimationBoundingBox->GetMinPoint(), m_pCurrentAnimationBoundingBox->GetDimension(), GetRenderer());
	GetRenderer().SetRenderType(IRenderer::eFill);
}

void CMesh::DisplaySkeletonInfo( INode* pRoot, bool bRecurse )
{
	CMatrix m;
	pRoot->GetWorldMatrix( m );
	GetRenderer().DrawBase( m, 20 );
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