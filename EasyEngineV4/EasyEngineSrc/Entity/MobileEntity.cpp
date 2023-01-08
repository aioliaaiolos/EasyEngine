#include "MobileEntity.h"
#include "Interface.h"
#include "IFileSystem.h"
#include <algorithm>
#include "ICollisionManager.h"
#include "IGeometry.h"
#include "Utils2/TimeManager.h"
#include "Scene.h"
#include "EntityManager.h"
#include "IGUIManager.h"
#include "Bone.h"
#include "../Utils2/StringUtils.h"

map< string, IEntity::TAnimation >			CMobileEntity::s_mAnimationStringToType;
map< IEntity::TAnimation, float > 			CMobileEntity::s_mOrgAnimationSpeedByType;
map< string, CMobileEntity::TAction >				CMobileEntity::s_mActions;
vector< CMobileEntity* >							CMobileEntity::s_vHumans;

map<string, IEntity::TAnimation> CMobileEntity::s_mStringToAnimation;



CObject::CObject(EEInterface& oInterface, string sFileName) :
	CEntity(oInterface, sFileName)
{
	m_pfnCollisionCallback = OnCollision;
}

void CObject::Update()
{
	ManageGravity();
	UpdateCollision();

	if (m_pCurrentAnimation) {
		m_pCurrentAnimation->Update();
		m_pBoundingGeometry = GetBoundingGeometry();
	}

	UpdateWorldMatrix();
	UpdateChildren();
	SendBonesToShader();

	m_oWorldMatrix *= m_oScaleMatrix;
	m_oRenderer.SetModelMatrix(m_oWorldMatrix);
	UpdateRessource();

	if (m_bDrawBoundingBox && m_pBoundingGeometry)
		UpdateBoundingBox();

	m_vNextLocalTranslate.Fill(0, 0, 0, 1);
	DispatchEntityEvent();
	CEntity::ExecuteScripts();
}

void CObject::OnCollision(CEntity* pThis, vector<INode*> entities)
{
	for (int i = 0; i < entities.size(); i++) {
		CEntity* pEntity = dynamic_cast<CEntity*>(entities[i]);
		IMesh* pMesh = static_cast<IMesh*>(pThis->GetRessource());
		ICollisionMesh* pCollisionMesh = pEntity ? pEntity->GetCollisionMesh() : NULL;
		if (pCollisionMesh)
			pThis->LinkAndUpdateMatrices(pEntity);
	}
}

void CObject::ManageGravity()
{
	m_oBody.Update();
	if (m_oBody.m_fWeight > 0.f)
	{
		float x, y, z;
		m_oLocalMatrix.GetPosition(x, y, z);
		int nDelta = CTimeManager::Instance()->GetTimeElapsedSinceLastUpdate();
		m_vNextLocalTranslate.m_y += m_oBody.m_oSpeed.m_y * (float)nDelta / 1000.f;
	}
}

