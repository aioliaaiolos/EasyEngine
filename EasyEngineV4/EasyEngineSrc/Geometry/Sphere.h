#include "IGeometry.h"

class CSphere : public ISphere
{
	CVector		m_oCenter;
	float		m_fRadius;

public:

	CSphere();
	CSphere( const CVector& oCenter, float fRadius );
	void				Set( CVector& oCenter, float fRadius );
	float				GetRadius() const override;
	void				SetRadius( float fRadius );
	void				SetCenter( CVector& oCenter );
	const CVector&		GetCenter() const;

};