#include "IGeometry.h"
#include "Shape.h"

class CQuadEntity : public CShape
{
public:
	CQuadEntity(IRenderer& oRenderer, IRessourceManager& oRessourceManager, IQuad& oQuad);
	void					Update();
	void					GetEntityName(string& sName);
	void					Colorize(float r, float g, float b, float a);
	int						GetCellSize() { throw 1; return -1.f; };

private:	

	IQuad&					m_oQuad;
	IRessourceManager&		m_oRessourceManager;
	string					m_sName;
	CVector					m_oColor;
};