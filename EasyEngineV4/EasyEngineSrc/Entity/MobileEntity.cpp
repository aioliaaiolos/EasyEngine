#include "MobileEntity.h"
#include "Interface.h"
#include "IFileSystem.h"
#include <algorithm>
#include "ICollisionManager.h"
#include "IGeometry.h"
#include "IPhysic.h"
#include "Utils2/TimeManager.h"
#include "Scene.h"
#include "EntityManager.h"
#include "IGUIManager.h"
#include "Bone.h"
#include "Interface.h"
#include "IFileSystem.h"
#include "Item.h"
#include "Weapon.h"
#include "BoxEntity.h"
#include "Utils2/StringUtils.h"

map< IEntity::TAnimation, string> CCharacter::s_mAnimationTypeToString;

// rapidjson
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/filereadstream.h"
#include <fstream>
#include <rapidjson/prettywriter.h>
using namespace rapidjson;


using namespace rapidjson;

map< string, IEntity::TAnimation >			CCharacter::s_mAnimationStringToType;
map< IEntity::TAnimation, float > 			CCharacter::s_mOrgAnimationSpeedByType;
map< string, CCharacter::TAction >				CCharacter::s_mActions;
vector< CCharacter* >							CCharacter::s_vHumans;
map<string, IEntity::TAnimation> CCharacter::s_mStringToAnimation;
map<string, map<string, pair<string, float>>> CCharacter::s_mBodiesAnimations;

CObject::CObject(EEInterface& oInterface):
	CEntity(oInterface)
{
	m_pfnCollisionCallback = OnCollision;
}

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
	m_pBody->Update();
	if (m_pBody->m_fWeight > 0.f)
	{
		float x, y, z;
		m_oLocalMatrix.GetPosition(x, y, z);
		int nDelta = m_oTimeManager.GetTimeElapsedSinceLastUpdate();
		m_vNextLocalTranslate.m_y += m_pBody->m_oSpeed.m_y * (float)nDelta / 1000.f;
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
		GetEntitiesCollision(entities);		

		CVector localPos;
		oLocalMatrix.GetPosition(localPos);

		CVector directriceLine = m_vNextLocalTranslate;
		CVector first = backupLocal.GetPosition();
		CVector last = localPos;
		CVector firstBottom = first;
		CVector lastBottom = last;
		CVector R = last;
		bool bCollision = false;
		m_bCollideOnObstacle = false;
		float fMaxHeight = -999999.f;
		if (m_pPhysic->GetGravity() > 0.f) {
			for (int i = 0; i < entities.size(); i++) {
				INode* pEntity = entities[i];
				if (pEntity->GetTypeName() == "Item")
					continue;
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
						m_pBody->m_oSpeed.m_y = 0;
						oLocalMatrix.m_13 = last.m_y;
						GetBoundingGeometry()->SetTM(oLocalMatrix);
					}
					else {
						oLocalMatrix.m_03 = last.m_x;
						oLocalMatrix.m_23 = last.m_z;
						GetBoundingGeometry()->SetTM(oLocalMatrix);
						m_bCollideOnObstacle = true;
					}
				}
				else {
					bCollision = true;
				}
			}
		}
		// Ground collision
		const float margin = 10.f;
		CVector nextPosition = m_oLocalMatrix * CVector(0, 0, 100);
		float fGroundHeight = m_pParent->GetGroundHeight(localPos.m_x, localPos.m_z) + margin;
		float fGroundHeightNext = m_pParent->GetGroundHeight(nextPosition.m_x, nextPosition.m_z) + margin;
		float fEntityY = last.m_y - h / 2.f;
		if (fEntityY <= fGroundHeight + m_pPhysic->GetEpsilonError()) {
			m_pBody->m_oSpeed.m_x = 0;
			m_pBody->m_oSpeed.m_y = 0;
			m_pBody->m_oSpeed.m_z = 0;
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

		if ((bCollision || !m_pPhysic->GetGravity()) && m_pfnCollisionCallback) {
			m_pfnCollisionCallback(this, entities);
		}
	}
}


