#include "Item.h"


map<CItem::Type, vector<string>> CItem::s_mBodyDummies = map<CItem::Type, vector<string>>{
	{ CItem::Type::eArm, vector<string>{"BodyDummyBrassiereG", "BodyDummyBrassiereD"} },
	{ CItem::Type::eSkin, vector<string>{""} },
	{ CItem::Type::eChest, vector<string>{"BodyDummyCuirasse"} },
	{ CItem::Type::eBelt, vector<string>{"BodyDummyJupe"} },
	{ CItem::Type::eLeftForearm, vector<string>{"BodyDummyBrassiereG"} },
	{ CItem::Type::eRightForearm, vector<string>{"BodyDummyBrassiereD"} },
	{ CItem::Type::eChest, vector<string>{"BodyDummyCuirasse"} },
	{ CItem::Type::eLeftShoulder, vector<string>{"BodyDummyEpauletteG"} },
	{ CItem::Type::eRightShoulder, vector<string>{"BodyDummyEpauletteD"} },
	{ CItem::Type::eLeftCalf, vector<string>{"BodyDummyJambiereG"} },
	{ CItem::Type::eRightCalf, vector<string>{"BodyDummyJambiereD"} },
	{ CItem::Type::eBelt, vector<string>{"BodyDummyJupe"} }
};

map<string, CItem::Type> CItem::s_mTypeString = map<string, CItem::Type>{
	{ "", CItem::eTypeNone },
	{ "Skin", CItem::eSkin },
	{ "Arm", CItem::eArm },
	{ "Right-Arm", CItem::eRightArm },
	{ "Left-Arm", CItem::eLeftArm },
	{ "Chest", CItem::eChest },
	{ "Belt", CItem::eBelt },
	{ "Left-Forearm", CItem::eLeftForearm },
	{ "Right-Forearm", CItem::eRightForearm },
	{ "Chest", CItem::eChest},
	{ "Left-Shoulder", CItem::eLeftShoulder },
	{ "Right-Shoulder", CItem::eRightShoulder },
	{ "Right-Calf", CItem::eRightCalf },
	{ "Left-Calf", CItem::eLeftCalf },
	{ "Belt", CItem::eBelt }
};

map<string, CItem::TClass> CItem::s_mClassString = map<string, CItem::TClass>{ 
	{"Cloth", CItem::TClass::eCloth},
	{"Armor", CItem::TClass::eArmor},
	{"Jewel", CItem::TClass::eJewel}
};

CItem::CItem(EEInterface& oInterface, string sID, TClass tclass, Type type, string sModelName, string sPreviewPath) :
	CEntity(oInterface),
	m_eClass(tclass),
	m_eType(type),
	m_sModelName(sModelName),
	m_bIsWear(false),
	m_sPreviewPath(sPreviewPath)
{
	m_sEntityID = sID;
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