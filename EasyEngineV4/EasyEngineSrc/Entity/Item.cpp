#include "Item.h"


map<CItem::Type, vector<string>> CItem::s_mBodyDummies = map<CItem::Type, vector<string>>{
	{ CItem::eArmlet, vector<string>{"BodyDummyBrassiereG", "BodyDummyBrassiereD"} },
	{ CItem::eCloth, vector<string>{""} },
	{ CItem::eTopCloth, vector<string>{"BodyDummyCuirasse"} },
	{ CItem::eBottomCloth, vector<string>{"BodyDummyJupe"} },
	{ CItem::eArmorGauntletLeft, vector<string>{"BodyDummyBrassiereG"} },
	{ CItem::eArmorGauntletRight, vector<string>{"BodyDummyBrassiereD"} },
	{ CItem::eArmorCuirass, vector<string>{"BodyDummyCuirasse"} },
	{ CItem::eArmorPauldronLeft, vector<string>{"BodyDummyEpauletteG"} },
	{ CItem::eArmorPauldronRight, vector<string>{"BodyDummyEpauletteD"} },
	{ CItem::eArmorBootLeft, vector<string>{"BodyDummyJambiereG"} },
	{ CItem::eArmorBootRight, vector<string>{"BodyDummyJambiereD"} },
	{ CItem::eArmorGreaves, vector<string>{"BodyDummyJupe"} }
};

map<string, CItem::Type> CItem::s_mTypeString = map<string, CItem::Type>{
	{ "", CItem::eNone },
	{ "Cloth", CItem::eCloth },
	{ "Armlet", CItem::eArmlet },
	{ "TopCloth", CItem::eTopCloth },
	{ "BottomCloth", CItem::eBottomCloth },
	{ "Armor_Gauntlet_Left", CItem::eArmorGauntletLeft },
	{ "Armor_Gauntlet_Right", CItem::eArmorGauntletRight },
	{ "Armor_Cuirass", CItem::eArmorCuirass },
	{ "Armor_Pauldron_Left", CItem::eArmorPauldronLeft },
	{ "Armor_Pauldron_Right", CItem::eArmorPauldronRight },
	{ "Armor_Boot_Right", CItem::eArmorBootRight },
	{ "Armor_Boot_Left", CItem::eArmorBootLeft },
	{ "Armor_Greave", CItem::eArmorGreaves }
};


CItem::CItem(EEInterface& oInterface, string sID, Type type, string sModelName, string sPreviewPath) :
	CEntity(oInterface),
	m_eType(type),
	m_sModelName(sModelName),
	m_bIsWear(false),
	m_sPreviewPath(sPreviewPath)
{
	m_sEntityID = sID;
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