void CObject::UpdateCollision()
{
	if (GetWeight() > 0) {
		float h = GetHeight();

		m_vNextLocalTranslate += m_vConstantLocalTranslate;

		CMatrix backupLocal = m_oLocalMatrix;

		CNode::LocalTranslate(m_vNextLocalTranslate.m_x, m_vNextLocalTranslate.m_y, m_vNextLocalTranslate.m_z);
		CNode::UpdateWorldMatrix();
		CMatrix oLocalMatrix = m_oLocalMatrix;

		vector<INode*> entities;
		if (!m_bFirstUpdate)
			GetEntitiesCollision(entities);
		else
			m_bFirstUpdate = false;

		CVector localPos;
		oLocalMatrix.GetPosition(localPos);

		CVector directriceLine = m_vNextLocalTranslate;
		CVector first = backupLocal.GetPosition();
		CVector last = localPos;
		CVector firstBottom = first;
		CVector lastBottom = last;
		CVector R = last;
		bool bCollision = false;
		float fMaxHeight = -999999.f;
		for (int i = 0; i < entities.size(); i++) {
			INode* pEntity = entities[i];
			pEntity->GetBoundingGeometry()->SetTM(pEntity->GetLocalMatrix());
			IGeometry* firstBox = GetBoundingGeometry()->Duplicate();
			firstBox->SetTM(backupLocal);
			IGeometry* lastBox = GetBoundingGeometry();
			lastBox->SetTM(oLocalMatrix);
			IGeometry::TFace collisionFace = IGeometry::eNone;
			collisionFace = pEntity->GetBoundingGeometry()->GetReactionYAlignedBox(*firstBox, *lastBox, R);
			delete firstBox;
			if (collisionFace != IBox::eNone) {
				lastBottom = R;
				last = R;
				bCollision = true;
				if (collisionFace == IBox::eYPositive) {
					last.m_y += h / 2.f;
					if (fMaxHeight < last.m_y)
						fMaxHeight = last.m_y;
					else
						last.m_y = fMaxHeight;
					m_oBody.m_oSpeed.m_y = 0;
				}
			}
			else {
				bCollision = true;
			}
		}
		// Ground collision
		const float margin = 10.f;
		CVector nextPosition = m_oLocalMatrix * CVector(0, 0, 100);
		float fGroundHeight = m_pParent->GetGroundHeight(localPos.m_x, localPos.m_z) + margin;
		float fGroundHeightNext = m_pParent->GetGroundHeight(nextPosition.m_x, nextPosition.m_z) + margin;
		float fEntityY = last.m_y - h / 2.f;
		if (fEntityY <= fGroundHeight + CBody::GetEpsilonError()) {
			m_oBody.m_oSpeed.m_x = 0;
			m_oBody.m_oSpeed.m_y = 0;
			m_oBody.m_oSpeed.m_z = 0;
			float newY = fGroundHeight + h / 2.f;
			last.m_y = newY;
		}
		SetLocalPosition(last);

		// Still into parent ?	
		if (!TestWorldCollision(m_pParent)) {
			CEntity* pEntity = dynamic_cast<CEntity*>(m_pParent->GetParent());
			if (pEntity)
				LinkAndUpdateMatrices(pEntity);
		}

		if (bCollision && m_pfnCollisionCallback) {
			m_pfnCollisionCallback(this, entities);
		}
	}
}


CMobileEntity::CMobileEntity(EEInterface& oInterface, string sFileName, string sID):
CObject(oInterface, sFileName),
m_bInitSkeletonOffset( false ),
m_fMaxEyeRotationH( 15 ),
m_fMaxEyeRotationV( 15 ),
m_fMaxNeckRotationH( 45 ),
m_fMaxNeckRotationV( 15 ),
m_fEyesRotH( 0 ),
m_fEyesRotV( 0 ),
m_fNeckRotH( 0 ),
m_fNeckRotV( 0 ),
m_bFirstUpdate(true),
m_sStandAnimation("stand-normal")
{
	m_sEntityName = sID;
	m_sTypeName = "Human";
	
	for( int i = 0; i < eAnimationCount; i++ )
		m_mAnimationSpeedByType[ (TAnimation)i ] = s_mOrgAnimationSpeedByType[ (TAnimation)i ];

	m_oBody.m_fWeight = 1.f;
	
	InitAnimations();

	s_vHumans.push_back( this );
	m_pLeftEye = dynamic_cast< IEntity* >( m_pSkeletonRoot->GetChildBoneByName( "OeilG" ) );
	m_pRightEye = dynamic_cast< IEntity* >( m_pSkeletonRoot->GetChildBoneByName( "OeilD" ) );
	m_pNeck = m_pSkeletonRoot->GetChildBoneByName( "Cou" );

	m_pfnCollisionCallback = OnCollision;
	m_sSecondaryAttackBoneName = "OrteilsG";
	m_sAttackBoneName = "MainD";
	IEntityManager* pEntityManager = static_cast<IEntityManager*>(oInterface.GetPlugin("EntityManager"));
	pEntityManager->AddNewCharacter(this);

	map< string, map< int, IBox* > >::iterator itBox = m_oKeyBoundingBoxes.find("stand-normal");
	if (itBox != m_oKeyBoundingBoxes.end()) {
		m_pBoundingGeometry = itBox->second.find(160)->second;
	}

	m_pBBox = dynamic_cast<IBox*>(m_pBoundingGeometry);
}

CMobileEntity::~CMobileEntity()
{

}

void CMobileEntity::InitAnimations()
{
	string sMask = "Animations/*.bke";
	WIN32_FIND_DATAA oData;
	IFileSystem* pFileSystem = static_cast<IFileSystem*>(m_oInterface.GetPlugin("FileSystem"));
	HANDLE hFirst = pFileSystem->FindFirstFile_EE(sMask, oData);
	if (hFirst != INVALID_HANDLE_VALUE)
	{
		do
		{
			string sFileNameFound = oData.cFileName;
			string sFileNameLow = oData.cFileName;
			transform(sFileNameFound.begin(), sFileNameFound.end(), sFileNameLow.begin(), tolower);
			AddAnimation(sFileNameLow);
			string sAnimationType = sFileNameLow.substr(0, sFileNameLow.size() - 4);
			m_mAnimations[s_mAnimationStringToType[sAnimationType]] = m_mAnimation[sFileNameLow];
		} while (FindNextFileA(hFirst, &oData));
	}
}

