#pragma once
#include "MobileEntity.h"

// rapidjson
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/filereadstream.h"
#include <rapidjson/prettywriter.h>
#include <fstream>
using namespace rapidjson;

class CItem : public CObject, public IItem
{
public:

	enum Type
	{
		eTypeNone = -1,
		eArm = 0,
		eRightArm,
		eLeftArm,
		eSkin,
		eChest,
		eBelt,
		eBeltWeapon,
		eLeftBelt,
		eForearm,
		eShoulder,
		eCalf,
		eLeftForearm,
		eRightForearm,
		eLeftShoulder,
		eRightShoulder,
		eLeftCalf,
		eRightCalf		
	};

	enum TClass
	{
		eClassNone = -1,
		eCloth = 0,
		eArmor,
		eWeapon,
		eJewel
	};

	CItem(EEInterface& oInterface, string sID, TClass tclass, Type type, string sModelName, string sPreviewPath);
	void operator=(const CItem& item);
	void SetOwner(ICharacter* pCharacter);
	void Wear() override;
	void UnWear() override;
	void Load() override;
	const vector<string>& GetDummyNames();
	string&	GetPreviewPath() override;
	bool IsWear() override;

	TClass m_eClass;
	Type m_eType;
	string m_sModelName;
	
	static Type GetTypeFromString(string sType);
	static map<string, TClass> s_mClassString;
	static void LoadDummyTypes(rapidjson::Document& doc);
	static map<Type, vector<string>> s_mBodyDummies;

	bool	m_bIsWear;

protected:
	static map<string, Type> s_mTypeString;
	string	m_sPreviewPath;
	ICharacter*	m_pOwner = nullptr;
	
};