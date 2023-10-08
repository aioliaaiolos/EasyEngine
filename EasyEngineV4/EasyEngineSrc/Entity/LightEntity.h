#ifndef LIGHTENTITY_H
#define LIGHTENTITY_H

#include "Entity.h"

class IRessource;
class IRessourceManager;
class IRenderer;

class CLightEntity : public CEntity, public ILightEntity
{
public:
	CLightEntity(EEInterface& oInterface, IRessource* pLight);
	~CLightEntity();
	void				Update() override;
	void				UpdateRessource() override;
	void				SetIntensity( float fInstensity );
	float				GetIntensity();
	CVector				GetColor();
	IRessource::TLight	GetType();
	void				Unlink();
	void				GetEntityInfos(ILoader::CObjectInfos*& pInfos);
	void				BuildFromInfos(const ILoader::CObjectInfos& infos, IEntity* pParent, bool bExcludeChildren = false) override;
	void				GetEntityID(string& sID) {};
	const string&		GetEntityID() const { return ""; };
	void				Yaw(float yaw);
	void				Pitch(float yaw);
	void				Roll(float yaw);;

private:
	ILight*				m_pLight;

};

#endif // LIGHTENTITY_H