void CMobileEntity::InitStatics()
{
	s_mAnimationStringToType["walk"] = eWalk;
	s_mAnimationStringToType["run"] = eRun;
	s_mAnimationStringToType["stand"] = eStand;
	s_mAnimationStringToType["HitLeftFoot"] = eHitLeftFoot;
	s_mAnimationStringToType["HitRightArm"] = eHitRightArm;
	s_mAnimationStringToType["jump"] = eJump;
	s_mAnimationStringToType["dying"] = eDying;
	s_mAnimationStringToType["MoveToGuard"] = eMoveToGuard;
	s_mOrgAnimationSpeedByType[eWalk] = -1.6f;
	s_mOrgAnimationSpeedByType[eStand] = 0.f;
	s_mOrgAnimationSpeedByType[eRun] = -7.f;
	s_mOrgAnimationSpeedByType[eHitLeftFoot] = 0.f;
	s_mOrgAnimationSpeedByType[eHitReceived] = 0.f;
	s_mOrgAnimationSpeedByType[eDying] = 0.f;
	s_mOrgAnimationSpeedByType[eMoveToGuard] = 0.f;

	s_mActions["walk"] = Walk;
	s_mActions["run"] = Run;
	s_mActions["stand"] = Stand;
	s_mActions["PlayReceiveHit"] = PlayReceiveHit;
	s_mActions["jump"] = Jump;
	s_mActions["dying"] = Dying;
	s_mActions["MoveToGuard"] = MoveToGuard;

	s_mStringToAnimation["run"] = IEntity::eRun;
}


void CMobileEntity::OnCollision(CEntity* pThis, vector<INode*> entities)
{
	for (int i = 0; i < entities.size(); i++) {
		CEntity* pEntity = dynamic_cast<CEntity*>(entities[i]);
		IMesh* pMesh = static_cast<IMesh*>(pThis->GetRessource());
		ICollisionMesh* pCollisionMesh = pEntity ? pEntity->GetCollisionMesh() : NULL;
		if (pCollisionMesh)
			pThis->LinkAndUpdateMatrices(pEntity);
	}
}

IGeometry* CMobileEntity::GetBoundingGeometry()
{
	return m_pBoundingGeometry;
	if (m_pCurrentAnimation) {
		string sAnimationName;
		m_pCurrentAnimation->GetName(sAnimationName);
		sAnimationName = sAnimationName.substr(sAnimationName.find_last_of("/") + 1);
		int time = m_pCurrentAnimation->GetAnimationTime();
		//int key = (time / 160) * 160;
		int key = 160;
		map<int, IBox*>::iterator itBox = m_oKeyBoundingBoxes[sAnimationName].find(key);
		if (itBox == m_oKeyBoundingBoxes[sAnimationName].end()) {
			map<int, IBox*>::iterator itLast = m_oKeyBoundingBoxes[sAnimationName].begin(), itNext = itBox;
			for (itBox = m_oKeyBoundingBoxes[sAnimationName].begin(); itBox != m_oKeyBoundingBoxes[sAnimationName].end(); itBox++) {
				if (itNext == m_oKeyBoundingBoxes[sAnimationName].end())
					itNext = itBox;
				else {
					std::advance(itNext, 1);
					if (itNext == m_oKeyBoundingBoxes[sAnimationName].end())
						itNext = itBox;
				}
				if (itBox->first >= itLast->first && itBox->first <= itNext->first) {
					itBox = itNext;
					break;
				}
				itLast = itBox;
			}
		}
		if(sAnimationName == "run")
			itBox = m_oKeyBoundingBoxes[sAnimationName].find(160);
		if(itBox != m_oKeyBoundingBoxes[sAnimationName].end())
			return (IBox*)itBox->second;
	}
	else {
		return m_pMesh->GetBBox();
	}
	static bool bAlreadyThrown = false;
	if (!bAlreadyThrown) {
		bAlreadyThrown = true;
		throw CEException("Warning : you don't associated current animation to bounding boxes for character '" + m_sEntityName + "', the default character box will be used");
	}
	return m_pMesh->GetBBox();
}

