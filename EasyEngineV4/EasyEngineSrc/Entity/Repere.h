#include "Shape.h"


class CRepere : public CShape
{
public:
	CRepere( IRenderer& oRenderer );
	void						Update();
	void						Colorize(float r, float g, float b, float a) {}
	virtual const string&		GetIDStr() const;
	void						SetEntityName(string sName);
	void						DrawCollisionBoundingBoxes(bool bDraw) {}
	int							GetCellSize() { throw 1; return -1; };
	const string&				GetTypeName() const;

private:
	CMatrix						m_oBase;
	string						m_sEntityID;
	string						m_sEntityType;
};