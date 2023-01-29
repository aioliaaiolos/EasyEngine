#include "Shape.h"
#include "IGeometry.h"

class CLineEntity : public CShape, public ILine
{
	CVector			m_oFirst;
	CVector			m_oLast;
	string			m_sEntityID;
	IGeometry*		m_pBoundingGemoetry;

public:

	CLineEntity( IRenderer& oRenderer );
	CLineEntity( IRenderer& oRenderer, const CVector& first, const CVector& last );
	void				Set( const CVector& first, const CVector& last );
	void				Update();
	void				Colorize(float r, float g, float b, float a) {}
	void				GetEntityID(string& sID);
	const string&		GetEntityID() const;
	void				GetPoints(CVector& first, CVector& last) const;
	int					GetCellSize() { throw 1; return -1.f; };
};