const string& CMobileEntity::GetAttackBoneName()
{
	return m_sAttackBoneName;
}

const string& CMobileEntity::GetSecondaryAttackBoneName()
{
	return m_sSecondaryAttackBoneName;
}

void CMobileEntity::WearArmorToDummy(string armorName)
{
	string path = "Meshes/Armors/" + armorName + "/";
	const int count = 8;
	string arrayPiece[count] = { "Cuirasse", "Jupe", "JambiereD", "JambiereG", "BrassiereD", "BrassiereG", "EpauletteD", "EpauletteG" };
	vector<string> armorDummies, bodyDummies;
	for(int i = 0; i < count; i++) {
		string armorDummy = string("Dummy") + arrayPiece[i];
		armorDummies.push_back(armorDummy);
		string bodyDummy = string("BodyDummy") + arrayPiece[i];
		bodyDummies.push_back(bodyDummy);
	}

	for (int i = 0; i < count; i++) {
		IBone* pBodyDummy = dynamic_cast<IBone*>(m_pSkeletonRoot->GetChildBoneByName(bodyDummies[i]));
		if (pBodyDummy) {
			CEntity* piece = new CEntity(m_oInterface, string("meshes/armors/") + armorName + "/" + arrayPiece[i] + ".bme");
			piece->LinkDummyParentToDummyEntity(this, bodyDummies[i]);
		}
	}
}

void CMobileEntity::UnWearShoes(string shoesPath)
{
	IBone* pDummyLShoes = m_pSkeletonRoot->GetChildBoneByName("BodyDummyLFoot");
	IBone* pDummyRShoes = m_pSkeletonRoot->GetChildBoneByName("BodyDummyRFoot");
	if (pDummyLShoes) {
		INode* pShoeNode = pDummyLShoes->GetChild(0);
		pShoeNode->Unlink();
	}
	if (pDummyRShoes) {
		INode* pShoeNode = pDummyRShoes->GetChild(0);
		pShoeNode->Unlink();
	}
}

void CMobileEntity::UnWearAllShoes()
{
	IBone* pBodyDummyLShoes = m_pSkeletonRoot->GetChildBoneByName("BodyDummyLFoot");
	pBodyDummyLShoes->ClearChildren();
	IBone* pBodyDummyRShoes = m_pSkeletonRoot->GetChildBoneByName("BodyDummyRFoot");
	pBodyDummyRShoes->ClearChildren();
}

void CMobileEntity::UnwearAllClothes()
{
	for (INode*& pNode : m_vClothes) {
		pNode->Unlink();
	}
}

void CMobileEntity::WearShoes(string shoesPath)
{
	string shoesPathLower = shoesPath;
	std::transform(shoesPath.begin(), shoesPath.end(), shoesPathLower.begin(), tolower);
	string sPrefix;
	int prefixIndex = shoesPathLower.find("hight");
	if (prefixIndex != -1) {
		sPrefix = shoesPath.substr(0, shoesPath.find_last_of("/") + 1);
		m_sStandAnimation = "stand";
	}
	else
		m_sStandAnimation = "stand-normal";
	Stand();

	// unlink old shoes if exists
	IBone* pDummyLShoes = m_pSkeletonRoot->GetChildBoneByName("BodyDummyLFoot");
	IBone* pDummyRShoes = m_pSkeletonRoot->GetChildBoneByName("BodyDummyRFoot");
	if (pDummyLShoes)
		pDummyLShoes->ClearChildren();
	if (pDummyRShoes)
		pDummyRShoes->ClearChildren();

	// link new shoes
	string shoesName = shoesPath.substr(shoesPath.find_last_of("/") + 1);
	CEntity* Lshoes = new CEntity(m_oInterface, string("clothes/shoes/") + sPrefix + "L" + shoesName + ".bme");
	Lshoes->LinkDummyParentToDummyEntity(this, "BodyDummyLFoot");
	CEntity* Rshoes = new CEntity(m_oInterface, string("clothes/shoes/") + sPrefix + "R" + shoesName + ".bme");
	Rshoes->LinkDummyParentToDummyEntity(this, "BodyDummyRFoot");
}

