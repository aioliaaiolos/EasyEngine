#pragma once
#include "Entity.h"

class CItem : public CEntity, public IItem
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

	CItem(EEInterface& oInterface, string sID, Type type, string sModelName, string sPreviewPath);
	void operator=(const CItem& item);
	void SetOwner(ICharacter* pCharacter);
	void Wear() override;
	void UnWear() override;
	void Load();
	const vector<string>& GetDummyNames();
	string&	GetPreviewPath() override;
	bool IsWear() override;

	Type m_eType;
	string m_sModelName;
	
	static map<string, Type> s_mTypeString;
	static map<Type, vector<string>> s_mBodyDummies;

	bool	m_bIsWear;
private:
	string	m_sPreviewPath;
	ICharacter*	m_pOwner = nullptr;
	
};