#pragma once

#include "Shape.h"


class CPlaneEntity : public CShape
{
public:
	CPlaneEntity(IRenderer& oRenderer, IRessourceManager& oRessourceManager, int slices, int size, string heightTexture, string diffuseTexture);
	virtual ~CPlaneEntity();
	void				Update() override;
	const string&		GetIDStr() const override;
	void				Colorize(float r, float g, float b, float a) override;
	int					GetCellSize() { throw 1; return -1.f; };
	const string&		GetTypeName() const;

private:
	IMesh*	m_pMesh;
	string	m_sEntityType;
};

