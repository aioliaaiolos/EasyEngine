#ifndef ILOADER_H
#define ILOADER_H



#include <string>
#include <vector>
#include "EEPlugin.h"
#include "Math/Quaternion.h"
#include "Math/Matrix.h"
#include "Exception.h"

class IFileSystem;
class CChunk;
class ITexture;
class IBox;
class IGeometryManager;
class IGeometry;

using namespace std;



class ILoader
{
protected:
	int		m_nAsciiExportPrecision;
public:

	enum TPixelFormat
	{
		ePixelFormatNone = -1,
		eRGB = 0,
		eRGBA,
		eBGR,
		eBGRA
	};

	struct IRessourceInfos : public IPersistantObject
	{
		virtual ~IRessourceInfos() = 0 {}
		string	m_sFileName;
		string	m_sName;
		string	m_sFileVersion;
		vector<string> m_vMessages;

		const IPersistantObject& operator >> (CBinaryFileStorage& store) const { return *this; }
		IPersistantObject& operator << (const CBinaryFileStorage& store) { return *this; }
		const IPersistantObject& operator >> (CAsciiFileStorage& store) const { return *this; }
		IPersistantObject& operator << (const CAsciiFileStorage& store) { return *this; }
		const IPersistantObject& operator >> (CStringStorage& store) const { return *this; }
		IPersistantObject& operator << (const CStringStorage& store) { return *this; }
	};

	struct CMaterialInfos : public IRessourceInfos
	{
		//std::string						m_sMaterialName;
		int								m_nID;
		std::string						m_sDiffuseMapName;
		std::vector< float >			m_vAmbient;
		std::vector< float >			m_vDiffuse;	
		std::vector< float >			m_vSpecular;
		std::vector< float >			m_vEmissive;
		float							m_fShininess;
		std::vector< CMaterialInfos >	m_vSubMaterials;
		string							m_sShaderName;
		bool							m_bExists;
		CMaterialInfos():m_nID(-1), m_fShininess( 0 ), m_bExists(false){}
		~CMaterialInfos(){}
	};

	//typedef map< int, pair< string, vector< float > > > TSkeletonMap;
	typedef map< int, pair< string, CMatrix > > TSkeletonMap;

	struct CMeshInfos : public IRessourceInfos
	{
		vector< float >									m_vVertex;
		vector< unsigned int >							m_vIndex;
		vector< float >									m_vNormalVertex;
		vector< float >									m_vNormalFace;
		vector< float >									m_vUVVertex;
		vector< unsigned int >							m_vUVIndex;
		vector< float >									m_vWeightVertex;
		vector< float >									m_vWeigtedVertexID;
		bool											m_bCanBeIndexed;
		ILoader::CMaterialInfos							m_oMaterialInfos;
		vector< unsigned short >						m_vFaceMaterialID;
		bool											m_bMultiMaterial;
		IBox*											m_pBoundingBox;
		string											m_sShaderName;
		int												m_nParentBoneID;
		//CMatrix											m_oOrgMaxMatrix;
		CVector											m_oOrgMaxPosition;
		map< string, map< int, IBox* > >				m_oKeyBoundingBoxes;
		map< string, int >								m_oPreferedKeyBBox;
		CMeshInfos() : 
		m_bCanBeIndexed( true ), m_nParentBoneID( -1 ), m_bMultiMaterial( false ), m_pBoundingBox(NULL){}
	};

	struct CAnimatableMeshData : public IRessourceInfos
	{
		map< int, IBox* >			m_mBonesBoundingBoxes;
		TSkeletonMap				m_mBones; // associe à chaque identifiant d'un bone une pair (nom , matrice )
		map< int, int >				m_mHierarchyBones;
		vector< CMeshInfos >		m_vMeshes;
		bool						m_bMultiMaterialActivated = true;
	};

	struct CAnimationBBoxInfos : public IRessourceInfos
	{
		map< string, map< int, IBox* > > mKeyBoundingBoxes;
	};

	struct CTextureInfos : public IRessourceInfos
	{
		vector< unsigned char >			m_vTexels;
		unsigned int					m_nWidth;
		unsigned int					m_nHeight;
		TPixelFormat					m_ePixelFormat;
		bool							m_bFlip;
		CTextureInfos() : m_bFlip( false ) {}
	};

