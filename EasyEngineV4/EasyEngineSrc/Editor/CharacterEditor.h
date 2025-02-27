#pragma once

#include "IEditor.h"
#include "Editor.h"
#include "IEventDispatcher.h"

class EEInterface;
class IEntityManager;
class ISceneManager;
class IInputManager;
class ICameraManager;
class IScene;
class ICharacter;
class CPlugin;
class CWorldEditor;
class ICamera;

class CCharacterEditor : public ICharacterEditor, public CEditor
{
public:
	CCharacterEditor(EEInterface& oInterface, ICameraManager::TCameraType type);
	void					SetEditionMode(bool bEditionMode) override;
	void					Load(string sCharacterId) override;
	void					Save() override;
	string					GetName() override;
	bool					IsEnabled() override;
	void					SpawnEntity(string sEntityFileName) override;
	void					SetCurrentEditablePlayer(IPlayer* pPlayer) override;
	void					SetCurrentEditableNPC(ICharacter* pNPCEntity) override;
	void					SetHairs(string sHairsName) override;
	void					WearShoes(string sShoesName) override;
	void					UnWearShoes(string sShoesName) override;
	void					UnWearAllShoes() override;
	void					SetTexture(string sTexture) override;
	void					SetBody(string sBodyName) override;
	void					SetUnique(bool bUnique = true) override;
	void					Edit(string id) override;
	void					SetSpecular(float r, float g, float b) override;
	void					SetShininess(float fValue) override;
	void					EditCloth(string sClothName);
	void					OffsetCloth(float x, float y, float z);
	void					OffsetEyes(float x, float y, float z);
	void					TurnEyes(float fYaw, float fPitch, float fRoll);
	void					SaveCurrentEditableCloth() override;
	void					SaveModifiedMesh();
	void					OnEditorExit();
	void					AddItem(string sItemName);
	void					RemoveItem(string sItemName);
	void					WearItem(string sItemID);
	ICharacter*				GetCurrentCharacter() override;

private:

	enum TZoomType
	{
		eLarge = 0,
		eBody,
		eHead,
		eEye		
	};

	void					InitEyeNodes();
	void					InitHeadNode(INode* pParent);
	void					InitSpawnedCharacter();
	void					InitCamera(const CVector& pos);
	void					ZoomCameraLarge();
	void					ZoomCameraBody();
	void					ZoomCameraHead();
	void					ZoomCameraEye();
	void					Zoom(const CVector& pos, float fYaw, float fPitch, float fRoll);
	static void				OnMouseEventCallback(CPlugin* plugin, IEventDispatcher::TMouseEvent e, int x, int y);

	IScene*					m_pScene;
	ICharacter*				m_pCurrentCharacter;
	bool					m_bIsLeftMousePressed;
	int						m_nMousePosX;
	CWorldEditor*			m_pWorldEditor;
	IEntity*				m_pCurrentEditableCloth;
	CVector					m_offsetCloth;
	CVector					m_vOffsetEyes;
	ILoaderManager&			m_oLoaderManager;
	string					m_sCurrentEditableCloth;
	INode*					m_pLeftEye = nullptr;
	INode*					m_pRightEye = nullptr;
	INode*					m_pHeadNode = nullptr;
	TZoomType				m_eZoomType = eBody;
};