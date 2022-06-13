#include "Shape.h"


class CRepere : public CShape
{
	CMatrix		m_oBase;
	string		m_sEntityName;
public:
	CRepere( IRenderer& oRenderer );
	void			Update();
	void			Colorize(float r, float g, float b, float a) {}
	void			GetEntityName(string& sName);
	void			SetEntityName(string sName);
	void			DrawCollisionBoundingBoxes(bool bDraw) {}
	int				GetCellSize() { throw 1; return -1.f; };
};