void CMobileEntity::WearCloth(string sClothPath, string sDummyName)
{
	string clothPathLower = sClothPath;
	std::transform(sClothPath.begin(), sClothPath.end(), clothPathLower.begin(), tolower);

	string sClothName = sClothPath.substr(sClothPath.find_last_of("/") + 1);
	CEntity* pCloth = dynamic_cast<CEntity*>(m_pEntityManager->CreateEntity(string("Clothes/") + sClothName + ".bme", ""));
	if (pCloth) {
		IMesh* pMesh = pCloth->GetMesh();
		if (pMesh && pMesh->IsSkinned()) {
			pCloth->SetSkeletonRoot(m_pSkeletonRoot, m_pOrgSkeletonRoot);
			pCloth->Link(this);
			pCloth->SetSkinOffset(pCloth->GetMesh()->GetOrgMaxPosition()); 
			m_vClothes.push_back(pCloth);
		}
		else {
			pCloth->LinkDummyParentToDummyEntity(this, sDummyName);
			m_vClothes.push_back(pCloth->GetSkeletonRoot());
		}
		
	}
}

void CMobileEntity::Link(INode* pParent)
{
	CEntity::Link(pParent);
	CEntity* pParentEntity = dynamic_cast<CEntity*>(m_pParent);
	m_pCollisionMap = pParentEntity->GetCollisionMap();
}

IBox* CMobileEntity::GetBoundingBox()
{
	return m_pBBox;
}

void CMobileEntity::AddHairs(string hairsName)
{
	string shoesPathLower = hairsName;
	std::transform(hairsName.begin(), hairsName.end(), shoesPathLower.begin(), tolower);

	// unlink old hairs if exists
	IBone* pDummyHairs = m_pSkeletonRoot->GetChildBoneByName("DummyHairs");
	
	if (pDummyHairs)
		pDummyHairs->Unlink();

	// link new hairs
	CEntity* hairs = new CEntity(m_oInterface, string("Meshes/hairs/") + hairsName + ".bme");
	hairs->LinkDummyParentToDummyEntity(this, "BodyDummyHairs");	
}

void CMobileEntity::SetBody(string sBodyName)
{
	SetRessource(string("Meshes/Bodies/") + sBodyName);
	InitAnimations();
}

void CMobileEntity::RunAction( string sAction, bool bLoop )
{
	map<string, CMobileEntity::TAction>::iterator itAction = s_mActions.find(sAction);
	if (itAction != s_mActions.end())
		itAction->second(this, bLoop);
}

void CMobileEntity::SetPredefinedAnimation( string s, bool bLoop )
{
	IMesh* pMesh = static_cast< IMesh* >( m_pRessource );
	string sAnimationName = s + ".bke";
	string sAnimationNameLow = sAnimationName;
	transform( sAnimationName.begin(), sAnimationName.end(), sAnimationNameLow.begin(), tolower );
	SetCurrentAnimation( sAnimationNameLow );
	if( !m_pCurrentAnimation )
	{
		string sMessage = string( "Erreur : fichier \"" ) + sAnimationNameLow + "\" manquant";
		CFileNotFoundException e( sMessage );
		e.m_sFileName = sAnimationNameLow;
		throw e;
	}
	m_pCurrentAnimation->Play( bLoop );
	if (m_pCloth) {
		m_pCloth->SetCurrentAnimation(sAnimationNameLow);
		m_pCloth->PlayCurrentAnimation(bLoop);
	}
	m_eCurrentAnimationType = s_mAnimationStringToType[ s ];
}

void CMobileEntity::Walk( bool bLoop )
{
	if (m_eCurrentAnimationType != eWalk)
	{
		SetPredefinedAnimation("walk", bLoop);
		if (!m_bUsePositionKeys)
			ConstantLocalTranslate(CVector(0.f, 0.f, -m_mAnimationSpeedByType[eWalk]));
	}
}

void CMobileEntity::Stand( bool bLoop )
{
	SetPredefinedAnimation( m_sStandAnimation, bLoop );
	if( !m_bUsePositionKeys )
		ConstantLocalTranslate( CVector( 0.f, m_mAnimationSpeedByType[ eStand ], 0.f ) );
}

void CMobileEntity::Run( bool bLoop )
{
	if( m_eCurrentAnimationType != eRun )
	{
		SetPredefinedAnimation( "run", bLoop );
		if( !m_bUsePositionKeys )
			ConstantLocalTranslate( CVector( 0.f, 0.f, -m_mAnimationSpeedByType[eRun]) );
	}
}

