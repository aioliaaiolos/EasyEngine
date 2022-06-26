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
	void				BuildFromInfos(const ILoader::CObjectInfos& infos, CEntity* pParent);

};

#endif // LIGHTENTITY_H