	struct CAnimationInfos : public IRessourceInfos
	{
		vector< unsigned int >		m_vBonesIDArray;
		vector< unsigned int >		m_vKeyCountArray;
		vector< unsigned int >		m_vTimeValueArray;
		vector< int >				m_vKeyTypeArray;
		vector< CMatrix >			m_vLocalTMArray;
		vector< CMatrix >			m_vWorldTMArray;
		vector< float >				m_vAngleArray;
		vector< float >				m_vAxisArray;
		vector< CQuaternion >		m_vLocalQuatArray;
		int							m_nBoneCount;
		int							m_nStartTime;
		int							m_nEndTime;
	};

	struct CCollisionModelInfos : public IRessourceInfos
	{
		vector<IGeometry*> m_vPrimitives;
	};

	struct CLightInfos : public IRessourceInfos
	{
		enum TLight
		{
			eDirectionnelle,
			eOmni,
			eTarget
		};
		TLight		m_eLightType;
		CVector		m_vPosition;
		CVector		m_oColor;
		float		m_fIntensity;
	};

	struct CObjectInfos : public IPersistantObject
	{
		CObjectInfos() : m_nParentBoneID(-1){}
		CMatrix		m_oXForm;
		string		m_sRessourceFileName;
		string		m_sRessourceName;
		string		m_sObjectID;
		string		m_sParentName;
		int			m_nParentBoneID;
		virtual		~CObjectInfos() = 0 {}

		virtual const IPersistantObject& operator >> (CBinaryFileStorage& store) const override
		{
			store << m_sObjectID << m_sRessourceName << m_sRessourceFileName << m_oXForm << m_sParentName << m_nParentBoneID;
			return *this;
		}

		IPersistantObject& operator << (const CBinaryFileStorage& store) override 
		{
			store >> m_sObjectID >> m_sRessourceName >> m_sRessourceFileName >> m_oXForm;
			store >> m_sParentName >> m_nParentBoneID;
			return *this; 
		}

		const IPersistantObject& operator >> (CAsciiFileStorage& store) const override 
		{
			store << "\nObject name : " << m_sObjectID
				<< "\nRessource name : " << m_sRessourceName 
				<< "\nRessource file name : " << m_sRessourceFileName 
				<< "\nXForm : \n" << m_oXForm 
				<< "\nParent name : " << m_sParentName 
				<< "\nParent bone id : " << m_nParentBoneID;
			return *this; 
		}
		IPersistantObject& operator << (const CAsciiFileStorage& store) override { return *this; }
		const IPersistantObject& operator >> (CStringStorage& store) const override { return *this; }
		IPersistantObject& operator << (const CStringStorage& store) override { return *this; }
	};

	struct CSceneInfos : public IRessourceInfos
	{
		vector< CObjectInfos* >		m_vObject;
		string						m_sSceneFileName;
		string						m_sOriginalSceneFileName;
		CVector						m_oBackgroundColor;
		string						m_sDiffuseFileName;
		bool						m_bUseDisplacementMap;
		int							m_nMapLength;
		float						m_fMapHeight;

		const IPersistantObject& operator >> (CBinaryFileStorage& store) const override
		{
			int nObjectCount = (int)m_vObject.size();
			store << m_sSceneFileName << m_sOriginalSceneFileName << m_sName << m_oBackgroundColor << m_bUseDisplacementMap << m_sDiffuseFileName << nObjectCount << m_nMapLength << m_fMapHeight;
			for (int i = 0; i < nObjectCount; i++)
				store << *m_vObject.at(i);
			//store << m_vObject;
			return *this;
		}

		IPersistantObject& operator << (const CBinaryFileStorage& store) override
		{
			int nObjectCount = 0;
			store >> m_sSceneFileName >> m_sOriginalSceneFileName >> m_sName >> m_oBackgroundColor >> m_bUseDisplacementMap >> m_sDiffuseFileName >> nObjectCount >> m_nMapLength >> m_fMapHeight;
			for (int i = 0; i < nObjectCount; i++) {
				int type = 0;
				store >> type;
				CObjectInfos* pInfos = nullptr;
				if (type == ILoader::eEntity)
					pInfos = new CEntityInfos;
				if (type == ILoader::eAnimatedEntity)
					pInfos = new CAnimatedEntityInfos;
				else if(type == ILoader::eLight)
					pInfos = new CLightEntityInfos;
				store >> *pInfos;
				m_vObject.push_back(pInfos);
			}
			return *this; 
		}


		const IPersistantObject& operator >> (CAsciiFileStorage& store) const override
		{
			int nObjectCount = (int)m_vObject.size();
			store << string("\nScene file name : ") << m_sSceneFileName 
				<< "\nOriginal scene file name : " << m_sOriginalSceneFileName 
				<< "\nName : " << m_sName 
				<< "\nBackground color :" << m_oBackgroundColor 
				<< "\nUse displacement map : " <<m_bUseDisplacementMap 
				<< "\nDiffuse file name : " << m_sDiffuseFileName 
				<< "\nObject count : " << nObjectCount 
				<< "\nMap length : " << m_nMapLength 
				<< "\nMap height : " << m_fMapHeight;
			for (int i = 0; i < nObjectCount; i++)
				store << *m_vObject.at(i);
			return *this;
		}
	};

