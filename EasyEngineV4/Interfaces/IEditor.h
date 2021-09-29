#pragma once

#include "EEPlugin.h"

class IEditorManager;
class IEntity;
class ICharacter;

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
	virtual void	Edit(string id) = 0;
	virtual void	Load(string fileName) = 0;
	virtual void	HandleEditorManagerCreation(IEditorManager* pEditor) = 0;
	virtual bool	IsEnabled() = 0;
	virtual void	SpawnEntity(string sEntityFileName) = 0;
};

class ISpawnableEditor : public IEditor
{
public:
	ISpawnableEditor(EEInterface& oInterface) : IEditor(oInterface) {}
	virtual void	DisplayPickingRay(bool enable) = 0;
	virtual void	Save(string fileName) = 0;
};

class IMapEditor : virtual public ISpawnableEditor
{
public:
	IMapEditor(EEInterface& oInterface) : ISpawnableEditor(oInterface) {}
	virtual void	SetGroundAdaptationHeight(float fHeight) = 0;
};

class ICharacterEditor : public IEditor
{
public:
	ICharacterEditor(EEInterface& oInterface) : IEditor(oInterface) {}
	virtual void	SetCurrentEditablePlayer(ICharacter* pPlayer) = 0;
	virtual void	SetCurrentEditableNPC(ICharacter* pNPCEntity) = 0;
	virtual void	AddHairs(string sHairsName) = 0;
	virtual void	WearShoes(string sShoesName) = 0;
	virtual void	SetTexture(string sTexture) = 0;
	virtual void	Save() = 0;
};

class IWorldEditor : virtual public ISpawnableEditor
{
public:
	IWorldEditor(EEInterface& oInterface) : ISpawnableEditor(oInterface) {}
	virtual ~IWorldEditor() = 0 {}
};


class IEditorManager : public CPlugin
{
public:
	IEditorManager(EEInterface& oInterface) : CPlugin(nullptr, "EditorManager") {}
	virtual IEditor*	GetEditor(IEditor::Type type) = 0;
};