CCharacter::CCharacter(EEInterface& oInterface, string sFileName, string sID):
CObject(oInterface, sFileName),
m_bInitSkeletonOffset( false ),
m_fMaxEyeRotationH( 15 ),
m_fMaxEyeRotationV( 15 ),
m_fMaxNeckRotationH( 45 ),
m_fMaxNeckRotationV( 15 ),
m_fEyesRotH( 0 ),
m_fEyesRotV( 0 ),
m_fNeckRotH( 0 ),
m_fNeckRotV( 0 )
{
	m_sID = sID;
	m_sTypeName = "Human";
	
	for( int i = 0; i < eAnimationCount; i++ )
		m_mAnimationSpeedByType[ (TAnimation)i ] = s_mOrgAnimationSpeedByType[ (TAnimation)i ];

	m_pBody->m_fWeight = 1.f;	
	InitAnimations();

	string bboxFileName = sFileName.substr(0, sFileName.find(".")) + ".bbox";
	ILoader::CAnimationBBoxInfos bboxInfos;
	try {
		m_pLoaderManager->Load(bboxFileName, bboxInfos);
		m_oKeyBoundingBoxes = bboxInfos.mKeyBoundingBoxes;
		string sAnimationName = s_mBodiesAnimations[m_sCurrentBodyName]["Stand"].first;
		string sAnimationNameLow = sAnimationName;
		std::transform(sAnimationName.begin(), sAnimationName.end(), sAnimationNameLow.begin(), tolower);
		map<string, map<int, IBox*> >::iterator itBoxes = m_oKeyBoundingBoxes.find(sAnimationNameLow);
		if (itBoxes == m_oKeyBoundingBoxes.end()) {
			ostringstream oss;
			oss << "Erreur : l'entite '" << sFileName << "' ne possede pas de bounding box pour '" + sAnimationNameLow + "'";
			exception e(oss.str().c_str());
			throw e;
		}
		m_pBoundingGeometry = itBoxes->second.begin()->second;
	}
	catch (CFileNotFoundException& e) {
		m_pBoundingGeometry = m_pMesh->GetBBox();
	}

	s_vHumans.push_back( this );
	m_pLeftEye = dynamic_cast< IEntity* >( m_pSkeletonRoot->GetChildBoneByName( "OeilG" ) );
	m_pRightEye = dynamic_cast< IEntity* >( m_pSkeletonRoot->GetChildBoneByName( "OeilD" ) );
	m_pNeck = m_pSkeletonRoot->GetChildBoneByName( "Cou" );

	m_pfnCollisionCallback = OnCollision;
	m_sSecondaryAttackBoneName = "OrteilsG";
	m_sAttackBoneName = "MainD";
	
	IEntityManager* pEntityManager = static_cast<IEntityManager*>(oInterface.GetPlugin("EntityManager"));
	pEntityManager->AddNewCharacterInWorld(this);

	map< string, map< int, IBox* > >::iterator itBox = m_oKeyBoundingBoxes.find("stand-normal");
	if (itBox != m_oKeyBoundingBoxes.end()) {
		m_pBoundingGeometry = itBox->second.find(160)->second;
	}

	m_pBBox = dynamic_cast<IBox*>(m_pBoundingGeometry);
	m_pDummyRHand = static_cast<CBone*>(m_pSkeletonRoot->GetChildBoneByName("BodyDummyRHand"));
	IBone* pBone = GetPreloadedBone(m_sAttackBoneName);
	const IBox* pAttackBBox = static_cast<const IBox*>(pBone->GetBoundingBox());
	IBox* pRHandBox = static_cast<IBox*>(pAttackBBox ? pAttackBBox->Duplicate() : nullptr);
	if (pRHandBox) {
		CVector oMinPoint;
		pRHandBox->SetMinPoint(oMinPoint);
		m_pRHandBoxEntity = new CBoxEntity(m_oInterface, *pRHandBox);
		m_pRHandBoxEntity->Link(m_pDummyRHand);
		m_pRHandBoxEntity->Hide(true);
	}
	else
		throw CEException("Error in CCharacter::CCharacter() : No bounding box for " + m_sAttackBoneName);

	IAnimation* pReverse = CreateReverseAnimation("MoveToGuardWeaponPart1");
	pReverse->SetSpeed(s_mBodiesAnimations[m_sCurrentBodyName]["MoveToGuardWeaponPart1"].second);
	pReverse = CreateReverseAnimation("MoveToGuardWeaponPart2");
	pReverse->SetSpeed(s_mBodiesAnimations[m_sCurrentBodyName]["MoveToGuardWeaponPart2"].second);
	pReverse = CreateReverseAnimation("Run");
	SetAnimationSetSpeed();
}

CCharacter::~CCharacter()
{

}

IAnimation* CCharacter::CreateReverseAnimation(string sAnimationType)
{
	string sAnimationName = s_mBodiesAnimations[m_sCurrentBodyName][sAnimationType].first;
	IAnimation* pAnimation = m_mAnimation[sAnimationName];
	IAnimation* pReversedAnimation = pAnimation->CreateReversedAnimation();
	pReversedAnimation->SetName(sAnimationName + "Reverse");
	AddAnimation(sAnimationName + "Reverse", pReversedAnimation);
	s_mBodiesAnimations[m_sCurrentBodyName][sAnimationType + "Reverse"].first = sAnimationName + "Reverse";
	return pReversedAnimation;
}

const CMatrix& CCharacter::GetWeaponTM() const
{
	if(m_pCurrentWeapon)
		return m_pCurrentWeapon->GetWorldMatrix();
	return m_pRHandBoxEntity->GetWorldMatrix();
}

bool CCharacter::GetFightMode() const
{
	return m_bFightMode;
}

int CCharacter::GetHitDamage()
{
	return m_nStrength + (m_pCurrentWeapon ? m_pCurrentWeapon->m_nDamage : 0);
}