	struct CEntityInfos : public CObjectInfos
	{
		float								m_fWeight;
		string								m_sTypeName;
		int									m_nGrandParentDummyRootID;
		vector< CObjectInfos* >				m_vSubEntityInfos;

		CEntityInfos() : m_fWeight(0.f), m_sTypeName("Entity"), m_nGrandParentDummyRootID(-1) {}

		const IPersistantObject& operator >> (CBinaryFileStorage& store) const override
		{
			store << (int)eEntity;
			ILoader::CObjectInfos::operator >> (store);
			store << m_sTypeName;
			store << m_fWeight << m_nGrandParentDummyRootID << (int)m_vSubEntityInfos.size();
			for (int iChild = 0; iChild < m_vSubEntityInfos.size(); iChild++)
				store << *m_vSubEntityInfos[iChild];
			return *this;
		}

		const IPersistantObject& operator >> (CAsciiFileStorage& store) const override
		{
			store << "\nEntity type : " << (int)eEntity;
			ILoader::CObjectInfos::operator >> (store);
			store << "\nType name : " << m_sTypeName;
			store << "\nWeight : " << m_fWeight << "\nGrand Parent Root : " << m_nGrandParentDummyRootID << "\nSub entity count : " << (int)m_vSubEntityInfos.size();
			for (int iChild = 0; iChild < m_vSubEntityInfos.size(); iChild++)
				store << *m_vSubEntityInfos[iChild];
			return *this;
		}

		IPersistantObject& operator << (const CBinaryFileStorage& store) override
		{
			ILoader::CObjectInfos::operator << (store);
			store >> m_sTypeName;
			int subEntityCount = 0;
			store >> m_fWeight >> m_nGrandParentDummyRootID >> subEntityCount;
			for (int iChild = 0; iChild < subEntityCount; iChild++) {

				int type = 0;
				CObjectInfos* pSubEntityInfos = nullptr;
				store >> type;
				switch (type) {
				case ILoader::TObjScene::eEntity:
					pSubEntityInfos = new CEntityInfos;
					break;
				case ILoader::TObjScene::eLight:
					pSubEntityInfos = new CLightEntityInfos;
					break;
				case ILoader::TObjScene::eAnimatedEntity:
					pSubEntityInfos = new CAnimatedEntityInfos;
					break;
				}
				store >> *pSubEntityInfos;
				m_vSubEntityInfos.push_back(pSubEntityInfos);
			}
			return *this;
		}
	};

	struct CAnimatedEntityInfos : public CEntityInfos
	{
		string									m_sAnimationFileName;
		string									m_sTextureName;
		bool									m_bUseCustomSpecular;
		CVector									m_vSpecular;
		map< string, float>						m_mAnimationSpeed;
		map<string, string>						m_mClothesToNode;
		map<string, vector<int>>				m_mItems;
		string									m_sHairs;
		float									m_fStrength = 0.f;
		string									m_sClass;

		const IPersistantObject& operator >> (CBinaryFileStorage& store) const override
		{
			store << (int)eAnimatedEntity;
			ILoader::CEntityInfos::operator >> (store);
			store << m_sAnimationFileName << m_sTextureName << m_bUseCustomSpecular << m_vSpecular;
			store << m_mAnimationSpeed;
			store << m_mClothesToNode;
			store << m_mItems;
			store << m_sHairs;
			return *this;
		}

		const IPersistantObject& operator >> (CAsciiFileStorage& store) const override
		{
			store << "\nEntity type : " << (int)eAnimatedEntity;
			ILoader::CEntityInfos::operator >> (store);
			store << "\nAnimation file name : " << m_sAnimationFileName;
			store << "\nTexture file name : " << m_sTextureName;
			store << "\nUse custom specular : " << m_bUseCustomSpecular;
			store << "\nCustom specular : " << m_vSpecular;
			store << "\nAnimation speed :";
			store << m_mAnimationSpeed;
			store << "\nClothes to node : ";
			store << m_mClothesToNode;
			store << "\nItems :";
			store << m_mItems;
			store << m_sHairs;
			return *this;
		}