void CMobileEntity::Jump(bool bLoop)
{
	if (m_eCurrentAnimationType != eJump)
	{
		//SetPredefinedAnimation("jump", bLoop);
		//if (!m_bUsePositionKeys)
		//ConstantLocalTranslate(CVector(0.f, m_mAnimationSpeedByType[eJump], 0.f));
		m_oBody.m_oSpeed.m_y = 2000;
	}
}

void CMobileEntity::Die()
{
	if (m_eCurrentAnimationType != eDying) {
		SetPredefinedAnimation("dying", false);
		m_pCurrentAnimation->AddCallback(OnDyingCallback, this);
	}
}

void CMobileEntity::Yaw(float fAngle)
{
	if(GetLife() > 0)
		CNode::Yaw(fAngle);
}

void CMobileEntity::Pitch(float fAngle)
{
	if (GetLife() > 0)
		CNode::Pitch(fAngle);
}

void CMobileEntity::Roll(float fAngle)
{
	if (GetLife() > 0)
		CNode::Roll(fAngle);
}

void CMobileEntity::OnDyingCallback(IAnimation::TEvent e, void* pEntity)
{
	CMobileEntity* pMobileEntity = (CMobileEntity*)pEntity;
	pMobileEntity->m_eCurrentAnimationType = eNone;
}

void CMobileEntity::PlayHitAnimation()
{
	SetPredefinedAnimation("HitRightArm", false);
}

void CMobileEntity::PlaySecondaryHitAnimation()
{
	SetPredefinedAnimation("HitLeftFoot", false);
}

void CMobileEntity::PlayReceiveHit( bool bLoop )
{
	SetPredefinedAnimation( "HitReceived", bLoop );
	if( !m_bUsePositionKeys )
		ConstantLocalTranslate( CVector( 0.f, m_mAnimationSpeedByType[ eHitReceived ], 0.f ) );
}

void CMobileEntity::PlayReceiveHit( CMobileEntity* pEntity, bool bLoop )
{
	pEntity->PlayReceiveHit( bLoop );
}

void CMobileEntity::PlayReceiveHit()
{
	RunAction("PlayReceiveHit", false);
}

void CMobileEntity::MoveToGuard()
{
	//RunAction("guard", false);
	if (m_eCurrentAnimationType != eMoveToGuard) {
		SetPredefinedAnimation("MoveToGuard", false);
	}
}

void CMobileEntity::Guard()
{
	//RunAction("guard", false);
	if (m_eCurrentAnimationType != eMoveToGuard) {
		SetPredefinedAnimation("Guard", false);
	}
}

void CMobileEntity::OnWalkAnimationCallback( IAnimation::TEvent e, void* pData )
{
	if( e == IAnimation::eBeginRewind )
	{
		CMobileEntity* pHuman = reinterpret_cast< CMobileEntity* >( pData );
		pHuman->LocalTranslate( 0, -pHuman->m_oSkeletonOffset.m_23, 0 );
	}
}

void CMobileEntity::Walk( CMobileEntity* pHuman, bool bLoop  )
{
	pHuman->Walk( bLoop );
}

void CMobileEntity::Stand( CMobileEntity* pHuman, bool bLoop  )
{
	pHuman->Stand( bLoop );
}

void CMobileEntity::Run( CMobileEntity* pHuman, bool bLoop  )
{
	pHuman->Run( bLoop );
}

void CMobileEntity::Jump(CMobileEntity* pHuman, bool bLoop)
{
	pHuman->Jump(bLoop);
}

void CMobileEntity::Dying(CMobileEntity* pHuman, bool bLoop)
{
	pHuman->Die();
}

void CMobileEntity::MoveToGuard(CMobileEntity* pHuman, bool bLoop)
{
	pHuman->MoveToGuard();
}

void CMobileEntity::SetAnimationSpeed( IEntity::TAnimation eAnimationType, float fSpeed )
{
	m_mAnimations[ eAnimationType ]->SetSpeed(fSpeed);
	m_mAnimationSpeedByType[ eAnimationType ] = s_mOrgAnimationSpeedByType[ eAnimationType ] * fSpeed;
}

float CMobileEntity::GetAnimationSpeed(IEntity::TAnimation eAnimationType)
{
	return m_mAnimations[eAnimationType]->GetSpeed();
}

