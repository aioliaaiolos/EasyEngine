#pragma once
#include "Entity.h"

class CItem : public CEntity
{
public:

	enum Type
	{
		eNone = -1,
		eArmlet = 0,
		eCloth,
		eTopCloth,
		eBottomCloth,
		eArmorGauntletLeft,
		eArmorGauntletRight,
		eArmorCuirass,
		eArmorPauldronLeft,
		eArmorPauldronRight,
		eArmorBootLeft,
		eArmorBootRight,
		eArmorGreaves
	};

	CItem(EEInterface& oInterface, string sID, Type type, string sModelName) :
		CEntity(oInterface),
		m_eType(type),
		m_sModelName(sModelName),
		m_bIsWear(false)
	{
		m_sEntityID = sID;
	}

	void operator=(const CItem& item)
	{
		m_eType = item.m_eType;
		m_sModelName = item.m_sModelName;
	}

	void Load()
	{
		if (m_sModelName.size() > 0)
		{
			SetRessource(string("meshes/") + m_sModelName);
		}
	}

	const vector<string>& GetDummyNames()
	{
		map<Type, vector<string>>::iterator itDummy = s_mBodyDummies.find(m_eType);
		if (itDummy != s_mBodyDummies.end()) {
			return itDummy->second;
		}
		throw CEException("Error in GetDummyNames() : Type '" + std::to_string((int)m_eType) + "' not found in s_mBodyDummies");
	}

	Type m_eType;
	string m_sModelName;
	bool	m_bIsWear;

	static map<string, Type> s_mTypeString;
	static map<Type, vector<string>> s_mBodyDummies;
};