//#ifndef MESH_CPP
//#ifndef RESSOURCEMANAGER_CPP
//#error
//#endif
//#endif

#ifndef MESH_H
#define MESH_H

// stl
#include <vector>
#include <map>

// Utils
#include "Math/Matrix.h"

// Engine
#include "IRenderer.h"
#include "IRessource.h"

class CTextureBase;
class CMaterial;
class CShader;
class CNode;
class IDrawTool;
class CRessourceManager;
class IShader;
class INode;

class CMesh : public IMesh
{
public:
	struct Desc : IRessource::Desc
	{
		std::vector< float >&			m_vVertexArray;
		std::vector< unsigned int >&	m_vIndexArray;
		std::vector< float >&			m_vUVVertexArray;
		std::vector< unsigned int >&	m_vUVIndexArray;
		std::vector< float >			m_vNormalFaceArray;
		std::vector< float >&			m_vNormalVertexArray;
		std::vector< float >			m_vVertexWeight;
		std::vector< float >			m_vWeightedVertexID;
		std::map< int, CMaterial* >		m_mMaterials;
		std::vector< unsigned short >	m_vFaceMaterialID;
		bool							m_bIndexed;
		IDrawTool*						m_pDrawTool;
		IBox*							m_pBbox;
		int								m_nParentBoneID;
		CVector							m_oOrgMaxPosition;
		map< string, IBox* >			m_mAnimationKeyBox;
		IShader*						m_pShader;
		IRenderer&						m_oRenderer;

		Desc(  std::vector< float >& vVertexArray, std::vector<  unsigned int  >& vIndexArray,
				std::vector< float >& vUVVertexArray, std::vector< unsigned int >& vUVIndexArray,
				std::vector< float >& vNormalVertexArray, IRenderer& oRenderer, IShader* pShader );
	};
	CMesh(const Desc& oDesc);
	virtual								~CMesh();
	void								Update() override;
	void								UpdateInstances(int instanceCount) override;
	bool								operator==(const IMesh& w);
	void								DrawBoundingBox(bool bDraw);
	void								DrawBoundingSphere(bool bDraw);
	void								SetCurrentAnimationBoundingBox(string AnimationName);
	void								DrawAnimationBoundingBox(bool bDraw);
	void								SetShader(IShader* pShader);
	IBox*								GetBBox();
	IShader*							GetShader() const { return m_pShader; }
	int									GetParentBoneID()const;
	void								GetOrgWorldPosition(CVector& v);
	void								SetRenderingType(IRenderer::TRenderType t);
	IBox*								GetAnimationBBox(string sAnimation);
	CVector&							GetOrgMaxPosition();
	void								Colorize(float r, float g, float b, float a);
	ITexture*							GetTexture(int nMaterialIndex);
	void								SetTexture(ITexture* pTexture);
	void								SetMaterial(IRessource* pMaterial) override;
	int									GetMaterialCount() override;
	IMaterial*							GetMaterial(int index) override;
	void								SetDrawStyle(IRenderer::TDrawStyle style) override;
	bool								IsSkinned();
	IShader*							GetShader();


private:
	int									m_nReponse;
	IBuffer*							m_pBuffer;
	std::map< int, CMaterial* >			m_mMaterials;
	std::vector< float >				m_vMaterialTextureData;
	std::string							m_sShaderName;
	std::vector< int >					m_vWeightedVertexID;
	std::vector< float >				m_vVertexWeight;
	unsigned int						m_nVertexWeightBufferID;
	unsigned int						m_nWeightedVertexIDBufferID;
	bool								m_bIndexedGeometry;
	bool								m_bSkinned;
	int									m_nFaceMaterialBufferID;
	ITexture*							m_pMaterialTexture;
	bool								m_bFirstUpdate;
	IBox*								m_pBbox;
	bool								m_bDrawBoundingBox;
	bool								m_bDrawAnimationBoundingBox;
	IBox*								m_pCurrentAnimationBoundingBox;
	IShader*							m_pShader;
	int									m_nParentBoneID;
	CVector								m_oOrgMaxPosition;
	IRenderer::TRenderType				m_eRenderType;
	map< string, IBox* >				m_mAnimationKeyBox;
	IRenderer::TDrawStyle				m_eDrawStyle;
	int									m_nEntityMatricesBufferID;
	IRenderer&							m_oRenderer;
	vector< float >						m_vMaterialArray;
	vector<float>						m_vShininessArray;
	vector<ITexture*>					m_vTextureArray;
	vector<int>							m_vUnitTextures;

	void								CreateMaterialTexture(const std::map< int, CMaterial* >&);
	void								DisplaySkeletonInfo(INode* pRoot, bool bRecurse = true);
	void								CreateNonIndexedMaterialVertexArray(const std::vector< unsigned short >& vMtlFace, const std::vector< unsigned int >& vIndexArray, std::vector<float>& vOut);
	void								CreateNonIndexedMaterialVertexArray(const std::vector< unsigned short >& vMtlFace, const std::vector< unsigned int >& vIndexArray, std::vector<unsigned int>& vOut);

};

#endif