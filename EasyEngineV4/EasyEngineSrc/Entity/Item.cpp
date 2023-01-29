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