void CMobileEntity::GetEntityInfos(ILoader::CObjectInfos*& pInfos)
{
	if(!pInfos)
		pInfos = new ILoader::CAnimatedEntityInfos;
	CEntity::GetEntityInfos(pInfos);
	ILoader::CAnimatedEntityInfos& animatedEntityInfos = static_cast<ILoader::CAnimatedEntityInfos&>(*pInfos);
	IAnimation* pAnimation = GetCurrentAnimation();
	if (pAnimation) {
		string animFile;
		pAnimation->GetFileName(animFile);
		animFile = animFile.substr(animFile.find_last_of("/") + 1);
		animatedEntityInfos.m_sAnimationFileName = animFile;
		for (map<string, IEntity::TAnimation>::iterator it = CMobileEntity::s_mStringToAnimation.begin(); it != CMobileEntity::s_mStringToAnimation.end(); it++) {
			float as = GetAnimationSpeed(it->second);
			animatedEntityInfos.m_mAnimationSpeed[it->first] = as;
		}
	}
	IBone* pSkeleton = GetSkeletonRoot();
	if (pSkeleton)
	{
		vector< CEntity* > vSubEntity;
		string sRessourceFileName;
		GetRessource()->GetFileName(sRessourceFileName);
		GetSkeletonEntities(dynamic_cast<CBone*>(pSkeleton), vSubEntity, sRessourceFileName);
		for (unsigned int iSubEntity = 0; iSubEntity < vSubEntity.size(); iSubEntity++)
		{
			ILoader::CObjectInfos* pSubEntityInfo = new ILoader::CEntityInfos;
			vSubEntity[iSubEntity]->GetEntityInfos(pSubEntityInfo);
			animatedEntityInfos.m_vSubEntityInfos.push_back(pSubEntityInfo);
		}
	}
	GetEntityName(animatedEntityInfos.m_sObjectName);
	string sTypeName;
	GetTypeName(sTypeName);
	animatedEntityInfos.m_sTypeName = sTypeName;
	animatedEntityInfos.m_fWeight = GetWeight();
	string sCustomTextureName;
	if (m_pCustomTexture) {
		m_pCustomTexture->GetFileName(sCustomTextureName);
		animatedEntityInfos.m_sTextureName = sCustomTextureName;
	}
	if (m_bUseCustomSpecular) {
		animatedEntityInfos.m_vSpecular = m_vCustomSpecular;
		animatedEntityInfos.m_bUseCustomSpecular = m_bUseCustomSpecular;
	}
	for (INode*& pNode : m_vClothes) {
		string sClothName, sNodeName;

		IEntity* pEntity = dynamic_cast<IEntity*>(pNode);
		if (!pEntity) {
			IBone* pBone = dynamic_cast<IBone*>(pNode);
			if (pBone && pBone->GetChildCount() == 1) {
				pEntity = dynamic_cast<IEntity*>(pBone->GetChild(0));
			}
		}

		IMesh* pMesh = dynamic_cast<IMesh*>(pEntity->GetRessource());
		pMesh->GetFileName(sClothName);

		if (pEntity) {
			for (int i = 0; i < animatedEntityInfos.m_vSubEntityInfos.size(); i++) {
				ILoader::CObjectInfos* pSubEntity = animatedEntityInfos.m_vSubEntityInfos[i];
				if (pSubEntity->m_sRessourceFileName == sClothName)
					animatedEntityInfos.m_vSubEntityInfos.erase(animatedEntityInfos.m_vSubEntityInfos.begin() + i);
			}
			CStringUtils::GetShortFileName(sClothName, sClothName);
			CStringUtils::GetFileNameWithoutExtension(sClothName, sClothName);
			if ((pEntity->GetParent() != this) && pEntity->GetParent()->GetParent()) {
				sNodeName = pEntity->GetParent()->GetParent()->GetName();
			}
			animatedEntityInfos.m_mClothesToNode[sClothName] = sNodeName;
		}
	}
}

