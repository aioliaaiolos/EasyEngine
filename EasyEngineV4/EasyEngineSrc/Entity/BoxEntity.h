#include "Shape.h"

class IBox;

class CBoxEntity : public CShape
{
	IBox&	m_oBox;
	string	m_sEntityName;

public:
	CBoxEntity( IRenderer& oRenderer, IBox& oBox );
	void				Update();
	IBox&				GetBox();
	void				Colorize(float r, float g, float b, float a) {}
	void				GetEntityName(string& sName);
	IGeometry*			GetBoundingGeometry();
	int					GetCellSize() { throw 1; return -1.f; };
};