#include "Shape.h"


class CRepere : public CShape
{
	CMatrix		m_oBase;
	string		m_sEntityID;
public:
	CRepere( IRenderer& oRenderer );
	void						Update();
	void						Colorize(float r, float g, float b, float a) {}
	void						GetEntityID(string& sName);
	virtual const string&		GetEntityID() const;
	void						SetEntityName(string sName);
	void						DrawCollisionBoundingBoxes(bool bDraw) {}
	int							GetCellSize() { throw 1; return -1; };
};