		IPersistantObject& operator << (const CBinaryFileStorage& store) override
		{
			int nType;
			store >> nType;
			ILoader::CEntityInfos::operator << (store);
			store >> m_sAnimationFileName >> m_sTextureName >> m_bUseCustomSpecular >> m_vSpecular;
			store >> m_mAnimationSpeed;
			store >> m_mClothesToNode;
			store >> m_mItems;
			static bool load = true;
			if(load)
				store >> m_sHairs;
			return *this;
		}
	};

	struct CLightEntityInfos : public CObjectInfos
	{
		CLightInfos::TLight		m_eType;
		CVector					m_oColor;
		float					m_fIntensity;

		const IPersistantObject& operator >> (CBinaryFileStorage& store) const override
		{
			store << (int)eLight;
			ILoader::CObjectInfos::operator >> (store);
			store << (int)m_eType << m_fIntensity << m_oColor;
			return *this;
		}

		const IPersistantObject& operator >> (CAsciiFileStorage& store) const override
		{
			ILoader::CObjectInfos::operator >> (store);
			store << "\nType : " << (int)m_eType << "\n Intensity : " << m_fIntensity << "\nColor : " << m_oColor;
			return *this;
		}

		IPersistantObject& operator << (const CBinaryFileStorage& store) override
		{
			ILoader::CObjectInfos::operator << (store);
			int type = 0;
			store >> type >> m_fIntensity >> m_oColor;
			m_eType = (CLightInfos::TLight)type;
			return *this;
		}
	};

	class CWorldInfos : public IRessourceInfos
	{
	public:
		map<string, CVector>			m_mMaps;
		map<string, CMatrix>			m_mCharacterMatrices;
		map<string, vector<CMatrix>>	m_mEntityMatrices;

		IPersistantObject& operator << (const CBinaryFileStorage& store) override
		{
			int mapCount = 0;
			store >> mapCount;
			for (int i = 0; i < mapCount; i++) {
				string mapName;
				CVector pos;
				store >> mapName >> pos;
				m_mMaps[mapName] = pos;
			}

			int characterCount = 0;
			store >> characterCount;
			for (int i = 0; i < characterCount; i++) {
				string id;
				CMatrix tm;
				store >> id >> tm;
				m_mCharacterMatrices[id] = tm;
			}
			int entityCount = 0;
			store >> entityCount;
			for (int i = 0; i < entityCount; i++) {
				string sFileName;
				int instanceCount = 0;
				store >> sFileName >> instanceCount;
				for (int iInstance = 0; iInstance < instanceCount; iInstance++) {
					CMatrix tm;
					store >> tm;
					m_mEntityMatrices[sFileName].push_back(tm);
				}
			}
			return *this;
		}
	
	};

	enum TObjScene
	{
		eEntity = 0,
		eLight,
		eAnimatedEntity,
	};

	protected:
		int					m_nFileOffset;
		int					m_nNumArray;
		static char*		s_RessourceDir;
		void				Read( void* pDestBuffer, unsigned int nElementSize, unsigned int nElementCount, FILE* pFile );
		int					ReadInt( FILE* pFile );

	public:	
		ILoader() : m_nFileOffset( 0 ), m_nAsciiExportPrecision( 3 ){}
		virtual void		Load( string sFileName, ILoader::IRessourceInfos& ri, IFileSystem& ) = 0;
		virtual void		Export( string sFileName, ILoader::IRessourceInfos& ri ) = 0;
		virtual void		SetAsciiExportPrecision( int nPrecision );
};

class ILoaderManager : public CPlugin
{
protected:
	ILoaderManager() : CPlugin( nullptr, ""){}

public:
	class CBadExtension : public CEException
	{
	public:
		CBadExtension( string& sMessage ) : CEException( sMessage ) {}
	};
	struct Desc : public CPlugin::Desc
	{
		IFileSystem&		m_oFileSystem;
		IGeometryManager&	m_oGeometryManager;
		Desc( IFileSystem& oFileSystem, IGeometryManager& oGeometryManager ) : 
			CPlugin::Desc( NULL, "" ), 
			m_oFileSystem( oFileSystem ),
			m_oGeometryManager( oGeometryManager ){}
	};

	virtual ILoader*		GetLoader( std::string sExtension ) = 0;
	virtual void			LoadTexture( string sFileName, ILoader::CTextureInfos& ti ) = 0;
	virtual void			Load( string sFileName, ILoader::IRessourceInfos& ri ) = 0;
	virtual void			Export( string sFileName, ILoader::IRessourceInfos& ri ) = 0;
	virtual void			CreateBMPFromData( const vector< unsigned char >& vData, int nWidth, int nHeight, int nBitPerPixel, string sFileName ) = 0;
};

#endif