void CMobileEntity::BuildFromInfos(const ILoader::CObjectInfos& infos, IEntity* pParent)
{
	m_vClothes.clear();
	CEntity::BuildFromInfos(infos, pParent);
	const ILoader::CAnimatedEntityInfos* pAnimatedEntityInfos = dynamic_cast< const ILoader::CAnimatedEntityInfos* >(&infos);
	if (GetSkeletonRoot()) {
		AddAnimation(pAnimatedEntityInfos->m_sAnimationFileName);
		SetCurrentAnimation(pAnimatedEntityInfos->m_sAnimationFileName);
		if(!pAnimatedEntityInfos->m_sTextureName.empty())
			SetDiffuseTexture(pAnimatedEntityInfos->m_sTextureName);
		m_bUseCustomSpecular = pAnimatedEntityInfos->m_bUseCustomSpecular;
		for (int i = 0; i < m_pMesh->GetMaterialCount(); i++)
			m_vCustomSpecular = pAnimatedEntityInfos->m_vSpecular;
		for (map<string, float>::const_iterator it = pAnimatedEntityInfos->m_mAnimationSpeed.begin(); it != pAnimatedEntityInfos->m_mAnimationSpeed.end(); it++)
			SetAnimationSpeed(CMobileEntity::s_mStringToAnimation[it->first], it->second);
		GetCurrentAnimation()->Play(true);

		for (const pair<string, string>& clothesInfos : pAnimatedEntityInfos->m_mClothesToNode) {
			WearCloth(clothesInfos.first, clothesInfos.second);
		}
	}
}

IEntity::TAnimation CMobileEntity::GetCurrentAnimationType() const
{
	return m_eCurrentAnimationType;
}

void CMobileEntity::TurnEyesH( float fValue )
{
	m_pRightEye->Roll( fValue );
	m_pLeftEye->Roll( fValue );
}

void CMobileEntity::TurnNeckH( float fNeckRotH )
{
	m_pNeck->Pitch( fNeckRotH );
}

ISphere* CMobileEntity::GetBoneSphere( string sBoneName )
{
	IBone* pBone = GetPreloadedBone ( sBoneName );
	float fBoneRadius = pBone->GetBoundingBox()->GetBoundingSphereRadius();
	CVector oBoneWorldPosition;
	pBone->GetWorldPosition(oBoneWorldPosition);	
	return m_oGeometryManager.CreateSphere( oBoneWorldPosition, fBoneRadius / 2.f );
}

void CMobileEntity::AddSpeed(float x, float y, float z)
{
	m_oBody.m_oSpeed.m_x += x;
	m_oBody.m_oSpeed.m_y += y;
	m_oBody.m_oSpeed.m_z += z;
}

IBone* CMobileEntity::GetPreloadedBone( string sName )
{
	IBone* pBone = m_mPreloadedBones[ sName ];
	if( pBone )
		return pBone;
	m_mPreloadedBones[ sName ] = static_cast< IBone* >( GetSkeletonRoot()->GetChildBoneByName( sName ) );
	return m_mPreloadedBones[ sName ];
}


void CMobileEntity::GetPosition( CVector& oPosition ) const
{ 
	GetWorldPosition( oPosition ); 
}

IMesh* CMobileEntity::GetMesh()
{ 
	return dynamic_cast< IMesh* >( m_pRessource ); 
}

IAnimation*	CMobileEntity::GetCurrentAnimation()
{ 
	return m_pCurrentAnimation; 
}

void CMobileEntity::WearSkinnedClothFull(string sClothName)
{
	IEntity* pCloth = m_pEntityManager->CreateEntity(sClothName);
	if (pCloth) {
		m_pCloth = dynamic_cast<CEntity*>(pCloth);
		if (m_pCloth) {
			m_pCloth->Link(m_pScene);
			m_pCloth->AddAnimation("walk.bke");
			m_pCloth->AddAnimation("run.bke");
			m_pCloth->AddAnimation("stand.bke");
			m_pCloth->AddAnimation("stand-normal.bke");
			m_pCloth->AddAnimation("test3.bke");
			m_pCloth->LocalTranslate(0, 24, -1);
			m_pCloth->Link(this);
		}
	}
}

IFighterEntity* CMobileEntity::GetFirstEnemy()
{
	IFighterEntity* pEntity = m_pEntityManager->GetFirstFighterEntity();
	if( pEntity == this )
		pEntity = m_pEntityManager->GetNextFighterEntity();
	return pEntity;
}

IFighterEntity* CMobileEntity::GetNextEnemy()
{
	IFighterEntity* pEntity = static_cast< IFighterEntity* >( m_pEntityManager->GetNextFighterEntity() );
	if( pEntity == this )
		pEntity = static_cast< IFighterEntity* >( m_pEntityManager->GetNextFighterEntity() );
	return pEntity;
}

void CMobileEntity::Stand()
{ 
	Stand( true ); 
}

CMatrix& CMobileEntity::GetWorldTM()
{ 
	return m_oWorldMatrix; 
}

float CMobileEntity::GetBoundingSphereRadius() 
{ 
	return m_fBoundingSphereRadius; 
}
