#pragma once

#include "EEPlugin.h"
#include "ILoader.h"

class IEditorManager;
class IEntity;
class ICharacter;
class IPlayer;

class IEditor : virtual public CPlugin
{
public:
	enum Type
	{
		eMap = 0,
		eCharacter,
		eWorld
	};

	IEditor(EEInterface& oInterface) : CPlugin(nullptr, "") {}
	virtual			~IEditor() = 0 {}
	virtual void	Edit(string fileName) = 0;
	virtual void	Load(string fileName) = 0;
	virtual void	HandleEditorManagerCreation(IEditorManager* pEditor) = 0;
	virtual bool	IsEnabled() = 0;
	virtual void	SpawnEntity(string sEntityFileName) = 0;
	virtual void	SetEditionMode(bool bEditionMode) = 0;
};

class ISpawnableEditor : public IEditor
{
public:
	ISpawnableEditor(EEInterface& oInterface) : IEditor(oInterface) {}
	virtual void	EnableDisplayPickingRaySelected(bool enable) = 0;
	virtual void	EnableDisplayPickingRayMouseMove(bool enable) = 0;
	virtual void	EnableDisplayPickingIntersectPlane(bool enable) = 0;
	virtual void	Save(string fileName) = 0;
};

class IMapEditor : virtual public ISpawnableEditor
{
public:
	IMapEditor(EEInterface& oInterface) : ISpawnableEditor(oInterface) {}
	virtual void	SetGroundAdaptationHeight(float fHeight) = 0;
	virtual void	AdaptGroundToAllEntities() = 0;
	virtual void	SetBias(float fBias) = 0;
};

class ICharacterEditor : public IEditor
{
public:
	ICharacterEditor(EEInterface& oInterface) : IEditor(oInterface) {}
	virtual void	SetCurrentEditablePlayer(IPlayer* pPlayer) = 0;
	virtual void	SetCurrentEditableNPC(ICharacter* pNPCEntity) = 0;
	virtual void	AddHairs(string sHairsName) = 0;
	virtual void	WearShoes(string sShoesName) = 0;
	virtual void	WearCloth(string sClothPath, string sDummyName) = 0;
	virtual void	SetTexture(string sTexture) = 0;
	virtual void	SetBody(string sBodyName) = 0;
	virtual void	Save() = 0;
	virtual void	SetSpecular(float r, float g, float b) = 0;
	virtual void	EditCloth(string sClothName) = 0;
	virtual void	OffsetCloth(float x, float y, float z) = 0;
	virtual void	SaveCurrentEditableCloth() = 0;
	virtual void	OffsetEyes(float x, float y, float z) = 0;
	virtual void	SaveCurrentEditableBody() = 0;
	virtual void	TurnEyes(float fYaw, float fPitch, float fRoll) = 0;
};

class IWorldEditor : virtual public ISpawnableEditor
{
public:
	IWorldEditor(EEInterface& oInterface) : ISpawnableEditor(oInterface) {}
	virtual ~IWorldEditor() = 0 {}
	virtual void	SpawnCharacter(string sID) = 0;
	virtual void	RemoveCharacter(string sID) = 0;
	virtual void	SaveGame(string fileName) = 0;
};


class IEditorManager : public CPlugin
{
public:
	IEditorManager(EEInterface& oInterface) : CPlugin(nullptr, "EditorManager") {}
	virtual IEditor*	GetEditor(IEditor::Type type) = 0;
};