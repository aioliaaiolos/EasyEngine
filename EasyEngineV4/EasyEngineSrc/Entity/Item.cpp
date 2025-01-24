#include "Item.h"
#include "IGeometry.h"



map<CItem::Type, vector<string>> CItem::s_mBodyDummies;

map<string, CItem::Type> CItem::s_mTypeString = map<string, CItem::Type>{
	{ "", CItem::eTypeNone },
	{ "Skin", CItem::eSkin },
	{ "Arm", CItem::eArm },
	{ "Right-Arm", CItem::eRightArm },
	{ "Left-Arm", CItem::eLeftArm },
	{ "Chest", CItem::eChest },
	{ "Shield", CItem::eShield },
	{ "Belt", CItem::eBelt },
	{ "Belt-Weapon", CItem::eBeltWeapon},
	{ "Left-Forearm", CItem::eLeftForearm },
	{ "Right-Forearm", CItem::eRightForearm },
	{ "Left-Shoulder", CItem::eLeftShoulder },
	{ "Right-Shoulder", CItem::eRightShoulder },
	{ "Right-Calf", CItem::eRightCalf },
	{ "Left-Calf", CItem::eLeftCalf },
	{ "Left-Foot", CItem::eLeftFoot},
	{ "Right-Foot", CItem::eRightFoot },
	{ "Hairs", CItem::eHairs}
};

map<string, CItem::TClass> CItem::s_mClassString = map<string, CItem::TClass>{ 
	{"", CItem::TClass::eClassNone },
	{"Cloth", CItem::TClass::eCloth},
	{"Armor", CItem::TClass::eArmor},
	{"Jewel", CItem::TClass::eJewel}
};

CItem::CItem(EEInterface& oInterface, string sID, TClass tclass, Type type, string sModelName, string sPreviewPath) :
	CObject(oInterface),
	m_oInterface(oInterface),
	m_eClass(tclass),
	m_eType(type),
	m_sModelName(sModelName),
	m_bIsWear(false),
	m_sPreviewPath(sPreviewPath)
{
	m_sID = sID;
	m_sTypeName = "Item";
}

void CItem::LoadDummyTypes(rapidjson::Document& doc)
{
	if (doc.HasMember("DummyTypes")) {
		rapidjson::Value& dummyTypes = doc["DummyTypes"];
		if (dummyTypes.IsArray()) {
			vector<string> sDummyNames = vector<string>{ "Skin", "Arm", "Right-Arm", "Left-Arm", "Chest", "Belt", "Belt-Weapon",			
				"Left-Forearm", "Right-Forearm", "Left-Shoulder", "Right-Shoulder", "Left-Calf", "Right-Calf" };
			for (int iType = 0; iType < dummyTypes.Size(); iType++) {
				rapidjson::Value& type = dummyTypes[iType];
				string sType = type["Type"].GetString();
				Value& dummies = type["Dummies"];
				for (int iDummy = 0; iDummy < dummies.Size(); iDummy++) {
					string sDummy = dummies[iDummy].GetString();
					s_mBodyDummies[GetTypeFromString(sType.c_str())].push_back(sDummy);
				}
			}
		}
	}
}

CItem::Type CItem::GetTypeFromString(string sType)
{
	map<string, CItem::Type>::iterator itType = s_mTypeString.find(sType);
	if (itType != s_mTypeString.end()) {
		return itType->second;
	}
	return eTypeNone;
}

void CItem::operator=(const CItem& item)
{
	m_eType = item.m_eType;
	m_sModelName = item.m_sModelName;
}

void CItem::Load()
{
	if (m_sModelName.size() > 0)
	{
		SetRessource(string("meshes/") + m_sModelName);
		if (m_pBoundingGeometry) {
			m_fBoundingSphereRadius = m_pBoundingGeometry->ComputeBoundingSphereRadius();
			IBone* pDummyRoot = dynamic_cast<IBone*>(GetChildCount() > 0 ? GetChild(0) : nullptr);
			if (pDummyRoot) {
				for (int i = 0; i < pDummyRoot->GetChildCount(); i++) {
					CEntity* pModel = dynamic_cast<CEntity*>(pDummyRoot->GetChild(i));
					if (pModel) {
						m_pModel = pModel;
						pModel->SetBoundingGeometry(m_pBoundingGeometry);
						if(!m_sDiffuse.empty())
							pModel->SetDiffuseTexture(m_sDiffuse);
					}
				}
			}
			else
				if (!m_sDiffuse.empty())
					SetDiffuseTexture(m_sDiffuse);
		}
	}
}

void CItem::SetOwner(ICharacter* pCharacter)
{
	m_pOwner = pCharacter;
}

void CItem::Wear()
{
	m_pOwner->WearItem(this);
}

void CItem::UnWear()
{
	m_pOwner->UnWearItem(this);
}

const vector<string>& CItem::GetDummyNames()
{
	map<Type, vector<string>>::iterator itDummy = s_mBodyDummies.find(m_eType);
	if (itDummy != s_mBodyDummies.end()) {
		return itDummy->second;
	}
	throw CEException("Error in GetDummyNames() : Type '" + std::to_string((int)m_eType) + "' not found in s_mBodyDummies");
}

string&	CItem::GetPreviewPath()
{
	return m_sPreviewPath;
}

bool CItem::IsWear()
{
	return m_bIsWear;
}

void CItem::DrawBoundingBox(bool bDraw)
{
	if (m_pModel)
		m_pModel->DrawBoundingBox(bDraw);
	else
		CObject::DrawBoundingBox(bDraw);
}

const CMatrix& CItem::GetWorldMatrix() const
{
	if(m_pModel)
		return m_pModel->GetWorldMatrix();
	return CObject::GetWorldMatrix();
}

CEntity* CItem::GetModel()
{
	return m_pModel;
}

void CItem::Update()
{
	if(m_eClass == CItem::TClass::eArmor || m_eClass == CItem::TClass::eCloth)
		m_oRenderer.CullFace(false);
	CObject::Update();
	if (m_eClass == CItem::TClass::eArmor || m_eClass == CItem::TClass::eCloth)
		m_oRenderer.CullFace(true);
}