void CCharacter::LoadAnimationsJsonFile(IFileSystem& oFileSystem)
{
	string sFileName = "animations.json";
	FILE* pFile = oFileSystem.OpenFile(sFileName, "r");
	fclose(pFile);
	string sJsonDirectory;
	oFileSystem.GetLastDirectory(sJsonDirectory);
	string sFilePath = sJsonDirectory + "\\" + sFileName;

	ifstream ifs(sFilePath);
	IStreamWrapper isw(ifs);
	Document doc;
	doc.ParseStream(isw);
	if (doc.IsObject()) {
		if (doc.HasMember("bodiesAnimations")) {
			rapidjson::Value& bodiesAnimations = doc["bodiesAnimations"];
			if (bodiesAnimations.IsArray()) {
				for (int iAnimSet = 0; iAnimSet < bodiesAnimations.Size(); iAnimSet++) {
					vector<string> vBodies;
					Value& bodyAnimation = bodiesAnimations[iAnimSet];
					if (bodyAnimation.HasMember("bodies")) {
						Value& bodies = bodyAnimation["bodies"];
						if (bodies.IsArray()) {
							for (int iBody = 0; iBody < bodies.Size(); iBody++) {
								Value& body = bodies[iBody];
								if (body.IsString()) {
									vBodies.push_back(body.GetString());
								}
							}
						}
					}
					if (bodyAnimation.HasMember("actionAnimations")) {
						Value& actionAnimations = bodyAnimation["actionAnimations"];
						if (actionAnimations.IsArray()) {
							for (int iActionAnim = 0; iActionAnim < actionAnimations.Size(); iActionAnim++) {
								Value& actionAnim = actionAnimations[iActionAnim];
								if (actionAnim.IsObject()) {
									string sMemberName = actionAnim.MemberBegin()->name.GetString();
									string sMemberValue = actionAnim.MemberBegin()->value.GetString();
									for (const string& sBody : vBodies) {										
										s_mBodiesAnimations[sBody][sMemberName].first = sMemberValue;
										if (actionAnim.HasMember("Speed")) {
											Value& speed = actionAnim["Speed"];
											s_mBodiesAnimations[sBody][sMemberName].second = speed.GetFloat();
										}
										else
											s_mBodiesAnimations[sBody][sMemberName].second = 1.f;
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void CCharacter::InitAnimations()
{
	string sMask = "Animations/*.bke";
	WIN32_FIND_DATAA oData;
	IFileSystem* pFileSystem = static_cast<IFileSystem*>(m_oInterface.GetPlugin("FileSystem"));
	HANDLE hFirst = pFileSystem->FindFirstFile_EE(sMask, oData);
	if (hFirst != INVALID_HANDLE_VALUE)
	{
		if (m_pCurrentAnimation) {
			m_pCurrentAnimation->Stop();
			m_pCurrentAnimation = nullptr;
		}
		m_mAnimation.clear();		
		m_pRessource->GetFileName(m_sCurrentBodyName);
		CStringUtils::GetShortFileName(m_sCurrentBodyName, m_sCurrentBodyName);
		CStringUtils::GetFileNameWithoutExtension(m_sCurrentBodyName, m_sCurrentBodyName);
		
		bool endLoop = false;
		map<string, pair<string, float>>& bodyAnimations = s_mBodiesAnimations[m_sCurrentBodyName];
		for (const pair<string, pair<string, float>>& actionAnim : bodyAnimations) {
			AddAnimation(actionAnim.second.first);
			TAnimation animType = s_mAnimationStringToType[actionAnim.first];
			if (animType == eStand)
				m_sStandAnimation = actionAnim.second.first;
		}
	}
}

void CCharacter::InitStatics(IFileSystem& oFileSystem)
{
	s_mAnimationStringToType["Walk"] = eWalk;
	s_mAnimationStringToType["Run"] = eRun;
	s_mAnimationStringToType["RunReverse"] = eRunReverse;
	s_mAnimationStringToType["Stand"] = eStand;
	s_mAnimationStringToType["HitReceived"] = eReceiveHit;
	s_mAnimationStringToType["HitWeapon"] = eHitWeapon;
	s_mAnimationStringToType["HitLeftFoot"] = eHitLeftFoot;
	s_mAnimationStringToType["HitRightArm"] = eHitRightArm;
	s_mAnimationStringToType["Jump"] = eJump;
	s_mAnimationStringToType["Dying"] = eDying;
	s_mAnimationStringToType["Guard"] = eGuard;
	s_mAnimationStringToType["MoveToGuard"] = eMoveToGuard;
	s_mAnimationStringToType["MoveToGuardWeaponPart1"] = eMoveToGuardWeaponPart1;
	s_mAnimationStringToType["MoveToGuardWeaponPart2"] = eMoveToGuardWeaponPart2;
	s_mAnimationStringToType["MoveToGuardWeaponPart1Reverse"] = eMoveToGuardWeaponPart1Reverse;
	s_mAnimationStringToType["MoveToGuardWeaponPart2Reverse"] = eMoveToGuardWeaponPart2Reverse;
	s_mAnimationStringToType["Original"] = eOriginal;

	s_mAnimationTypeToString[eWalk] = "Walk";
	s_mAnimationTypeToString[eRun] = "Run";
	s_mAnimationTypeToString[eRunReverse] = "RunReverse";
	s_mAnimationTypeToString[eStand] = "Stand";
	s_mAnimationTypeToString[eReceiveHit] = "HitReceived";
	s_mAnimationTypeToString[eHitWeapon] = "HitWeapon";
	s_mAnimationTypeToString[eHitLeftFoot] = "HitLeftFoot";
	s_mAnimationTypeToString[eHitRightArm] = "HitRightArm";
	s_mAnimationTypeToString[eJump] = "Jump";
	s_mAnimationTypeToString[eDying] = "Dying";
	s_mAnimationTypeToString[eMoveToGuard] = "MoveToGuard";
	s_mAnimationTypeToString[eGuard] = "Guard";
	s_mAnimationTypeToString[eMoveToGuardWeaponPart1] = "MoveToGuardWeaponPart1";
	s_mAnimationTypeToString[eMoveToGuardWeaponPart2] = "MoveToGuardWeaponPart2";
	s_mAnimationTypeToString[eMoveToGuardWeaponPart1Reverse] = "MoveToGuardWeaponPart1Reverse";
	s_mAnimationTypeToString[eMoveToGuardWeaponPart2Reverse] = "MoveToGuardWeaponPart2Reverse";
	s_mAnimationTypeToString[eOriginal] = "Original";

	s_mOrgAnimationSpeedByType[eWalk] = -1.6f;
	s_mOrgAnimationSpeedByType[eStand] = 0.f;
	s_mOrgAnimationSpeedByType[eHitWeapon] = 0.f;
	s_mOrgAnimationSpeedByType[eRun] = -7.f;
	s_mOrgAnimationSpeedByType[eRunReverse] = 7.f;
	s_mOrgAnimationSpeedByType[eHitLeftFoot] = 0.f;
	s_mOrgAnimationSpeedByType[eReceiveHit] = 0.f;
	s_mOrgAnimationSpeedByType[eDying] = 0.f;
	s_mOrgAnimationSpeedByType[eMoveToGuard] = 0.f;
	s_mOrgAnimationSpeedByType[eMoveToGuardWeaponPart1] = 0.f;
	s_mOrgAnimationSpeedByType[eMoveToGuardWeaponPart2] = 0.f;
	s_mOrgAnimationSpeedByType[eOriginal] = 0.f;

	s_mActions["Walk"] = Walk;
	s_mActions["Run"] = Run;
	s_mActions["RunReverse"] = RunReverse;
	s_mActions["Stand"] = Stand;
	s_mActions["ReceiveHit"] = ReceiveHit;
	s_mActions["Jump"] = Jump;
	s_mActions["Dying"] = Dying;
	s_mActions["MoveToGuard"] = MoveToGuard;
	s_mActions["MoveToGuardWeapon"] = MoveToGuardWeapon;
	s_mActions["StopRunning"] = StopRunning;

	s_mStringToAnimation["Run"] = IEntity::eRun;

	LoadAnimationsJsonFile(oFileSystem);
}

IGeometry* CCharacter::GetBoundingGeometry()
{
	string sAnimationType = "Stand";
	if (m_bFightMode)
		sAnimationType = "Guard";

	string sAnimationName = s_mBodiesAnimations[m_sCurrentBodyName][sAnimationType].first;
	string sAnimationNameLow = sAnimationName;
	std::transform(sAnimationName.begin(), sAnimationName.end(), sAnimationNameLow.begin(), tolower);
	map<int, IBox*>::iterator itBox = m_oKeyBoundingBoxes[sAnimationNameLow].begin();
	if (itBox != m_oKeyBoundingBoxes[sAnimationNameLow].end()) {
		return itBox->second;
	}
	return nullptr;

	if (m_pCurrentAnimation) {
		string sAnimationName;
		m_pCurrentAnimation->GetName(sAnimationName);
		sAnimationName = sAnimationName.substr(sAnimationName.find_last_of("/") + 1);
		int time = m_pCurrentAnimation->GetAnimationTime();
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
		throw CEException("Warning : you don't associated current animation to bounding boxes for character '" + m_sID + "', the default character box will be used");
	}
	return m_pMesh->GetBBox();
}

IGeometry* CCharacter::GetAttackGeometry()
{
	return m_pWeaponGeometry;
}

const string& CCharacter::GetSecondaryAttackBoneName()
{
	return m_sSecondaryAttackBoneName;
}

void CCharacter::WearArmorToDummy(string armorName)
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

void CCharacter::UnWearShoes(string shoesPath)
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

void CCharacter::UnWearAllShoes()
{
	IBone* pBodyDummyLShoes = m_pSkeletonRoot->GetChildBoneByName("BodyDummyLFoot");
	pBodyDummyLShoes->ClearChildren();
	IBone* pBodyDummyRShoes = m_pSkeletonRoot->GetChildBoneByName("BodyDummyRFoot");
	pBodyDummyRShoes->ClearChildren();
}


void CCharacter::WearShoes(string shoesPath)
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

void CCharacter::Wear(string sClothPath, string sDummyName)
{
	string clothPathLower = sClothPath;
	std::transform(sClothPath.begin(), sClothPath.end(), clothPathLower.begin(), tolower);

	string sClothName = sClothPath.substr(sClothPath.find_last_of("/") + 1);
	CEntity* pCloth = new CEntity(m_oInterface, string("Meshes/") + sClothName + ".bme");
	Wear(pCloth, sDummyName);
}

void CCharacter::Wear(CEntity* pCloth, string sDummyName)
{
	if (pCloth) {
		IMesh* pMesh = pCloth->GetMesh();
		if (pMesh && pMesh->IsSkinned()) {
			pCloth->SetSkeletonRoot(m_pSkeletonRoot, m_pOrgSkeletonRoot);
			pCloth->Link(this);
		}
		else {
			pCloth->LinkDummyParentToDummyEntity(this, sDummyName);
			//delete pCloth;
		}
	}
}

void CCharacter::UnWear(CEntity* pCloth)
{
	if (pCloth) {
		IMesh* pMesh = pCloth->GetMesh();
		if (pMesh && pMesh->IsSkinned())
			pCloth->Unlink();
		else
			pCloth->UnLinkDummyParentToDummyEntity();
	}
}

void CCharacter::AddItem(string sItemID)
{
	IItem* pItem = m_pEntityManager->CreateItemEntity(sItemID);
	if (pItem) {
		m_mItems[sItemID].push_back(pItem);
		pItem->SetOwner(this);
	}
	else
		throw CEException(string("Error in CCharacter::AddItem() : item '" + sItemID + "' not found"));
}

void CCharacter::AddItem(CItem* pItem)
{
	m_mItems[pItem->GetIDStr()].push_back(pItem);
	pItem->SetOwner(this);
}

void CCharacter::RemoveItem(string sItemID)
{
	map<string, vector<IItem*>>::iterator itItem = m_mItems.find(sItemID);
	if (itItem != m_mItems.end()) {
		itItem->second.back()->UnWear();
		itItem->second.back()->SetOwner(nullptr);
		itItem->second.pop_back();
		if(itItem->second.size() == 0)
			m_mItems.erase(itItem);
	}
	else
		throw CEException(string("Error in CCharacter::RemoveItem() : item '") + sItemID + "' not exists");
}

void CCharacter::WearItem(string sItemID)
{
	map<string, vector<IItem*>>::iterator itItem = m_mItems.find(sItemID);
	if (itItem != m_mItems.end()) {
		CItem* pItem = dynamic_cast<CItem*>(itItem->second.at(0));
		WearItem(pItem);
	}
	else {
		throw CEException(string("Error in CCharacter::WearItem() : item '") + sItemID + "' not exists");
	}
}

void CCharacter::WearItem(IItem* pBaseItem)
{
	CItem* pItem = dynamic_cast<CItem*>(pBaseItem);
	if (!pItem->GetMesh())
		pItem->Load();
	pItem->m_bIsWear = true;
	const string& sDummyName = pItem->GetDummyNames().at(0);
	Wear(pItem, sDummyName);
}

void CCharacter::UnWearItem(string sItemID)
{
	map<string, vector<IItem*>>::iterator itItem = m_mItems.find(sItemID);
	if (itItem != m_mItems.end())
		UnWearItem(itItem->second.at(0));
	else
		throw CEException(string("Error in CCharacter::RemoveItem() : item '") + sItemID + "' not exists");
}

void CCharacter::UnWearItem(IItem* pBaseItem)
{
	CItem* pItem = dynamic_cast<CItem*>(pBaseItem);
	if (pItem->m_bIsWear) {
		pItem->m_bIsWear = false;
		UnWear(pItem);
		if (pItem == m_pCurrentWeapon)
			m_pCurrentWeapon = nullptr;
	}
}

int CCharacter::GetItemCount(string sItemID)
{
	map<string, vector<IItem*>>::iterator itItem = m_mItems.find(sItemID);
	if (itItem != m_mItems.end()) {
		return itItem->second.size();
	}
	return 0;
}

void CCharacter::SetCurrentWeapon(CWeapon* pWeapon)
{
	m_pCurrentWeapon = pWeapon;
}

void CCharacter::SetFightMode(bool fightMode)
{
	if (m_bFightMode == fightMode)
		return;
	m_bFightMode = fightMode;
	if (m_bFightMode) {
		if (m_pCurrentWeapon) {
			m_pWeaponGeometry = m_pCurrentWeapon->GetBoundingGeometry();
			MoveToGuardWeaponPart1();
			m_pCurrentAnimation->AddCallback([this](IAnimation::TEvent e)
			{
				switch (e) {
				case IAnimation::TEvent::eBeginRewind:
					m_pCurrentAnimation->RemoveAllCallback();
					m_pCurrentWeapon->LinkToHand(m_pDummyRHand);
					MoveToGuardWeaponPart2();
					m_pCurrentAnimation->AddCallback([this](IAnimation::TEvent e) {
						switch (e) {
						case IAnimation::TEvent::eBeginRewind:
							e = e;
							break;
						}
					});
					break;
				}
			});
		}
		else {
			m_pWeaponGeometry = m_pRHandBoxEntity->GetBoundingGeometry();
			MoveToGuard();
		}
	}
	else {
		if (m_pCurrentWeapon) {
			MoveToGuardWeaponPart2Reverse();
			m_pCurrentAnimation->AddCallback([this](IAnimation::TEvent e)
			{
				switch (e) {
				case IAnimation::TEvent::eBeginRewind:
					m_pCurrentAnimation->RemoveAllCallback();
					m_pCurrentWeapon->Wear();
					MoveToGuardWeaponPart1Reverse();
					break;
				}

			});
		}
		else {
			Stand();				
		}
	}
}

bool CCharacter::GetFightMode()
{
	return m_bFightMode;
}

void CCharacter::Link(INode* pParent)
{
	CEntity::Link(pParent);
	CEntity* pParentEntity = dynamic_cast<CEntity*>(m_pParent);
	m_pCollisionMap = pParentEntity->GetCollisionMap();
}

IBox* CCharacter::GetBoundingBox()
{
	return m_pBBox;
}

const map<string, vector<IItem*>>& CCharacter::GetItems() const
{
	return m_mItems;
}

void CCharacter::SetHairs(string hairsName)
{
	string hairsPathLower = hairsName;
	std::transform(hairsName.begin(), hairsName.end(), hairsPathLower.begin(), tolower);

	// unlink old hairs if exists
	IBone* pDummyHairs = m_pSkeletonRoot->GetChildBoneByName("DummyHairs");
	
	if (pDummyHairs)
		pDummyHairs->Unlink();

	// link new hairs
	CEntity* hairs = new CEntity(m_oInterface, string("Meshes/hairs/") + hairsName + ".bme");
	hairs->LinkDummyParentToDummyEntity(this, "BodyDummyHairs");
	if (hairs->GetSkeletonRoot() && hairs->GetSkeletonRoot()->GetChildCount() > 0) {
		IEntity* pEntity = dynamic_cast<IEntity*>(hairs->GetSkeletonRoot()->GetChild(0));
		if (pEntity)
			pEntity->SetEntityID(hairsName);
	}
	delete hairs;
}

void CCharacter::SetBody(string sBodyName)
{
	ILoader::CObjectInfos* pInfos = nullptr;
	GetEntityInfos(pInfos);
	int slashPos = pInfos->m_sRessourceFileName.find_last_of('/');
	pInfos->m_sRessourceFileName = pInfos->m_sRessourceFileName.substr(0, slashPos + 1) + sBodyName;

	SetRessource(string("Meshes/Bodies/") + sBodyName);
	InitAnimations();

	IEntity* pParent = dynamic_cast<IEntity*>(GetParent());
	BuildFromInfos(*pInfos, pParent, true);
}

void CCharacter::RunAction( string sAction, bool bLoop )
{
	map<string, CCharacter::TAction>::iterator itAction = s_mActions.find(sAction);
	if (itAction != s_mActions.end())
		itAction->second(this, bLoop);
}

void CCharacter::SetPredefinedAnimation( string s, bool bLoop, int nFrameNumber)
{
	IMesh* pMesh = static_cast< IMesh* >( m_pRessource );
	string sAnimationName = s_mBodiesAnimations[m_sCurrentBodyName][s].first;
	SetCurrentAnimation( sAnimationName );
	if( !m_pCurrentAnimation )
	{
		string sMessage = string( "Erreur : Animation type \"" ) + s + "\" manquante";
		CEException e( sMessage );
		throw e;
	}
	m_pCurrentAnimation->SetAnimationTime(nFrameNumber);
	m_pCurrentAnimation->Play( bLoop );
	m_eCurrentAnimationType = s_mAnimationStringToType[ s ];
}

void CCharacter::Walk( bool bLoop )
{
	if (m_eCurrentAnimationType != eWalk)
	{
		SetPredefinedAnimation("Walk", bLoop);
		if (!m_bUsePositionKeys)
			ConstantLocalTranslate(CVector(0.f, 0.f, -m_mAnimationSpeedByType[eWalk]));
	}
}

void CCharacter::Stand( bool bLoop )
{
	SetPredefinedAnimation( "Stand", bLoop );
	if( !m_bUsePositionKeys )
		ConstantLocalTranslate( CVector( 0.f, m_mAnimationSpeedByType[ eStand ], 0.f ) );
}

void CCharacter::Run( bool bLoop )
{
	if( m_eCurrentAnimationType != eRun ) {
		SetPredefinedAnimation( "Run", bLoop );
		if( !m_bUsePositionKeys )
			ConstantLocalTranslate( CVector( 0.f, 0.f, -m_mAnimationSpeedByType[eRun]) );
	}
}

void CCharacter::RunReverse(bool bLoop)
{
	if (m_eCurrentAnimationType != eRunReverse) {
		SetPredefinedAnimation("RunReverse", bLoop);
		if (!m_bUsePositionKeys)
			ConstantLocalTranslate(CVector(0.f, 0.f, -m_mAnimationSpeedByType[eRunReverse]));
	}
}

void CCharacter::Jump(bool bLoop)
{
	if (m_eCurrentAnimationType != eJump)
	{
		//SetPredefinedAnimation("jump", bLoop);
		//if (!m_bUsePositionKeys)
		//ConstantLocalTranslate(CVector(0.f, m_mAnimationSpeedByType[eJump], 0.f));
		m_pBody->m_oSpeed.m_y = 2000;
	}
}

void CCharacter::Die()
{
	if (m_eCurrentAnimationType != eDying) {
		SetPredefinedAnimation("Dying", false);
		m_pCurrentAnimation->AddCallback([this](IAnimation::TEvent e){ m_eCurrentAnimationType = eNone;});
	}
}

void CCharacter::Yaw(float fAngle)
{
	if(GetLife() > 0)
		CNode::Yaw(fAngle);
}

void CCharacter::Pitch(float fAngle)
{
	if (GetLife() > 0)
		CNode::Pitch(fAngle);
}

void CCharacter::Roll(float fAngle)
{
	if (GetLife() > 0)
		CNode::Roll(fAngle);
}

void CCharacter::PlayHitAnimation()
{
	if (m_pCurrentWeapon) {
		SetPredefinedAnimation("HitWeapon", false);
	}
	else
		SetPredefinedAnimation("HitRightArm", false);
}

void CCharacter::PlaySecondaryHitAnimation()
{
	SetPredefinedAnimation("HitLeftFoot", false);
}

void CCharacter::ReceiveHit( bool bLoop )
{
	SetPredefinedAnimation( "HitReceived", bLoop );
	if( !m_bUsePositionKeys )
		ConstantLocalTranslate( CVector( 0.f, m_mAnimationSpeedByType[ eReceiveHit ], 0.f ) );
}

void CCharacter::ReceiveHit( CCharacter* pEntity, bool bLoop )
{
	pEntity->ReceiveHit( bLoop );
}

void CCharacter::ReceiveHit()
{
	RunAction("ReceiveHit", false);
}

void CCharacter::MoveToGuard()
{
	//RunAction("guard", false);
	if (m_eCurrentAnimationType != eMoveToGuard) {
		SetPredefinedAnimation("MoveToGuard", false);
	}
}

void CCharacter::HitWeapon()
{
	if (m_eCurrentAnimationType != eHitWeapon) {
		SetPredefinedAnimation("HitWeapon", false);
	}
}

void CCharacter::MoveToGuardWeaponPart1()
{
	if (m_eCurrentAnimationType != eMoveToGuardWeaponPart1) {
		SetPredefinedAnimation("MoveToGuardWeaponPart1", false);
	}
}

void CCharacter::MoveToGuardWeaponPart2()
{
	if (m_eCurrentAnimationType != eMoveToGuardWeaponPart2) {
		SetPredefinedAnimation("MoveToGuardWeaponPart2", false);
	}
}

void CCharacter::MoveToGuardWeaponPart1Reverse()
{
	if (m_eCurrentAnimationType != eMoveToGuardWeaponPart1Reverse) {
		SetPredefinedAnimation("MoveToGuardWeaponPart1Reverse", false);
	}
}

void CCharacter::MoveToGuardWeaponPart2Reverse()
{
	if (m_eCurrentAnimationType != eMoveToGuardWeaponPart2Reverse) {
		SetPredefinedAnimation("MoveToGuardWeaponPart2Reverse", false);
	}
}

void CCharacter::Guard()
{
	//RunAction("guard", false);
	if (m_eCurrentAnimationType != eMoveToGuard) {
		SetPredefinedAnimation("Guard", false);
	}
}

void CCharacter::OnWalkAnimationCallback( IAnimation::TEvent e, void* pData )
{
	if( e == IAnimation::eBeginRewind )
	{
		CCharacter* pHuman = reinterpret_cast< CCharacter* >( pData );
		pHuman->LocalTranslate( 0, -pHuman->m_oSkeletonOffset.m_23, 0 );
	}
}

void CCharacter::Walk( CCharacter* pCharacter, bool bLoop  )
{
	pCharacter->Walk( bLoop );
}

void CCharacter::Stand( CCharacter* pCharacter, bool bLoop  )
{
	pCharacter->Stand( bLoop );
}

void CCharacter::Run( CCharacter* pCharacter, bool bLoop  )
{
	pCharacter->Run( bLoop );
}

void CCharacter::RunReverse(CCharacter* pCharacter, bool bLoop)
{
	pCharacter->RunReverse(bLoop);
}

void CCharacter::Jump(CCharacter* pCharacter, bool bLoop)
{
	pCharacter->Jump(bLoop);
}

void CCharacter::Dying(CCharacter* pCharacter, bool bLoop)
{
	pCharacter->Die();
}

void CCharacter::MoveToGuard(CCharacter* pCharacter, bool bLoop)
{
	pCharacter->MoveToGuard();
}

void CCharacter::MoveToGuardWeapon(CCharacter* pCharacter, bool bLoop)
{
	pCharacter->MoveToGuard();
}

void CCharacter::StopRunning(CCharacter* pCharacter, bool bLoop)
{
	if (!pCharacter->m_bFightMode)
		pCharacter->Stand();
	else {
		pCharacter->MoveToGuardWeaponPart2();
		pCharacter->GetCurrentAnimation()->SetAnimationTime(pCharacter->GetCurrentAnimation()->GetEndAnimationTime());
		if (!pCharacter->m_bUsePositionKeys)
			pCharacter->ConstantLocalTranslate(CVector(0.f, pCharacter->m_mAnimationSpeedByType[eStand], 0.f));
	}
}

IAnimation* CCharacter::GetAnimation(TAnimation eAnimationType)
{
	string sAnimationName = s_mBodiesAnimations[m_sCurrentBodyName][s_mAnimationTypeToString[eAnimationType]].first;
	if (!sAnimationName.empty()) {
		map<string, IAnimation*>::iterator itAnimation = m_mAnimation.find(sAnimationName);
		if (itAnimation != m_mAnimation.end()) {
			return itAnimation->second;
		}
	}
	return nullptr;
}

void CCharacter::SetAnimationSetSpeed()
{
	map<string, pair<string, float>>& animations = s_mBodiesAnimations[m_sCurrentBodyName];
	for (const pair<string, pair<string, float>>& anim : animations) {
		map< string, IEntity::TAnimation >::iterator itAnimation = s_mAnimationStringToType.find(anim.first);
		if((itAnimation != s_mAnimationStringToType.end()) && (anim.second.second > 0.f))
			SetAnimationSpeed(itAnimation->second, anim.second.second);
	}
}

void CCharacter::SetAnimationSpeed(TAnimation eAnimationType, float fSpeed)
{
	IAnimation* pAnimation = GetAnimation(eAnimationType);
	if (pAnimation != nullptr)
		pAnimation->SetSpeed(fSpeed);
}

void CCharacter::SetMovmentSpeed(TAnimation eAnimationType, float fSpeed)
{
	IAnimation* pAnimation = GetAnimation(eAnimationType);
	SetAnimationSpeed(eAnimationType, pAnimation->GetSpeed() * fSpeed);
	m_mAnimationSpeedByType[eAnimationType] = s_mOrgAnimationSpeedByType[eAnimationType] * fSpeed;
}

float CCharacter::GetAnimationSpeed(IEntity::TAnimation eAnimationType)
{
	string sType = s_mAnimationTypeToString[eAnimationType];
	string sAnimationName = s_mBodiesAnimations[m_sCurrentBodyName][sType].first;
	map<string, IAnimation*>::iterator itAnimation = m_mAnimation.find(sAnimationName);
	IAnimation* pAnimation = nullptr;
	if (itAnimation != m_mAnimation.end())
		pAnimation = itAnimation->second;
	if(pAnimation)
		return pAnimation->GetSpeed();
	return 1.f;
}

void CCharacter::GetEntityInfos(ILoader::CObjectInfos*& pInfos)
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
		for (map<string, IEntity::TAnimation>::iterator it = CCharacter::s_mStringToAnimation.begin(); it != CCharacter::s_mStringToAnimation.end(); it++) {
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
	string sTypeName = GetTypeName();
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


	for (const pair<string, vector<IItem*>>& item : m_mItems) {
		for (IItem* pItem : item.second) {
			animatedEntityInfos.m_mItems[item.first].push_back(pItem->IsWear() ? 1 : 0);
		}
	}
	IBone* pDummyHairs = m_pSkeletonRoot->GetChildBoneByName("DummyHairs");
	if (pDummyHairs) {
		if (pDummyHairs->GetChildCount() > 0) {
			IEntity* pHairs = dynamic_cast<IEntity*>(pDummyHairs->GetChild(0));
			animatedEntityInfos.m_sHairs = pHairs->GetIDStr();
		}
	}

}

void CCharacter::BuildFromInfos(const ILoader::CObjectInfos& infos, IEntity* pParent, bool bExcludeChildren)
{
	CEntity::BuildFromInfos(infos, pParent, true);
	const ILoader::CAnimatedEntityInfos* pAnimatedEntityInfos = dynamic_cast< const ILoader::CAnimatedEntityInfos* >(&infos);
	if (GetSkeletonRoot()) {
		if(!pAnimatedEntityInfos->m_sTextureName.empty())
			SetDiffuseTexture(pAnimatedEntityInfos->m_sTextureName);
		m_bUseCustomSpecular = pAnimatedEntityInfos->m_bUseCustomSpecular;
		for (int i = 0; i < m_pMesh->GetMaterialCount(); i++)
			m_vCustomSpecular = pAnimatedEntityInfos->m_vSpecular;
		for (map<string, float>::const_iterator it = pAnimatedEntityInfos->m_mAnimationSpeed.begin(); it != pAnimatedEntityInfos->m_mAnimationSpeed.end(); it++)
			SetMovmentSpeed(CCharacter::s_mStringToAnimation[it->first], it->second);
		Stand();
		for (const pair<string, vector<int>>& item : pAnimatedEntityInfos->m_mItems) {
			CItem* pItem = new CItem(*m_pEntityManager->GetItem(item.first));
			m_mItems[item.first].push_back(pItem);
			pItem->SetOwner(this);
			pItem->m_bIsWear = item.second[0] == 1;
			if (pItem->m_bIsWear)
				WearItem(pItem->GetIDStr());
		}
		if (!pAnimatedEntityInfos->m_sHairs.empty())
			SetHairs(pAnimatedEntityInfos->m_sHairs);
	}
}

void CCharacter::Save()
{
	SaveToJson();
}

void CCharacter::SaveToJson()
{
	ILoader::CObjectInfos* pInfos = nullptr;
	GetEntityInfos(pInfos);
	ILoader::CAnimatedEntityInfos* pCharacterInfos = static_cast<ILoader::CAnimatedEntityInfos*>(pInfos);

	rapidjson::Document doc;

	string sFileName = "characters.json";
	string root;
	IFileSystem* pFileSystem = static_cast<IFileSystem*>(m_oInterface.GetPlugin("FileSystem"));
	pFileSystem->GetLastDirectory(root);
	string sFilePath = root + "/" + sFileName;

	std::ifstream ifs(sFilePath);

	if (!ifs.eof())
	{
		rapidjson::IStreamWrapper isw(ifs);
		doc.ParseStream(isw);

		if (!doc.IsObject())
		{
			doc.SetObject();
		}
		bool characterExixts = false;
		int characterIndex = 0;
		rapidjson::Value entityID(rapidjson::kStringType);
		string sEntityIDLow = m_sID;
		std::transform(m_sID.begin(), m_sID.end(), sEntityIDLow.begin(), tolower);
		entityID.SetString(sEntityIDLow.c_str(), doc.GetAllocator());

		rapidjson::Value characters(rapidjson::kArrayType);
		if (doc.HasMember("Characters"))
		{
			characters = doc["Characters"];
			for (rapidjson::Value::ConstValueIterator itCharacter = characters.Begin(); itCharacter != characters.End(); itCharacter++)
			{
				const rapidjson::Value& character = *itCharacter;
				if (character.IsObject()) {
					if (character.HasMember("EntityID")) {
						if (character["EntityID"] == entityID.GetString()) {
							characterExixts = true;
							break;
						}
					}
					else
						throw CEException("Error : Character does not have an 'EntityID' member");
				}
				characterIndex++;
			}
		}

		rapidjson::Value character(rapidjson::kObjectType);
		rapidjson::Value name(rapidjson::kStringType);
		character.AddMember("EntityID", entityID, doc.GetAllocator());		

		rapidjson::Value ressourceName(kStringType), ressourceFileName(kStringType), parentName(kStringType), parentBoneID(kNumberType), typeName(kStringType),
			weight(kNumberType), strength(kNumberType), grandParentDummyRootID(kNumberType), diffuseTextureName(kStringType), useCustomSpecular(kNumberType), specular(kArrayType), animationSpeeds(kArrayType),
			items(kArrayType), hairs(kStringType);

		ressourceName.SetString(pInfos->m_sRessourceName.c_str(), doc.GetAllocator());
		ressourceFileName.SetString(pInfos->m_sRessourceFileName.c_str(), doc.GetAllocator());
		parentName.SetString(pInfos->m_sParentName.c_str(), doc.GetAllocator());
		parentBoneID.SetInt(pInfos->m_nParentBoneID);
		typeName.SetString(pCharacterInfos-> m_sTypeName.c_str(), doc.GetAllocator());
		weight.SetFloat(pCharacterInfos->m_fWeight);
		strength.SetInt(m_nStrength);
		grandParentDummyRootID.SetInt(pCharacterInfos->m_nGrandParentDummyRootID);
		diffuseTextureName.SetString(pCharacterInfos->m_sTextureName.c_str(), doc.GetAllocator());
		specular.SetArray();		
		rapidjson::Value specX(rapidjson::kNumberType), specY(rapidjson::kNumberType), specZ(rapidjson::kNumberType);
		specX.SetFloat(pCharacterInfos->m_vSpecular.m_x);
		specY.SetFloat(pCharacterInfos->m_vSpecular.m_y);
		specZ.SetFloat(pCharacterInfos->m_vSpecular.m_y);
		specular.PushBack(specX, doc.GetAllocator());
		specular.PushBack(specY, doc.GetAllocator());
		specular.PushBack(specZ, doc.GetAllocator());

		rapidjson::Value animations(rapidjson::kArrayType);
		for (map<string, float>::const_iterator it = pCharacterInfos->m_mAnimationSpeed.begin(); it != pCharacterInfos->m_mAnimationSpeed.end(); it++) {
			Value animation{kObjectType};
			rapidjson::Value animationName(rapidjson::kStringType), animationSpeed(rapidjson::kNumberType);
			animationName.SetString(it->first.c_str(), doc.GetAllocator());
			animationSpeed.SetFloat(it->second);
			animation.AddMember("Name", animationName, doc.GetAllocator());
			animation.AddMember("Speed", animationSpeed, doc.GetAllocator());
			animations.PushBack(animation, doc.GetAllocator());
		}
		items.SetArray();
		for (const pair<string, vector<int>>& oItems : pCharacterInfos->m_mItems) {
			rapidjson::Value item(kObjectType), itemName(kStringType);
			item.SetObject();
			itemName.SetString(oItems.first.c_str(), doc.GetAllocator());
			rapidjson::Value isWearArray(rapidjson::kArrayType);
			for (int isWear : oItems.second) {
				isWearArray.PushBack(isWear, doc.GetAllocator());
			}
			item.AddMember("ItemName", itemName, doc.GetAllocator());
			item.AddMember("IsWearArray", isWearArray, doc.GetAllocator());
			items.PushBack(item, doc.GetAllocator());
		}
		hairs.SetString(pCharacterInfos->m_sHairs.c_str(), doc.GetAllocator());
		
		character.AddMember("RessourceName", ressourceName, doc.GetAllocator());
		character.AddMember("RessourceFileName", ressourceFileName, doc.GetAllocator());
		character.AddMember("ParentName", parentName, doc.GetAllocator());
		character.AddMember("ParentBoneID", parentBoneID, doc.GetAllocator());
		character.AddMember("TypeName", typeName, doc.GetAllocator());
		character.AddMember("Weight", weight, doc.GetAllocator());
		character.AddMember("Strength", strength, doc.GetAllocator());
		character.AddMember("GrandParentDummyRootID", grandParentDummyRootID, doc.GetAllocator());
		character.AddMember("DiffuseTextureName", diffuseTextureName, doc.GetAllocator());
		character.AddMember("Specular", specular, doc.GetAllocator());
		character.AddMember("AnimationSpeeds", animations, doc.GetAllocator());
		character.AddMember("Items", items, doc.GetAllocator());
		character.AddMember("Hairs", hairs, doc.GetAllocator());

		if (!characterExixts)
		{
			characters.PushBack(character, doc.GetAllocator());
			if (!doc.HasMember("Characters"))
			{
				doc.AddMember("Characters", characters, doc.GetAllocator());
			}
			else
			{
				doc["Characters"] = characters;
			}
		}
		else {
			characters[characterIndex] = character;
			doc["Characters"] = characters;
		}
	}

	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	writer.SetIndent('\t', 1);
	doc.Accept(writer);
	std::ofstream ofs(sFilePath);
	ofs << buffer.GetString();
	ofs.close();
}

IEntity::TAnimation CCharacter::GetCurrentAnimationType() const
{
	return m_eCurrentAnimationType;
}

void CCharacter::TurnEyesH( float fValue )
{
	m_pRightEye->Roll( fValue );
	m_pLeftEye->Roll( fValue );
}

void CCharacter::TurnNeckH( float fNeckRotH )
{
	m_pNeck->Pitch( fNeckRotH );
}

ISphere* CCharacter::GetBoneSphere( string sBoneName )
{
	IBone* pBone = GetPreloadedBone ( sBoneName );
	float fBoneRadius = pBone->GetBoundingBox()->GetBoundingSphereRadius();
	CVector oBoneWorldPosition;
	pBone->GetWorldPosition(oBoneWorldPosition);	
	return m_oGeometryManager.CreateSphere( oBoneWorldPosition, fBoneRadius / 2.f );
}

void CCharacter::AddSpeed(float x, float y, float z)
{
	m_pBody->m_oSpeed.m_x += x;
	m_pBody->m_oSpeed.m_y += y;
	m_pBody->m_oSpeed.m_z += z;
}

IBone* CCharacter::GetPreloadedBone( string sName )
{
	IBone* pBone = m_mPreloadedBones[ sName ];
	if( pBone )
		return pBone;
	m_mPreloadedBones[ sName ] = static_cast< IBone* >( GetSkeletonRoot()->GetChildBoneByName( sName ) );
	return m_mPreloadedBones[ sName ];
}


void CCharacter::GetPosition( CVector& oPosition ) const
{ 
	GetWorldPosition( oPosition ); 
}

IMesh* CCharacter::GetMesh()
{ 
	return dynamic_cast< IMesh* >( m_pRessource ); 
}

IAnimation*	CCharacter::GetCurrentAnimation()
{ 
	return m_pCurrentAnimation; 
}

IFighterEntity* CCharacter::GetFirstEnemy()
{
	IFighterEntity* pEntity = m_pEntityManager->GetFirstFighterEntity();
	if( pEntity == this )
		pEntity = m_pEntityManager->GetNextFighterEntity();
	return pEntity;
}

IFighterEntity* CCharacter::GetNextEnemy()
{
	IFighterEntity* pEntity = static_cast< IFighterEntity* >( m_pEntityManager->GetNextFighterEntity() );
	if( pEntity == this )
		pEntity = static_cast< IFighterEntity* >( m_pEntityManager->GetNextFighterEntity() );
	return pEntity;
}

void CCharacter::Stand()
{ 
	Stand( true ); 
}

CMatrix& CCharacter::GetWorldTM()
{ 
	return m_oWorldMatrix; 
}

float CCharacter::GetBoundingSphereRadius() 
{ 
	return m_fBoundingSphereRadius; 
}
